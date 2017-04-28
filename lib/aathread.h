void aathread_init(void);
void aathread_finish(void);

int aathread_start(void (*start_routine)(void *), void *arg);
void aathread_exit(void);
int aathread_kill(int tid);

int aamutex_create(void);
void aamutex_destroy(int mid);

void aamutex_lock(int mid);
void aamutex_unlock(int mid);
