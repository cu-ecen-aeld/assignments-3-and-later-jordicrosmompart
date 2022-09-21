#!/bin/sh
# Tester script for assignment 1 and assignment 2
# Author: Siddhant Jajoo

set -e
set -u

NUMFILES=10
WRITESTR=AELD_IS_FUN
WRITEDIR=/tmp/aeld-data
OUTPUTDIR=/tmp/assignment-4-result.txt
username=$(cat /etc/finder-app/conf/username.txt)

if [ $# -lt 2 ]
then
	echo "Using default value ${WRITESTR} for string to write"
	if [ $# -lt 1 ]
	then
		echo "Using default value ${NUMFILES} for number of files to write"
	else
		NUMFILES=$1
	fi	
else
	NUMFILES=$1
	WRITESTR=$2
fi

MATCHSTR="The number of files are ${NUMFILES} and the number of matching lines are ${NUMFILES}"

echo "Writing ${NUMFILES} files containing string ${WRITESTR} to ${WRITEDIR}"

rm -rf "${WRITEDIR}"
mkdir -p "$WRITEDIR"

#The WRITEDIR is in quotes because if the directory path consists of spaces, then variable substitution will consider it as multiple argument.
#The quotes signify that the entire string in WRITEDIR is a single string.
#This issue can also be resolved by using double square brackets i.e [[ ]] instead of using quotes.
if [ -d "$WRITEDIR" ]
then
	echo "$WRITEDIR created"
else
	exit 1
fi

#echo "Removing the old writer utility and compiling as a native application"
#make clean
#make

#Know if the system holds "writer" in PATH
if [ -z "$(which writer)" ]
then
	echo "Local writer will be used. Not in PATH."
	WRITER=./writer
else
	echo "Using writer from PATH"
	WRITER=writer
fi

for i in $( seq 1 $NUMFILES)
do
	${WRITER} "$WRITEDIR/${username}$i.txt" "$WRITESTR"
done

#Know if the system holds "finder.sh" in PATH
if [ -z "$(which finder.sh)" ]
then
	FINDER=./finder.sh
else
	FINDER=finder.sh
fi

OUTPUTSTRING=$(${FINDER} "$WRITEDIR" "$WRITESTR")

set +e
echo ${OUTPUTSTRING} | grep "${MATCHSTR}"

if [ $? -eq 0 ]; then
	echo "success"
	#Write output of finder command as requested per Assignment 4 Part 2
	echo ${OUTPUTSTRING} > ${OUTPUTDIR} 
	exit 0
else
	echo "failed: expected  ${MATCHSTR} in ${OUTPUTSTRING} but instead found"
	#Write output of finder command as requested per Assignment 4 Part 2
	echo ${OUTPUTSTRING} > ${OUTPUTDIR} 
	exit 1
fi
