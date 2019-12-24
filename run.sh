#!/bin/bash

if [ "$#" -ne 4 ]
then
	echo "Usage: $0 <seq|rand> <Memory size (MiB)> <Object size (MiB)> <Access stride (B)>"
	exit
fi

TYPE=$1
MEM=$2
SIZE=$3
STRIDE=$4

./generate $TYPE $MEM $SIZE $STRIDE &&
./run_memcg_lim.sh $MEM "./access $TYPE $MEM $SIZE $STRIDE"
