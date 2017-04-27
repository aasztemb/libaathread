void aathread_init(void);
void aathread_finish(void);

int aathread_start(void (*start_routine)(void *), void *arg);
void aathread_exit(void);
int aathread_kill(int tid);
