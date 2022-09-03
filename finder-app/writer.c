/**
* @file writer.c
* @brief Creates a file and appends a string into it
*
* Takes two arguments from terminal: <path_and_filename> and <string>
* Creates as file with the directory and name specified in <path_and_filename>
* Writes the contents in <string> into the created file.
*/

//Includes
#include <stdlib.h>
#include <stdio.h>
#include <syslog.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

/**
* usage
* @brief Prints the usage of the program.
*
* @param  char* command called
* @return void
*/
void usage(char *command)
{
    //Print the usage
    printf("Command: %s <filename> <string>\n", command);
    printf("Functionality: creates a file <filename> and writes <string> into it.\n");
    
    return;
}

/**
* main
* @brief Follows the steps described in the file header.
* 
* @param int	number of command arguments
* @param char**	array of arguments; argv[1] is <filename>, and argv[2] is <string>
* @return void
*/
void main(int c, char **argv)
{
    //Open the log to write to the default "/var/log/syslog" and set the LOG_USER facility
    openlog(NULL, 0, LOG_USER);

    //Check that the two arguments have been provided
    if(c != 2 + 1)
    {
        syslog(LOG_ERR, "Invalid number of arguments");
        usage(argv[0]);
        exit(1);
    }
    
    //Keep the terminal arguments in new variables to ease future code refactors
    char *filename = argv[1];
    char *string = argv[2];
    //Log to syslog what the program intends to do
    syslog(LOG_DEBUG, "Writing %s to %s.", string, filename);

    //Create the file, overwriting if it exists, and with write-only privileges
    int fd = open(filename, O_CREAT | O_TRUNC | O_WRONLY, 0666);
    //Check if an error has occurred
    if(fd == -1)
    {
        //Print it to syslog
        syslog(LOG_ERR, "Could not create the file: %s", strerror(errno));
        exit(1);
    }
    //Write the string to the file
    int written_bytes;
    int len_to_write = strlen(string);
    char *ptr_to_write = string;

    while(len_to_write != 0)
    {
        written_bytes = write(fd, ptr_to_write, len_to_write);
        if(written_bytes == -1)
        {
            //If the error is caused by an interruption of the system call try again
            if(errno == EINTR)
                continue;

            //Else, error occurred, print it to syslog and finish program
            syslog(LOG_ERR, "Could not write to the file: %s", strerror(errno));
            exit(1);
        }
        len_to_write -= written_bytes;
        ptr_to_write += written_bytes; 
    }

    //Once the write is finished, the file can be closed
    int ret = close(fd);
    //Check if an error has occurred closing the file
    if(ret == -1)
    {
        //Print it to syslog
        syslog(LOG_ERR, "Error happened while closing the file: %s", strerror(errno));
        exit(1);
    }

    return;
}