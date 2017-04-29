#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#include "../lib/aathread.h"


#define THREADS_COUNT	8

int alive_threads = THREADS_COUNT;

int mutex_a, mutex_b;

void block_on_a(void *sleep_time);
void block_on_b(void *sleep_time);

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
	
	fprintf(stderr, "Zero incarnation.\n");
	aathread_init();

	mutex_a = aamutex_create();
	mutex_b = aamutex_create();

	for (i = 1; i < THREADS_COUNT; ++i) {
		if (i % 2 == 1)
			aathread_start(block_on_a, (void *)i);
		else
			aathread_start(block_on_b, (void *)i);
	}
	
	while (alive_threads > 1) {
		/* time(&now); */
		/* fprintf(stderr, "Zero locking mutex: %s", ctime(&now)); */
		aamutex_lock(mutex_b);
		time(&now);
		fprintf(stderr, "Zero locked mutex: %s", ctime(&now));
		sleep_through_signals(1);
		aamutex_unlock(mutex_b);
		time(&now);
		fprintf(stderr, "Zero released mutex: %s", ctime(&now));
		sleep_through_signals(1);
	}

	aamutex_destroy(mutex_b);
	aamutex_destroy(mutex_a);
	aathread_finish();
	return 0;
}

void block_on_a(void *sleep_time) {
	int i, mynum = (int)sleep_time;
	time_t now;

	for (i = 0; i < 8; ++i) {
		/* time(&now); */
		/* fprintf(stderr, "%d locking mutex: %s", mynum, ctime(&now)); */
		aamutex_lock(mutex_a);
		time(&now);
		fprintf(stderr, "%d locked mutex: %s", mynum, ctime(&now));
		sleep_through_signals(mynum);
		aamutex_unlock(mutex_a);
		time(&now);
		fprintf(stderr, "%d released mutex: %s", mynum, ctime(&now));
		sleep_through_signals(1);
	}
	--alive_threads;
	fprintf(stderr, "%d dying. alive_threads: %d\n", mynum, alive_threads);
}

void block_on_b(void *sleep_time) {
	int i, mynum = (int)sleep_time;
	time_t now;

	for (i = 0; i < 4; ++i) {
		/* time(&now); */
		/* fprintf(stderr, "%d locking mutex: %s", mynum, ctime(&now)); */
		aamutex_lock(mutex_b);
		time(&now);
		fprintf(stderr, "%d locked mutex: %s", mynum, ctime(&now));
		sleep_through_signals(mynum);
		aamutex_unlock(mutex_b);
		time(&now);
		fprintf(stderr, "%d released mutex: %s", mynum, ctime(&now));
		sleep_through_signals(1);
	}
	--alive_threads;
	fprintf(stderr, "%d dying. alive_threads: %d\n", mynum, alive_threads);
	aathread_exit();
}
