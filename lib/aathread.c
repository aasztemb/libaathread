#include <stdlib.h>
#include <sys/mman.h>
#include <ucontext.h>

#include "aathread-internal.h"
#include "aaqueue.h"
#include "aascheduler.h"
#include "aathread.h"

#define AA_THREAD_STACK_SIZE 32768

struct aaqueue *runnable_queue = NULL;
struct aaqueue *disposal_queue = NULL;

void aathread_init(void) {
	struct aathread *thread;
	
	runnable_queue = aaqueue_new();
	disposal_queue = aaqueue_new();
	thread = malloc(sizeof(struct aathread));
	getcontext(&thread->context);
	thread->start_routine = NULL;
	thread->arg = NULL;
	thread->tid = aaqueue_push_tail(runnable_queue, thread);
	schedule_timer();
}

void aathread_finish(void) {
	cancel_timer();
	aaqueue_pop_thread_by_id(runnable_queue, 0);
	aaqueue_destroy(runnable_queue);
	runnable_queue = NULL;
}

static void start_routine_wrapper(int tid) {
	struct aathread *thread = aaqueue_get_thread_by_id(runnable_queue, tid);
	/* fprintf(stderr, "%s: is_scheduling_signal_blocked() == %d\n", __func__, is_scheduling_signal_blocked()); */
	force_unblock_scheduling_signal();
	thread->start_routine(thread->arg);
	aathread_exit();
}

int aathread_start(void (*start_routine)(void *), void *arg) {
	int tid;
	struct aathread *new_thread = malloc(sizeof(struct aathread)),
		*current_thread = aaqueue_get_head(runnable_queue);
	sigset_t sigmask;

	getcontext(&new_thread->context);
	new_thread->context.uc_stack.ss_sp = mmap(NULL, AA_THREAD_STACK_SIZE, PROT_EXEC | PROT_READ | PROT_WRITE,
						  MAP_PRIVATE | MAP_ANONYMOUS | MAP_STACK, -1, 0);
	new_thread->context.uc_stack.ss_size = AA_THREAD_STACK_SIZE;
	new_thread->context.uc_link = &current_thread->context;
	new_thread->start_routine = start_routine;
	new_thread->arg = arg;
	disable_signals(&sigmask);
	tid = aaqueue_push_tail(runnable_queue, new_thread);
	new_thread->tid = tid;
	makecontext(&new_thread->context, (void (*)(void))start_routine_wrapper, 2, tid);
	enable_signals(&sigmask);

	return tid;
}

/* exiting / killing main (0) thread results in program termination */

static void end_thread(struct aathread *thread) {
	sigset_t sigmask;
	disable_signals(&sigmask);
	aaqueue_pop_thread_by_id(runnable_queue, thread->tid);
	aaqueue_relink_parent_contexts(runnable_queue, &thread->context, thread->context.uc_link);
	aaqueue_push_tail(disposal_queue, thread);
	enable_signals(&sigmask);
}

void dispose_threads(void) {
	struct aathread *thread;
	
	while ((thread = aaqueue_pop_head(disposal_queue)) != NULL) {
		munmap(thread->context.uc_stack.ss_sp, AA_THREAD_STACK_SIZE);
		free(thread);
	}
}

void aathread_exit(void) {
	struct aathread *thread;

	thread = aaqueue_get_head(runnable_queue);
	end_thread(thread);
	reshedule(NULL);
}

int aathread_kill(int tid) {
	struct aathread *thread;

	thread = aaqueue_get_thread_by_id(runnable_queue, tid);
	if (thread == NULL)
		return -1;
	
	end_thread(thread);
	reshedule(NULL);

	return 0;
}
