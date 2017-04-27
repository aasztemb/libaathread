struct aaqueue;

struct aaqueue *aaqueue_new(void);
void aaqueue_destroy(struct aaqueue *queue);

int aaqueue_push_tail(struct aaqueue *queue, struct aathread *thread);
struct aathread *aaqueue_pop_head(struct aaqueue *queue);
struct aathread *aaqueue_pop_thread_by_id(struct aaqueue *queue, int tid);

void aaqueue_relink_parent_contexts(struct aaqueue *queue, ucontext_t *old_context, ucontext_t *new_context);

void aaqueue_head_to_tail(struct aaqueue *queue);

struct aathread *aaqueue_get_thread_by_id(struct aaqueue *queue, int tid);
struct aathread *aaqueue_get_head(struct aaqueue *queue);
struct aathread *aaqueue_get_tail(struct aaqueue *queue);
