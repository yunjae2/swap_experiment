#include "common.h"
#include <sys/stat.h>
#include <sys/types.h>

void generate_object(long size, int access_type, int stride)
{
	int i, swap_with;
	int from, to;
	int nr_entries, word_per_stride;
	int temp;
	int *access_seq;
	int *object;
	struct timespec start, end;
	FILE *fp;
	char filename[30];

	if (access("objects", W_OK))
		mkdir("objects", 0777);
	sprintf(filename, "objects/%d-%ldMiB-%dB.bin", access_type,
			size / (1024 * 1024), stride);

	/* If object exists, use it */
	if (!access(filename, F_OK))
		return;

	clock_gettime(CLOCK_REALTIME, &start);

	object = malloc(size);
	memset(object, 0, size);

	nr_entries = size / stride;
	word_per_stride = stride / sizeof(int);
	if (access_type == SEQUENTIAL) {
		from = 0;
		for (i = 0; i < nr_entries; i++) {
			to = from + word_per_stride;
			object[from] = to;
			from = to;
		}
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
		from = access_seq[nr_entries - 1] * word_per_stride;
		for (i = 0; i < nr_entries; i++) {
			to = access_seq[i] * word_per_stride;
			object[from] = to;
			from = to;
		}

		free(access_seq);
	}

	fp = fopen(filename, "wb");
	fwrite(object, size, 1, fp);
	fclose(fp);

	free(object);

	clock_gettime(CLOCK_REALTIME, &end);
	printf("Generating time: ");
	print_interval(&start, &end);
}

int main(int argc, char **argv)
{
	long size;
	int access_type;
	int stride;
	struct input_args args;

	if (handle_args(argc, argv, &args))
		return -1;

	size = args.size;
	access_type = args.access_type;
	stride = args.stride;

	generate_object(size, access_type, stride);

	return 0;
}
