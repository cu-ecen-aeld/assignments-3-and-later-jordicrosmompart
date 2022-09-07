#include "systemcalls.h"
#include <errno.h>
#include <syslog.h>
#include <sys/wait.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>


#define	STDOUT	(1)

/**
 * @param cmd the command to execute with system()
 * @return true if the command in @param cmd was executed
 *   successfully using the system() call, false if an error occurred,
 *   either in invocation of the system() call, or if a non-zero return
 *   value was returned by the command issued in @param cmd.
*/
bool do_system(const char *cmd)
{

/*
 *  Call the system() function with the command set in the cmd
 *   and return a boolean true if the system() call completed with success
 *   or false() if it returned a failure
*/
	int ret_val = system(cmd);
	
	//If cmd is NULL, the value returned is interpreted differently
	if(cmd == NULL)
	{
		//0 means that no shell is available
		if(!ret_val)
		{
			syslog(LOG_ERR, "No shell available.");
			return false;
		}
		else
		{	
			syslog(LOG_DEBUG, "Shell available.");
			return true;
		}
	}
	//cmd is not NULL, check return value and return accordingly
	else
	{
		if(ret_val == -1)
		{
			//Check what error occurred and print it on syslog (default facility)
			syslog(LOG_ERR, "Could not successfully execute the command \"%s\": %s", cmd, strerror(errno));
			//Return false
			return false;
		}
		else if(ret_val == 127)
		{
			//Unknown to the system() caller if the call returned 127 or the invocation failed
			syslog(LOG_ERR, "Could not successfully execute the command \"%s\"", cmd);
			//Return false, expecting that the process called does not return 127 on success
			return false;
		}
		else
		{
			//Syslog the return value  (default facility)
			syslog(LOG_DEBUG, "The command %s ran successfully and it returned %d.", cmd, ret_val);
			return true;
		}
    }
		
}

/**
* @param count -The numbers of variables passed to the function. The variables are command to execute.
*   followed by arguments to pass to the command
*   Since exec() does not perform path expansion, the command to execute needs
*   to be an absolute path.
* @param ... - A list of 1 or more arguments after the @param count argument.
*   The first is always the full path to the command to execute with execv()
*   The remaining arguments are a list of arguments to pass to the command in execv()
* @return true if the command @param ... with arguments @param arguments were executed successfully
*   using the execv() call, false if an error occurred, either in invocation of the
*   fork, waitpid, or execv() command, or if a non-zero return value was returned
*   by the command issued in @param arguments with the specified arguments.
*/

bool do_exec(int count, ...)
{
    va_list args;
    va_start(args, count);
    char * command[count+1];
    int i;
    
    //Load all the arguments to "command"
    for(i=0; i<count; i++)
    {
        command[i] = va_arg(args, char *);
    }
    command[count] = NULL;

/*
 *   Execute a system command by calling fork, execv(),
 *   and wait instead of system (see LSP page 161).
 *   Use the command[0] as the full path to the command to execute
 *   (first argument to execv), and use the remaining arguments
 *   as second argument to the execv() command.
 *
*/
	int fork_ret = fork();
	if(fork_ret == -1)
	{
		syslog(LOG_ERR, "Unable to perform fork: %s.", strerror(errno));
		return false;
	}
	else if(fork_ret == 0)
	{
		//Child process, execute the other process
		execv(command[0], command);
		syslog(LOG_ERR, "execv failed: %s.", strerror(errno));
		return false;
	}

    va_end(args);

	//Wait for the child process to finalize
	int status;
	int wait_ret = waitpid(fork_ret, &status, 0);
	if(wait_ret == -1)
	{
		syslog(LOG_ERR, "waitpid() failed: %s.", strerror(errno));
		return false;
	}
	else if(wait_ret == fork_ret)
	{
		if(WIFEXITED(status))
		{
			//Execution finished calling exit.
			//The value matters to this function
			if(WEXITSTATUS(status) != 0)
			{
				syslog(LOG_ERR, "Child process exited with a non-zero value."); 
				return false;
			}
			else
			{
				syslog(LOG_DEBUG, "Child process finalized successfully.");
				return true;
			}
		}
		else
		{
			syslog(LOG_ERR, "Child process did not reach the end of its execution normally.");
			return false;
		}
	}
	else
	{
		syslog(LOG_ERR, "waitpid() returned an unexpected result.");
    	return false;
	}
	
	//This should never reach here, but the compiler complains
	return false;
}

/**
* @param outputfile - The full path to the file to write with command output.
*   This file will be closed at completion of the function call.
* All other parameters, see do_exec above
*/
bool do_exec_redirect(const char *outputfile, int count, ...)
{
    va_list args;
    va_start(args, count);
    char * command[count+1];
    int i;
    for(i=0; i<count; i++)
    {
        command[i] = va_arg(args, char *);
    }
    command[count] = NULL;
    // this line is to avoid a compile warning before your implementation is complete
    // and may be removed
    command[count] = command[count];


/*
 *   Call execv, but first using https://stackoverflow.com/a/13784315/1446624 as a refernce,
 *   redirect standard out to a file specified by outputfile.
 *   The rest of the behaviour is same as do_exec()
 *
*/
	int fd = open(outputfile, O_WRONLY | O_TRUNC | O_CREAT, 0644);
	if(fd < 0)
	{
		syslog(LOG_ERR, "Could not create the file where stdout will be redirected: %s", strerror(errno));
		return false;
	}
	if(dup2(fd, STDOUT) < 0)
	{
		syslog(LOG_ERR, "Could not redirect stdout to the file: %s", strerror(errno));
		close(fd);
		return false;
	}
	
	//Call do_exec
	bool exec_ret = do_exec(count, args);

    va_end(args);

    return exec_ret;
}