#!/bin/sh

usage() {
	echo "Command: $0 <start/stop>"
	echo "Functionality: starts/stops the aesdsocket daemon."
}

#Check that one arguments has been provided
if [ ! $# -eq 1 ]
then
	usage
	exit 1
fi

if [ $1 = "start" ]
then
    echo "Starting aesdsocket"
    start-stop-daemon -S -n aesdsocket -a /usr/bin/aesdsocket -- -d
elif [ $1 = "stop" ]
then
    echo "Stopping aesdsocket"
    start-stop-daemon -K -n aesdsocket
fi