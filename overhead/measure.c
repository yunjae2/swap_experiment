#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PAGE_SIZE		4096

enum access_type {
	SEQUENTIAL = 0,
	RANDOM,
};

struct input_args {
	long size;		// object size
	int access_type;	// seq or rand
};

int handle_args(int argc, char **argv, struct input_args *args)
{
	if (argc != 3)
		goto error;

	if (!strcmp(argv[1], "seq"))
		args->access_type = SEQUENTIAL;
	else if (!strcmp(argv[1], "rand"))
		args->access_type = RANDOM;
	else
		goto error;

	args->size = atol(argv[2]) * 1024 * 1024;

	return 0;

error:
	printf("Usage: %s <seq|rand> <object size (MiB)>\n", argv[0]);
	return -1;
}

void perf_init(void)
{
}

void init_object(int **objp, long size, int access_type)
{
	int i, swap_with;
	int from, to;
	int nr_entries;
	int temp;
	int *access_seq;
	int *object;

	if (posix_memalign((void **)objp, PAGE_SIZE, size)) {
		printf("Object allocation failed!\n");
		exit(1);
	}
	object = *objp;

	memset(object, 0, size);

	nr_entries = size / sizeof(int);
	if (access_type == SEQUENTIAL) {
		for (i = 0; i < nr_entries; i++)
			object[i] = (i + 1) % nr_entries;
	} else {
		srand(42);
		access_seq = malloc(size);
		for (i = 0; i < nr_entries; i++)
			access_seq[i] = i;

		/* Randomize access sequence */
		for (i = 0; i < nr_entries; i++) {
			swap_with = rand() % nr_entries;
			if (i == swap_with)
				continue;
			temp = access_seq[i];
			access_seq[i] = access_seq[swap_with];
			access_seq[swap_with] = temp;
		}

		/* Write access sequence to object */
		from = access_seq[nr_entries - 1];
		for (i = 0; i < nr_entries; i++) {
			to = access_seq[i];
			object[from] = to;
			from = to;
		}

		free(access_seq);
	}
}

void perf_record_start(void)
{
}

void access_object(void)
{
}

void perf_record_end(void)
{
}

void perf_report(void)
{
}

int main(int argc, char **argv)
{
	int *object;
	long size;
	int access_type;
	struct input_args args;

	if (handle_args(argc, argv, &args))
		return -1;

	size = args.size;
	access_type = args.access_type;

	perf_init();
	init_object(&object, size, access_type);
	perf_record_start();
	access_object();
	perf_record_end();
	perf_report();

	return 0;
}
