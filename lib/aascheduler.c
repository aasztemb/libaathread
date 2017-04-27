#include <signal.h>
#include <time.h>
#include <ucontext.h>

#include "aathread-internal.h"
#include "aaqueue.h"
#include "aascheduler.h"

#define SCHEDULING_SIGNAL	(SIGRTMAX - 1)
#define SCHEDULING_SECS		0
#define SCHEDULING_NSECS	((long int)100e6)

timer_t timer;

static void timer_handler(int signum __attribute__((unused))) {
	dispose_threads();
	aaqueue_head_to_tail(runnable_queue);
	reshedule(aaqueue_get_tail(runnable_queue));
}

void schedule_timer(void) {
	struct sigaction signal_action;
	struct sigevent signal_event;
	struct itimerspec timer_spec;

	signal_action.sa_handler = timer_handler;
	sigemptyset(&signal_action.sa_mask);
	signal_action.sa_flags = SA_RESTART;
	sigaction(SCHEDULING_SIGNAL, &signal_action, NULL);
	
	signal_event.sigev_notify = SIGEV_SIGNAL;
	signal_event.sigev_signo = SCHEDULING_SIGNAL;
	timer_create(CLOCK_MONOTONIC, &signal_event, &timer);
	/* Not CLOCK_PROCESS_CPUTIME_ID because time spent in sleep state does not count there */

	timer_spec.it_interval.tv_sec = timer_spec.it_value.tv_sec = SCHEDULING_SECS;
	timer_spec.it_interval.tv_nsec = timer_spec.it_value.tv_nsec = SCHEDULING_NSECS;
	timer_settime(&timer, 0, &timer_spec, NULL);
}

void cancel_timer(void) {
	signal(SCHEDULING_SIGNAL, SIG_IGN);
	timer_delete(timer);
	signal(SCHEDULING_SIGNAL, SIG_DFL);
}

void enable_signals(sigset_t *oldmask) {
	sigprocmask(SIG_SETMASK, oldmask, NULL);
}

void disable_signals(sigset_t *oldmask) {
	sigset_t scheduling_signal_set;

	sigemptyset(&scheduling_signal_set);
	sigaddset(&scheduling_signal_set, SCHEDULING_SIGNAL);
	
	sigprocmask(SIG_BLOCK, &scheduling_signal_set, oldmask);
}

int is_scheduling_signal_blocked(void) {
	sigset_t mask;
	sigprocmask(0, NULL, &mask);
	return sigismember(&mask, SCHEDULING_SIGNAL);
}

void force_unblock_scheduling_signal(void) {
	sigset_t scheduling_signal_set;

	sigemptyset(&scheduling_signal_set);
	sigaddset(&scheduling_signal_set, SCHEDULING_SIGNAL);
	
	sigprocmask(SIG_UNBLOCK, &scheduling_signal_set, NULL);
}

void reshedule(struct aathread *previous_thread) {
	struct aathread *new_thread;
	
	new_thread = aaqueue_get_head(runnable_queue);
	
	if (new_thread == NULL)
		return;
	
	if (previous_thread == NULL)
		setcontext(&new_thread->context);
	else
		swapcontext(&previous_thread->context, &new_thread->context);
}
