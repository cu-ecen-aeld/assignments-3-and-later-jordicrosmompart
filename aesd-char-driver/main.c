/**
 * @file aesdchar.c
 * @brief Functions and data related to the AESD char driver implementation
 *
 * Based on the implementation of the "scull" device driver, found in
 * Linux Device Drivers example code.
 *
 * @author Dan Walkes
 * @date 2019-10-22
 * @copyright Copyright (c) 2019
 *
 */

#include <linux/module.h>
#include <linux/syscalls.h>
#include <linux/init.h>
#include <linux/printk.h>
#include <linux/types.h>
#include <linux/cdev.h>
#include <linux/fs.h> // file_operations
#include <asm/uaccess.h>
#include "aesdchar.h"
int aesd_major =   0; // use dynamic major
int aesd_minor =   0;

MODULE_AUTHOR("Jordi Cros Mompart");
MODULE_LICENSE("Dual BSD/GPL");

struct aesd_dev aesd_device;

int aesd_open(struct inode *inode, struct file *filp)
{
    //Handle open
    PDEBUG("open\n");
    try_module_get(THIS_MODULE);

    return 0;
}

int aesd_release(struct inode *inode, struct file *filp)
{
    //Handle release
    PDEBUG("release\n");
    module_put(THIS_MODULE);

    return 0;
}

ssize_t aesd_read(struct file *filp, char __user *buf, size_t count,
                loff_t *f_pos)
{
    ssize_t retval = 0;
    size_t  entry_offset = 0;
    struct aesd_buffer_entry *entry = NULL;
    PDEBUG("read %zu bytes with offset %lld\n",count,*f_pos);
    
    //"count" is the number of bytes to be returned
    //"f_pos" is the offset from where to start the read

    //Set mutex
    mutex_lock(&aesd_device.lock);

    //Get the entry corresponding to the request
    entry = aesd_circular_buffer_find_entry_offset_for_fpos(&aesd_device.buffer, *f_pos, &entry_offset);
    //Check for the results
    if(!entry)
    {
        PDEBUG("Access to aesdchar is returning without contents read\n");
        mutex_unlock(&aesd_device.lock);
        *f_pos = 0;
        retval = 0;
        return retval;
    }
    //Copy from entry offset, "count" or size
    if(entry_offset + count >= entry->size)
    {
        //Copy only until the size
        if(copy_to_user(buf, &entry->buffptr[entry_offset],  entry->size - entry_offset) != 0)
        {
            PDEBUG("Could not copy memory to the user\n");
            mutex_unlock(&aesd_device.lock);
            retval = -EINVAL;
            return retval;
        }
        *f_pos = *f_pos + entry->size;
        retval = entry->size - entry_offset;
    }
    else
    {
        //Copy until the count
        //Copy only until the size
        if(copy_to_user(buf, &entry->buffptr[entry_offset],  count) != 0)
        {
            PDEBUG("Could not copy memory to the user\n");
            mutex_unlock(&aesd_device.lock);
            retval = -EINVAL;
            return retval;
        }
        *f_pos = *f_pos + count;
        retval = count;
    }

    //Release mutex
    mutex_unlock(&aesd_device.lock);

    return retval;
}

