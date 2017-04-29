#include <stdlib.h>
#include <ucontext.h>

#include "aathread-internal.h"
#include "aaslist.h"
#include "aaqueue.h"
#include "aascheduler.h"
#include "aathread.h"
#include "aamutex.h"

#define AAMUTEX_UNLOCKED	1
#define AAMUTEX_LOCKED		0

struct aamutex {
	int mid;
	int value;
	struct aaqueue *blocked_queue;
};

struct aaslist *mutex_list = NULL;

int get_top_blocked_tid(void) {
	int top_tid = -1, top_mutex_tid;
	struct aaslist *node;
	
	for (node = mutex_list; node != NULL; node = node->next) {
		top_mutex_tid = get_top_tid(node->mutex->blocked_queue);
		if (top_tid < top_mutex_tid)
			top_tid = top_mutex_tid;
	}

	return top_tid;			
}

int aamutex_create(void) {
	sigset_t oldmask;
	struct aamutex *mutex = malloc(sizeof(struct aamutex));
	struct aaslist *node = malloc(sizeof(struct aaslist));
	mutex->value = 1;
	mutex->blocked_queue = aaqueue_new();
	
	disable_signals(&oldmask);
	
	if (mutex_list == NULL)
		mutex->mid = 0;
	else
		mutex->mid = mutex_list->mutex->mid + 1; /* grow head */

	node->next = mutex_list;
	node->mutex = mutex;
	mutex_list = node;

	enable_signals(&oldmask);
	
	return mutex->mid;
}

void aamutex_destroy(int mid) {
	sigset_t oldmask;
	struct aaslist *node, *last_node = NULL;

	disable_signals(&oldmask);

	for (node = mutex_list; node != NULL; node = node->next) {
		if (node->mutex->mid == mid)
			break;
		last_node = node;
	}

	if (node == NULL) /* mid not found */
		return;

	if (last_node == NULL) { /* pop head of the list */
		mutex_list = node->next;
	} else {
		last_node->next = node->next;
	}

	enable_signals(&oldmask);

	aaqueue_destroy(node->mutex->blocked_queue); /* TODO: unblock threads in the queue */
	free(node->mutex);
	free(node);
}

static struct aamutex *get_mutex_by_id(int mid) {
	struct aaslist *node;
	
	for (node = mutex_list; node != NULL; node = node->next)
		if (node->mutex->mid == mid)
			return node->mutex;
	return NULL;
}	

void aamutex_lock(int mid) {
	sigset_t oldmask;
	struct aamutex *mutex;
	struct aathread *current_thread;
	
	disable_signals(&oldmask);

	mutex = get_mutex_by_id(mid);
	if (mutex == NULL) {
		enable_signals(&oldmask);
		return;
	}

	if (mutex->value == AAMUTEX_UNLOCKED) {
		mutex->value = AAMUTEX_LOCKED;
		enable_signals(&oldmask);
		return;
	}

	current_thread = aaqueue_pop_head(runnable_queue);
	aaqueue_push_tail(mutex->blocked_queue, current_thread);
	reshedule(current_thread);
	enable_signals(&oldmask);
}

void aamutex_unlock(int mid) {
	sigset_t oldmask;
	struct aamutex *mutex;
	struct aathread *thread_to_unblock;
	
	disable_signals(&oldmask);

	mutex = get_mutex_by_id(mid);
	if (mutex == NULL) {
		enable_signals(&oldmask);
		return;
	}

	if (mutex->value == AAMUTEX_UNLOCKED) {
		enable_signals(&oldmask);
		return;
	}

	thread_to_unblock = aaqueue_pop_head(mutex->blocked_queue);
	if (thread_to_unblock == NULL) {
		mutex->value = AAMUTEX_UNLOCKED;
		enable_signals(&oldmask);
		return;
	}

	aaqueue_push_tail(runnable_queue, thread_to_unblock);
	enable_signals(&oldmask);
}
