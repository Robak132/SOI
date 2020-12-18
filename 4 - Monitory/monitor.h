#ifndef MONITOR_H
#define MONITOR_H

#include <sys/types.h> 
#include <sys/stat.h> 
#include <string.h> 
#include <errno.h> 
#include <fcntl.h> 
#include <pthread.h> 
#include <unistd.h>
#include <semaphore.h>

class Semaphore {
public:
    Semaphore(int value) {
       sem_init(&sem, 0, value);
    }
    ~Semaphore() {
        sem_destroy(&sem);
    }
    void p() {
        sem_wait(&sem);
    }
    void v() {
        sem_post(&sem);
    }
private:
    sem_t sem;
};

class Condition {
friend class Monitor;

public:
	Condition() : w(0) {
		waitingCount = 0;
	}
	void wait() {
		w.p();
	}
	bool signal() {
		if(waitingCount) {
			-- waitingCount;
			w.v();
			return true;
		}
		else
			return false;
	}
private:
	Semaphore w;
	int waitingCount;
};

class Monitor {
public:
	Monitor() : s(1) {}
	void enter() {
		s.p();
	}
	void leave() {
		s.v();
	}
	void wait(Condition &cond) {
		++ cond.waitingCount;
		leave();
		cond.wait();
	}
	void signal(Condition &cond) {
		if( cond.signal() )
			enter();
	}
private:
	Semaphore s;
};
#endif