#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#define PAGE_SIZE		4096

enum access_type {
	SEQUENTIAL = 0,
	RANDOM,
};

struct input_args {
	long memory_size;	// memory size
	long size;		// object size
	int access_type;	// seq or rand
	int stride;		// access stride
	int nr_repeat;		// number of repeat
};

int handle_args(int argc, char **argv, struct input_args *args)
{
	if (argc != 6)
		goto error;

	if (!strcmp(argv[1], "seq"))
		args->access_type = SEQUENTIAL;
	else if (!strcmp(argv[1], "rand"))
		args->access_type = RANDOM;
	else
		goto error;

	args->memory_size = atol(argv[2]) * 1024 * 1024;
	args->size = atol(argv[3]) * 1024 * 1024;
	args->stride = atoi(argv[4]);
	args->nr_repeat = atoi(argv[5]);

	if (args->stride % 4 != 0) {
		printf("The stride must be a multiple of 4.\n");
		goto error;
	}

	return 0;

error:
	printf("Usage: %s <seq|rand> <mem_limit (MiB)> <obj_sz (MiB)> <stride (B)> <nr_repeat>\n", argv[0]);
	return -1;
}

void print_interval(struct timespec *start, struct timespec *end)
{
	time_t diff_sec;
	long diff_nsec;

	diff_sec = end->tv_sec - start->tv_sec;
	diff_nsec = end->tv_nsec - start->tv_nsec;

	if (diff_nsec < 0) {
		diff_nsec += 1000 * 1000 * 1000;
		diff_sec -= 1;
	}

	printf("%ld.%06lds\n", diff_sec, diff_nsec / 1000);
}
