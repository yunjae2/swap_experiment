#define _GNU_SOURCE
#include <asm/unistd.h>
#include <linux/perf_event.h>
#include <sys/ioctl.h>
#include <sys/times.h>
#include <sched.h>
#include "common.h"

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

int set_affinity(int cpu)
{
	cpu_set_t cpu_mask;
	CPU_ZERO(&cpu_mask);
	CPU_SET(cpu, &cpu_mask);

	if (sched_setaffinity(0, sizeof(cpu_mask), &cpu_mask)) {
		printf("Affinity set failed\n");
		return -1;
	}

	return 0;
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
	po.nr_objects = 3;
	strcpy(po.desc[0], "nr_minor_page_faults");
	po.fd[0] = perf_init_object(PERF_TYPE_SOFTWARE,
			PERF_COUNT_SW_PAGE_FAULTS_MIN, 0);
	strcpy(po.desc[1], "nr_major_page_faults");
	po.fd[1] = perf_init_object(PERF_TYPE_SOFTWARE,
			PERF_COUNT_SW_PAGE_FAULTS_MAJ, 0);
	strcpy(po.desc[2], "nr_page_faults");
	po.fd[2] = perf_init_object(PERF_TYPE_SOFTWARE,
			PERF_COUNT_SW_PAGE_FAULTS, 0);

	return po;
}

void load_object(int **objp, long size, int access_type)
{
	int nr_entries;
	int *object;
	struct timespec start, end;
	FILE *fp;
	char filename[30];

	clock_gettime(CLOCK_REALTIME, &start);

	if (posix_memalign((void **)objp, PAGE_SIZE, size)) {
		printf("Object allocation failed!\n");
		exit(1);
	}
	object = *objp;

	sprintf(filename, "objects/%d-%ldMiB.bin", access_type,
			size / (1024 * 1024));
	if (access(filename, R_OK)) {
		printf("Object load failed!\n");
		exit(1);
	}
	fp = fopen(filename, "rb");
	fread(object, size, 1, fp);
	fclose(fp);

	clock_gettime(CLOCK_REALTIME, &end);
	printf("Load time: ");
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
	struct tms start, end;
	clock_t utime, stime;
	long tps = sysconf(_SC_CLK_TCK);	// tick per second

	times(&start);

	for (i = 0; nr_iters < nr_entry; i = object[i])
		nr_iters++;

	times(&end);

	utime = end.tms_utime - start.tms_utime;
	stime = end.tms_stime - start.tms_stime;
	printf("Access time (user): %ld.%02lds\n", utime / tps, utime % tps);
	printf("Access time (sys): %ld.%02lds\n", stime / tps, stime % tps);
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

	if (set_affinity(0))
		return -1;

	po = perf_init();
	load_object(&object, size, access_type);
	perf_record_start(&po);
	access_object(object, size);
	perf_record_end(&po);
	perf_report(&po);

	return 0;
}