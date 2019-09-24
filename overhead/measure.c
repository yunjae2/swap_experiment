#include <stdio.h>

int handle_args(int argc, char **argv)
{
	return 0;
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
	if (handle_args(argc, argv))
		return -1;

	perf_init();
	init_object();
	perf_record_start();
	access_object();
	perf_record_end();
	perf_report();
}
