#!/bin/bash

if [ ! $# -eq 2 ]
then
	echo "You must provide two arguments. First is the full filename to be created and second the string to appended into it."
	exit 1
fi

#Expression based on the following reference: https://www.baeldung.com/linux/create-folder-path-file
$(mkdir -p $(dirname $1))

if [ $? -eq 1 ]
then
	exit 1
fi

$(echo $2 > $1)

