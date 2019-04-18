#include "pthread.h"

static void pthread_print(const char* func_name) {
	printf("pthread_%s\n", func_name);
}

pthread_t pthread_self(void) {
	// pthread_print("self");
}

int pthread_once (void *once_control, void (*init_routine) (void)) {
	init_routine();
}

int pthread_create(pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine) (void *), void *arg) {
	pthread_print("create");
}

// YOLO thread-safety (for the moment at least)

int pthread_mutex_destroy(pthread_mutex_t *mutex) {}
int pthread_mutex_init(pthread_mutex_t *restrict mutex, const pthread_mutexattr_t *restrict attr) {}

int pthread_cond_destroy(pthread_cond_t *cond) {}
int pthread_cond_init(pthread_cond_t *restrict cond, const pthread_condattr_t *restrict attr) {}

int pthread_cond_timedwait(pthread_cond_t *restrict cond, pthread_mutex_t *restrict mutex, const struct timespec *restrict abstime) {
	pthread_print("cond_timedwait");
}
int pthread_cond_wait(pthread_cond_t *restrict cond, pthread_mutex_t *restrict mutex) {
	pthread_print("cond_wait");
}

int pthread_cond_broadcast(pthread_cond_t *cond) {
	//pthread_print("cond_broadcast");
}
int pthread_cond_signal(pthread_cond_t *cond) {
	pthread_print("cond_signal");
}

int pthread_join(pthread_t thread, void **retval) {
	pthread_print("join");
}

int pthread_mutex_lock(pthread_mutex_t *mutex) {
	//pthread_print("mutex_lock");
}
int pthread_mutex_trylock(pthread_mutex_t *mutex) {
	pthread_print("mutex_trylock");
}
int pthread_mutex_unlock(pthread_mutex_t *mutex) {
	//pthread_print("mutex_unlock");
}
