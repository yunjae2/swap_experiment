#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <asm/unistd.h>
#include <linux/perf_event.h>
#include <sys/ioctl.h>

#define PAGE_SIZE		4096

enum access_type {
	SEQUENTIAL = 0,
	RANDOM,
};

struct input_args {
	long size;		// object size
	int access_type;	// seq or rand
};

struct perf_objects {
	int nr_objects;		// up to 10
	int fd[10];
	char desc[10][30];	// description for each object
};

long perf_event_open(struct perf_event_attr *hw_event, pid_t pid,
		int cpu, int group_fd, unsigned long flags)
{
	int ret;

	ret = syscall(__NR_perf_event_open, hw_event, pid, cpu, group_fd,
			flags);
	return ret;
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

int perf_init_object(__u32 type, __u64 config, int cpu)
{
	struct perf_event_attr pe;
	int fd;

	memset(&pe, 0, sizeof(struct perf_event_attr));
	pe.type = type;
	pe.size = sizeof(struct perf_event_attr);
	pe.disabled = 1;
	pe.exclude_hv = 1;
	pe.config = config;

	fd = perf_event_open(&pe, 0, cpu, -1, 0);
	if (fd == -1) {
		fprintf(stderr, "Error opening leader %llx\n", pe.config);
		exit(EXIT_FAILURE);
	}

	ioctl(fd, PERF_EVENT_IOC_RESET, 0);

	return fd;
}


struct perf_objects perf_init(void)
{
	struct perf_objects po;
	po.nr_objects = 1;
	strcpy(po.desc[0], "nr_page_faults");
	po.fd[0] = perf_init_object(PERF_TYPE_SOFTWARE,
			PERF_COUNT_SW_PAGE_FAULTS, 0);

	return po;
}

void init_object(int **objp, long size, int access_type)
{
	int i, swap_with;
	int from, to;
	int nr_entries;
	int temp;
	int *access_seq;
	int *object;
	struct timespec start, end;

	clock_gettime(CLOCK_REALTIME, &start);

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

	clock_gettime(CLOCK_REALTIME, &end);
	printf("Init time: ");
	print_interval(&start, &end);
}

void perf_record_start(struct perf_objects *po)
{
	int i;

	for (i = 0; i < po->nr_objects; i++)
		ioctl(po->fd[i], PERF_EVENT_IOC_ENABLE, 0);
}

void access_object(int *object, int size)
{
	int i;
	int nr_iters = 0;
	int nr_entry = size / sizeof(int);
	struct timespec start, end;

	clock_gettime(CLOCK_REALTIME, &start);

	for (i = 0; nr_iters < nr_entry; i = object[i])
		nr_iters++;

	clock_gettime(CLOCK_REALTIME, &end);
	printf("Access time: ");
	print_interval(&start, &end);
}

void perf_record_end(struct perf_objects *po)
{
	int i;

	for (i = 0; i < po->nr_objects; i++)
		ioctl(po->fd[i], PERF_EVENT_IOC_DISABLE, 0);
}

void perf_report(struct perf_objects *po)
{
	int i;
	long long count;

	for (i = 0; i < po->nr_objects; i++) {
		read(po->fd[i], &count, sizeof(long long));
		printf("%s: %lld\n", po->desc[i], count);
		close(po->fd[i]);
	}
}

int main(int argc, char **argv)
{
	int *object;
	long size;
	int access_type;
	struct input_args args;
	struct perf_objects po;

	if (handle_args(argc, argv, &args))
		return -1;

	size = args.size;
	access_type = args.access_type;

	po = perf_init();
	init_object(&object, size, access_type);
	perf_record_start(&po);
	access_object(object, size);
	perf_record_end(&po);
	perf_report(&po);

	return 0;
}
