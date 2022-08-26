#!/bin/bash
if [ ! $# -eq 2 ]
then
	echo "You must provide two arguments. First is the folder and second the string to be searched."
	exit 1
fi

if [ ! -d $1 ]
then
	echo "$1 is not a directory."
	exit 1
fi

#Expression based on the following reference: https://stackoverflow.com/questions/9157138/recursively-counting-files-in-a-linux-directory
NUMBER_OF_FILES=$(find $1 -type f | wc -l)

#Expression based on the following reference: https://stackoverflow.com/questions/16956810/how-to-find-all-files-containing-specific-text-string-on-linux
LINE_COUNT=$(grep -rn $1 -e $2 | wc -l)

echo "The number of files are ${NUMBER_OF_FILES} and the number of matching lines are ${LINE_COUNT}"
