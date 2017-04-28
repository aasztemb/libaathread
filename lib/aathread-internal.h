struct aathread {
	int tid;
	ucontext_t context;
	void (*start_routine)(void *);
	void *arg;
};

extern struct aaqueue *runnable_queue;

struct aaslist;
extern struct aaslist *mutex_list;

void dispose_threads(void);
