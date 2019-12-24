# Swapbench

## How to build
```
$ make
```

## How to run
```
$ ./measure.sh <seq|rand> <memory limit (MiB)> <object size (MiB)> <stride (B)>
```

## Troubleshooting
### "Error opening leader .." error
There are two ways to avoid this error.
1. Run the benchmark in root, or
2. Set `/proc/sys/kernel/perf_event_paranoid` to 1 or under
