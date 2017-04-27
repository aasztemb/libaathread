struct aathread {
	int tid;
	ucontext_t context;
	void (*start_routine)(void *);
	void *arg;
};

extern struct aaqueue *runnable_queue;
void dispose_threads(void);
