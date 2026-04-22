#define _GNU_SOURCE

#include <stdio.h>
#include <sys/time.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>


extern int accounts_count;
extern int queries;
extern int transfers;
extern int thread_count;
extern int *accounts;

extern int initial_sum;

extern pthread_t *threads;

int hello(int argc, char *argv[]);

/* coarse grained mutex */
extern pthread_mutex_t mutex;
void *coarse_mutex(void *arg);

/* fine grained mutex */
extern pthread_mutex_t *cell_mutex;
void *fine_mutex(void *arg);

/* coarse grained rwlock */
extern pthread_rwlock_t rwlock;
void *coarse_rwlock(void *arg);

/* fine grained rwlock */
extern pthread_rwlock_t *cell_rwlock;
void *fine_rwlock(void *arg);

double get_time(struct timespec *tic, struct timespec *toc);
