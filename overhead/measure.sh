#!/bin/bash

if [ "$#" -ne 2 ]
then
	echo "Usage: $0 <seq|rand> <Object size (MiB)>"
	exit
fi

TYPE=$1
SIZE=$2

./generate $TYPE $SIZE && ./access $TYPE $SIZE
