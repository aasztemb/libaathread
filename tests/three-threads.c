#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#include "../lib/aathread.h"


#define THREADS_COUNT	3

int alive_threads = THREADS_COUNT;

void two(void *sleep_time);
void three(void *sleep_time);

void sleep_through_signals(unsigned int seconds) {
	struct timespec to_sleep;

	clock_gettime(CLOCK_MONOTONIC, &to_sleep);
	to_sleep.tv_sec += seconds;
	
	while (clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &to_sleep, NULL) != 0)
		;
}

int main(int argc __attribute__((unused)), char *argv[] __attribute__((unused))) {
	int i;
	time_t now;
	void (*thread_funcs[THREADS_COUNT])(void *) = { NULL, two, three };
	int sleep_times[THREADS_COUNT] = { 1, 2, 3 };
	
	fprintf(stderr, "One incarnation.\n");
	aathread_init();

	for (i = 1; i < THREADS_COUNT; ++i) {
		aathread_start(thread_funcs[i], &sleep_times[i]);
	}
	
	while (alive_threads > 1) {
		time(&now);
		fprintf(stderr, "One: %s", ctime(&now));
		sleep_through_signals(sleep_times[0]);
	}

	aathread_finish();
	return 0;
}

void two(void *sleep_time) {
	int i;
	time_t now;

	for (i = 0; i < 8; ++i) {
		time(&now);
		fprintf(stderr, "Two: %s", ctime(&now));
		sleep_through_signals(*(int *)sleep_time);
	}
	--alive_threads;
	fprintf(stderr, "Two dying. alive_threads: %d\n", alive_threads);
}

void three(void *sleep_time) {
	int i;
	time_t now;

	for (i = 0; i < 4; ++i) {
		time(&now);
		fprintf(stderr, "Three: %s", ctime(&now));
		sleep_through_signals(*(int *)sleep_time);
	}
	--alive_threads;
	fprintf(stderr, "Three dying. alive_threads: %d\n", alive_threads);
	aathread_exit();
}
