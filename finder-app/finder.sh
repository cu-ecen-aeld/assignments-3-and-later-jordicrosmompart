#!/bin/bash

usage() {
	echo "Command: $0 $1 $2"
	echo "Usage: $0 <searchdir> <searchstring>"
	echo "Functionality: search for files in <searchdir> that contain <searchstring>"
}

#Check that two arguments have been provided
if [ ! $# -eq 2 ]
then
	usage
	exit 1
fi

#Use variables instead of the command arguments. This implies easier refactor of the code
SEARCHDIR=$1
SEARCHSTRING=$2

#Check that the first argument is a directory
if [ ! -d $SEARCHDIR ]
then
	echo "$SEARCHDIR is not a directory."
	usage
	exit 1
fi

#Expression based on the following reference: https://stackoverflow.com/questions/9157138/recursively-counting-files-in-a-linux-directory
NUMBER_OF_FILES=$(find $SEARCHDIR -type f | wc -l)

#Expression based on the following reference: https://stackoverflow.com/questions/16956810/how-to-find-all-files-containing-specific-text-string-on-linux
LINE_COUNT=$(grep -rn $SEARCHDIR -e $SEARCHSTRING | wc -l)

echo "The number of files are ${NUMBER_OF_FILES} and the number of matching lines are ${LINE_COUNT}"