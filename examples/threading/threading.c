#include "threading.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <errno.h>
#include <syslog.h>

//Defines
#define NS_TO_MS    (1000000)

// Optional: use these functions to add debug or error prints to your application
#define DEBUG_LOG(msg,...)
//#define DEBUG_LOG(msg,...) printf("threading: " msg "\n" , ##__VA_ARGS__)
#define ERROR_LOG(msg,...) printf("threading ERROR: " msg "\n" , ##__VA_ARGS__)

/**
 * Implements the functionality stated in threading.h
 * @param thread_param needs to be parsed to thread_data*
 * @return the same structure modified depending on the results
 */
void* threadfunc(void* thread_param)
{
    //Parse the parameter to a thread_data*
    struct thread_data *parameter = (struct thread_data *)thread_param;

    //Wait (implemented thanks to https://man7.org/linux/man-pages/man2/nanosleep.2.html)
    struct timespec req = {0, (parameter->wait_acquire_ms)*NS_TO_MS};
    struct timespec rem;
    int ret = nanosleep(&req, &rem);
    //Check if an error occurred
    while(ret == -1)
    {
        //Check if the sleep has stopped due to a signal
        if(errno == EINTR)
        {
            syslog(LOG_DEBUG, "Sleep interrupted by a signal, resuming remaining time...");
            req.tv_sec= 0;
            req.tv_nsec = rem.tv_nsec;
            ret = nanosleep(&req, &rem);
        }
        //If not, an error occurred and the thread exits
        else
        {
            syslog(LOG_ERR, "Sleep error detected.");
            parameter->thread_complete_success = false;
            return (void *)parameter;
        }
        
    }

    //Obtain mutex
    pthread_mutex_lock(parameter->mutex);
    syslog(LOG_DEBUG, "The thread has obtained the mutex.");

    //Wait after locking
    req.tv_sec = 0;
    req.tv_nsec = (parameter->wait_locked_ms)*NS_TO_MS;
    ret = nanosleep(&req, &rem);
    //Check if an error occurred
    while(ret == -1)
    {
        //Check if the sleep has stopped due to a signal
        if(errno == EINTR)
        {
            req.tv_sec= 0;
            req.tv_nsec = rem.tv_nsec;
            ret = nanosleep(&req, &rem);
        }
        //If not, an error occurred and the thread exits
        else
        {
            syslog(LOG_ERR, "Sleep error detected.");
            parameter->thread_complete_success = false;
            return (void *)parameter;
        }
        
    }

    //Release mutex as described by thread_data structure
    pthread_mutex_unlock(parameter->mutex);
    syslog(LOG_DEBUG, "The thread has released the mutex.");

    //Return success
    parameter->thread_complete_success = true;
    return (void *)parameter;
}


bool start_thread_obtaining_mutex(pthread_t *thread, pthread_mutex_t *mutex,int wait_to_obtain_ms, int wait_to_release_ms)
{
    //Allocate memory for thread_data passed to the thread to be created
    struct thread_data *parameter = malloc(sizeof(struct thread_data));
    //Check if it has been possible to allocate
    if(!parameter)
    {
        syslog(LOG_ERR, "Could not create a new thread - no memory available");
        return false;
    }

    //Set up structure assuming the mutex passed is already initialized
    parameter->mutex = mutex;
    parameter->wait_acquire_ms = wait_to_obtain_ms;
    parameter->wait_locked_ms = wait_to_release_ms;

    //Create the thread
    int ret = pthread_create(thread, NULL, threadfunc, (void *)parameter);
    //Check if the thread has been started
    if(ret != 0)
    {
        syslog(LOG_ERR, "The thread could not be created.");
        //"thread" param should be set to NULL since no thread was created
        thread = NULL;
        return false;
    }
    
    //Do not join to the created thread to not wait
    //Do not detach to prevent this function caller to lose the created thread's data
    //Successful function call
    return true;
}

