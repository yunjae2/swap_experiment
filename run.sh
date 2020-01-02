#!/bin/bash

if [ "$#" -ne 5 ]
then
	echo "Usage: $0 <seq|rand> <mem_limit (MiB)> <obj_sz (MiB)> <stride (B)> <nr_repeat>"
	exit
fi

TYPE=$1
MEM=$2
SIZE=$3
STRIDE=$4
REPEAT=$5

./generate $TYPE $MEM $SIZE $STRIDE $REPEAT &&
./run_memcg_lim.sh $MEM "./access $TYPE $MEM $SIZE $STRIDE $REPEAT"
