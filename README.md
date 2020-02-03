# Swapbench

## What is it?
Swapbench is a simple benchmark tool which measures swap performance of a system.
It supports testing various memory access patterns including sequential, random, and strided access patterns.
The number of page faults, swap-in/outs and running time are reported.

## How to build
```
$ make
```

## How to run
```
$ ./run.sh <seq|rand> <memory limit (MiB)> <object size (MiB)> <stride (B)> <nr_repeat>
```

## Example
```
$ ./run.sh seq 8 10 4096 10
Generating time: 0.01s
COMM: ./access seq 8 10 4096 10
Load time: 0.02s
Access time (user): 0.04s
Access time (sys): 0.40s
Access time (real): 0.77s
nr_minor_page_faults: 22389
nr_major_page_faults: 3225
nr_page_faults: 25614
pswpin: 25598
pswpout: 7
```

## Troubleshooting
### "Error opening leader .." error
```
$ sudo ./setup.sh
```
