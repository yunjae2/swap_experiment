#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct input_args {
	long size;		// object size
	char *access_type;	// seq or rand
};

int handle_args(int argc, char **argv, struct input_args *args)
{
	if (argc != 3)
		goto error;
	if (strcmp(argv[1], "seq") && strcmp(argv[1], "rand"))
		goto error;

	args->size = atol(argv[2]) * 1024 * 1024;
	args->access_type = malloc(sizeof(argv[1]));
	strcpy(args->access_type, argv[1]);

	return 0;

error:
	printf("Usage: %s <seq|rand> <object size (MiB)>\n", argv[0]);
	return -1;
}

void perf_init(void)
{
}

void init_object(void)
{
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
	struct input_args args;
	if (handle_args(argc, argv, &args))
		return -1;

	perf_init();
	init_object();
	perf_record_start();
	access_object();
	perf_record_end();
	perf_report();

	return 0;
}
