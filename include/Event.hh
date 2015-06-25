#include <pthread.h>

class Event {
private:
  cond_t event;

  // in order to wait on a condition variable you must have a locked
  // mutex, which we don't really need.
  mutex_t dummy_mutex;

public:
  Event(void) {
    cond_init(&event, USYNC_PROCESS, (void*)0);
    mutex_init(&dummy_mutex, USYNC_PROCESS, (void*)0);
  }

  ~Event(void) {
    event_destroy(&event);
  }

  void signal(void) {
    cond_broadcast(&event);
  }

  void wait(void) {
    mutex_lock(&dummy_mutex);
    cond_wait(&event, &dummy_mutex);
    mutex_unlock(&dummy_mutex);
  }
};
