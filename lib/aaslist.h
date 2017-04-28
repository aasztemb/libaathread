struct aathread;
struct aamutex;

struct aaslist {
	union {
		struct aathread *thread;
		struct aamutex *mutex;
	};
	struct aaslist *next;
};