ssize_t aesd_write(struct file *filp, const char __user *buf, size_t count,
                loff_t *f_pos)
{
    ssize_t retval = -ENOMEM;
    const char *to_be_freed = NULL;
    PDEBUG("write %zu bytes with offset %lld\n",count,*f_pos);
    

    //Check filp and buf validity
    if(!filp || !buf)
    {
        PDEBUG("Could not reallocate a temporary command\n");
        retval = -EINVAL;
        return retval;
    }

    //Check that the count is not 0
    if(!count)
    {
        PDEBUG("Attempt to write 0 bytes\n");
        retval = 0;
        return retval;
    }

    //Set mutex
    mutex_lock(&aesd_device.lock);

    //Check if there is a partial write active
    if(aesd_device.partial_size)
    {
        aesd_device.partial_content = krealloc(aesd_device.partial_content, 
                                              sizeof(char)*(aesd_device.partial_size + count), GFP_KERNEL); 
        if(!aesd_device.partial_content)
        {
            PDEBUG("Could not reallocate a temporary command\n");
            mutex_unlock(&aesd_device.lock);
            return retval;
        }
    }
    //If not, allocate temporary space
    else
    {
        aesd_device.partial_content = kmalloc(sizeof(char)*count, GFP_KERNEL);
        if(!aesd_device.partial_content)
        {
            PDEBUG("Could not allocate a temporary command\n");
            mutex_unlock(&aesd_device.lock);
            return retval;
        }
    }
    
    //Perform the copy
    if(copy_from_user(&aesd_device.partial_content[aesd_device.partial_size], buf, count) != 0)
    {
        PDEBUG("Could not copy memory from the user\n");
        mutex_unlock(&aesd_device.lock);
        retval = -EINVAL;
        return retval;
    }
    aesd_device.partial_size += count;
    retval = count;

    //Check if last element is a '\n'
    if(aesd_device.partial_content[aesd_device.partial_size - 1] == '\n')
    {
        //Create the new entry
        struct aesd_buffer_entry entry;
        entry.buffptr = kmalloc(sizeof(char)*aesd_device.partial_size, GFP_KERNEL);
        if(!entry.buffptr)
        {
            PDEBUG("Not enough memory available\n");
            mutex_unlock(&aesd_device.lock);
            retval = -ENOMEM;
            return retval;
        }
        entry.size = aesd_device.partial_size;

        //Copy the contents inside the entry
        memcpy((void *)entry.buffptr, aesd_device.partial_content, aesd_device.partial_size);

        //Add the entry to the buffer
        to_be_freed = aesd_circular_buffer_add_entry(&aesd_device.buffer, &entry);
        //Check if a buffer needs to be freed
        if(to_be_freed)
        {
            kfree(to_be_freed);
            to_be_freed = NULL;
        }

        //Free the partial contents
        kfree(aesd_device.partial_content);
        aesd_device.partial_size = 0;
    }

    //Release mutex
    mutex_unlock(&aesd_device.lock);


    return retval;
}
struct file_operations aesd_fops = {
    .owner =    THIS_MODULE,
    .read =     aesd_read,
    .write =    aesd_write,
    .open =     aesd_open,
    .release =  aesd_release,
};

static int aesd_setup_cdev(struct aesd_dev *dev)
{
    int err, devno = MKDEV(aesd_major, aesd_minor);

    cdev_init(&dev->cdev, &aesd_fops);
    dev->cdev.owner = THIS_MODULE;
    dev->cdev.ops = &aesd_fops;
    err = cdev_add (&dev->cdev, devno, 1);
    if (err) {
        printk(KERN_ERR "Error %d adding aesd cdev\n", err);
    }
    return err;
}



int aesd_init_module(void)
{
    dev_t dev = 0;
    int result;
    result = alloc_chrdev_region(&dev, aesd_minor, 1,
            "aesdchar");
    aesd_major = MAJOR(dev);
    if (result < 0) {
        printk(KERN_WARNING "Can't get major %d\n", aesd_major);
        return result;
    }
    memset(&aesd_device,0,sizeof(struct aesd_dev));

    //After memsetting the struct, the mutex needs to be initialized
    mutex_init(&aesd_device.lock);
    //The partial writes pointer will be used within the write operations

    result = aesd_setup_cdev(&aesd_device);

    if( result ) {
        unregister_chrdev_region(dev, 1);
    }
    return result;

}

void aesd_cleanup_module(void)
{
    dev_t devno = MKDEV(aesd_major, aesd_minor);

    cdev_del(&aesd_device.cdev);

    //Destroy the mutex
    mutex_destroy(&aesd_device.lock);

    unregister_chrdev_region(devno, 1);
}



module_init(aesd_init_module);
module_exit(aesd_cleanup_module);
