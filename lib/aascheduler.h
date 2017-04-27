void schedule_timer(void);
void cancel_timer(void);

void enable_signals(sigset_t *oldmask);
void disable_signals(sigset_t *oldmask);
/* void remove_scheduling_signal_from_mask(sigset_t *mask); */
int is_scheduling_signal_blocked(void);
void force_unblock_scheduling_signal(void);

void reshedule(struct aathread *previous_thread);
