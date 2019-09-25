#!/bin/bash

if [ "$#" -ne 3 ]
then
	echo "Usage: $0 <seq|rand> <Memory size (MiB)> <Object size (MiB)>"
	exit
fi

TYPE=$1
MEM=$2
SIZE=$3

./generate $TYPE $SIZE && ./run_memcg_lim.sh $MEM "./access $TYPE $SIZE"
