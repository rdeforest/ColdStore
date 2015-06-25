extern "C" {
#include <pthread.h>
    // there's a bug in pthread.h where this isn't declared.
    extern int pthread_mutexattr_setkind_np(pthread_mutexattr_t *__attr,
                                            int __kind);
}
class Gate {
private:
    pthread_mutex_t lock;
    pthread_cond_t  wakeup;
    int waiting;	// how many tasks are waiting
    bool closed;
    
public:
    Gate(bool opened=false)
        : closed(!opened)
        {
            pthread_mutexattr_t mattr;
            pthread_mutexattr_init(&mattr);
            pthread_mutexattr_setkind_np(&mattr, PTHREAD_MUTEX_RECURSIVE_NP);
            
            pthread_mutex_init(&lock, &mattr);
            pthread_cond_init(&wakeup, NULL);
        }
    
    virtual ~Gate(void) {
        pthread_mutex_destroy(&lock);
        pthread_cond_destroy(&wakeup);
    }
    
    bool open(void) {
        pthread_mutex_lock(&lock);
        bool was = closed;
        if (closed) {
            closed = false;
            pthread_cond_broadcast(&wakeup);
            waiting = 0;
        }
        pthread_mutex_unlock(&lock);
        return !was;
    }
    
    bool close(void) {
        pthread_mutex_lock(&lock);
        bool was = closed;
        closed = true;
        pthread_mutex_unlock(&lock);
        return !was;
    }
    
    // temporarily open the gate, allowing any waiting threads to
    // proceed
    void release(void) {
        pthread_mutex_lock(&lock);
        waiting=0;
        pthread_cond_broadcast(&wakeup);
        pthread_mutex_unlock(&lock);
    }
    
    void wait(void) {
        pthread_mutex_lock(&lock);
        if (closed) {
            waiting++;
            pthread_cond_wait(&wakeup, &lock);
        }
        pthread_mutex_unlock(&lock);
    }

    int blocked(void) {
        pthread_mutex_lock(&lock);
        return waiting;
        pthread_mutex_unlock(&lock);
    }
};
