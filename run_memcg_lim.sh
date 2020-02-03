#!/bin/bash
#
# Run a command with memory cgroup that has specific memory limit
# Forked from https://github.com/sjp38/lazybox

if [ $# -ne 2 ]
then
	echo "Usage: $0 <mem limit in MiB> <command>"
	exit 1
fi

MEMLIM=`printf '%.0f\n' "$(echo "scale=1; $1*1024*1024" | bc -l)"`
COMM=$2

MEMCG_ORIG_DIR=/sys/fs/cgroup/memory/
MEMCG_DIR=/sys/fs/cgroup/memory/run_memcg_lim_$USER

if [ -d "$MEMCG_DIR" ]
then
	sudo rmdir $MEMCG_DIR
fi

sudo mkdir $MEMCG_DIR
sudo bash -c "echo $$ > $MEMCG_DIR/tasks"
sudo bash -c "echo $MEMLIM > $MEMCG_DIR/memory.limit_in_bytes"

eval "$COMM"

while read pid; do
	sudo bash -c "echo $pid > $MEMCG_ORIG_DIR/tasks"
done < $MEMCG_DIR/tasks

sudo rmdir $MEMCG_DIR
