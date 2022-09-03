#!/bin/bash

usage() {
	echo "Command: $0 $1 $2"
	echo "Usage: $0 <path_and_filename> <string>"
	echo "Functionality: creates a file in <path_and_filename> and writes <string> into it"
}

#Check that two arguments have been provided
if [ ! $# -eq 2 ]
then
	usage
	exit 1
fi

#Use variables instead of the command arguments. This implies easier refactor of the code
PATH_FILENAME=$1
STRING=$2

#Expression based on the following reference: https://www.baeldung.com/linux/create-folder-path-file
$(mkdir -p $(dirname $PATH_FILENAME))

#Check if there has been an error creating the directory
if [ $? -eq 1 ]
then
	echo "The directories could not be created."
	exit 1
fi

$(echo $STRING > $PATH_FILENAME)

