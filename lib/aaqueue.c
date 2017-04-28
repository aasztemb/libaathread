#include <stdlib.h>
#include <ucontext.h>

#include "aathread-internal.h"
#include "aaslist.h"
#include "aaqueue.h"

struct aaqueue {
	struct aaslist *head;
	struct aaslist *tail;
	int length;
};

struct aaqueue *aaqueue_new(void) {
	struct aaqueue *queue = malloc(sizeof(struct aaqueue));
	queue->head = NULL;
	queue->tail = NULL;
	queue->length = 0;
	return queue;
}

void aaqueue_destroy(struct aaqueue *queue) {
	free(queue);
}

struct aathread *aaqueue_get_thread_by_id(struct aaqueue *queue, int tid) {
	struct aaslist *list;

	if (queue == NULL)
		return NULL;
	
	for (list = queue->head; list != NULL; list = list->next) {
		if (list->thread->tid == tid)
			return list->thread;
	}
	
	return NULL;
}

int get_top_tid(struct aaqueue *queue) {
	struct aaslist *node;
	int top_tid = -1;

	if (queue == NULL)
		return -1;

	for (node = queue->head; node != NULL; node = node->next) {
		if (node->thread->tid > top_tid)
			top_tid = node->thread->tid;
	}

	return top_tid;
}

void aaqueue_push_tail(struct aaqueue *queue, struct aathread *thread) {
	struct aaslist *new_node;

	if (queue == NULL)
		return;

	new_node = malloc(sizeof(struct aaslist));
	new_node->thread = thread;
	new_node->next = NULL;
	
	if (queue->head == NULL) { /* empty queue */
		queue->head = new_node;
	} else {
		queue->tail->next = new_node;
	}

	queue->tail = new_node;
	++queue->length;
}

struct aathread *aaqueue_pop_head(struct aaqueue *queue) {
	if (queue == NULL || queue->head == NULL)
		return NULL;
	return aaqueue_pop_thread_by_id(queue, queue->head->thread->tid);
}

struct aathread *aaqueue_pop_thread_by_id(struct aaqueue *queue, int tid) {
	struct aaslist *node, *last_node = NULL;
	struct aathread *thread;

	if (queue == NULL)
		return NULL;

	for (node = queue->head; node != NULL; node = node->next) {
		if (node->thread->tid == tid)
			break;
		last_node = node;
	}

	if (node == NULL) /* tid not found */
		return NULL;

	if (last_node == NULL) { /* pop head of the queue */
		queue->head = node->next;
	} else {
		last_node->next = node->next;
	}

	if (node == queue->tail)
		queue->tail = last_node;
	
	--queue->length;

	thread = node->thread;
	free(node);
	
	return thread;
}

void aaqueue_relink_parent_contexts(struct aaqueue *queue, ucontext_t *old_context, ucontext_t *new_context) {
	struct aaslist *node;

	if (queue == NULL)
		return;
	
	for (node = queue->head; node != NULL; node = node->next) {
		if (node->thread->context.uc_link == old_context)
			node->thread->context.uc_link = new_context;
	}
}

void aaqueue_head_to_tail(struct aaqueue *queue) {
	if (queue == NULL || queue->head == NULL)
		return;

	queue->tail->next = queue->head; /* connect tail to head */
	queue->head = queue->head->next; /* set up second as head */
	queue->tail = queue->tail->next; /* set up old head as tail */
	queue->tail->next = NULL; /* break tail to head connection */
}

struct aathread *aaqueue_get_head(struct aaqueue *queue) {
	if (queue == NULL || queue->head == NULL)
		return NULL;

	return queue->head->thread;
}

struct aathread *aaqueue_get_tail(struct aaqueue *queue) {
	if (queue == NULL || queue->tail == NULL)
		return NULL;

	return queue->tail->thread;
}
