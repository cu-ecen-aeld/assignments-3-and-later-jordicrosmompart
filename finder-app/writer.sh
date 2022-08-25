#!/bin/bash

if [ ! $# -eq 2 ]
then
	echo "You must provide two arguments. First is the full filename to be created and second the string to appended into it."
	exit 1
fi

$(mkdir -p $(dirname $1))

if [ $? -eq 1 ]
then
	exit 1
fi

$(echo $2 > $1)

