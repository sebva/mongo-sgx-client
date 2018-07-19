#include "pthread.h"

int pthread_once (void *once_control, void (*init_routine) (void)) {
	init_routine();
}

// YOLO thread-safety (for the moment at least)

int pthread_mutex_destroy(pthread_mutex_t *mutex) {}
int pthread_mutex_init(pthread_mutex_t *restrict mutex, const pthread_mutexattr_t *restrict attr) {}

int pthread_cond_destroy(pthread_cond_t *cond) {}
int pthread_cond_init(pthread_cond_t *restrict cond, const pthread_condattr_t *restrict attr) {}

int pthread_cond_timedwait(pthread_cond_t *restrict cond, pthread_mutex_t *restrict mutex, const struct timespec *restrict abstime) {}
int pthread_cond_wait(pthread_cond_t *restrict cond, pthread_mutex_t *restrict mutex) {}

int pthread_cond_broadcast(pthread_cond_t *cond) {}
int pthread_cond_signal(pthread_cond_t *cond) {}

int pthread_join(pthread_t thread, void **retval) {}

int pthread_mutex_lock(pthread_mutex_t *mutex) {}
int pthread_mutex_trylock(pthread_mutex_t *mutex) {}
int pthread_mutex_unlock(pthread_mutex_t *mutex) {}
