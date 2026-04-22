#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int count;

pthread_mutex_t lock;

void *increment_count_mutex(void *arg) {
    int iters = *(int *)arg;
    for (int i = 0; i < iters; i++) {
        pthread_mutex_lock(&lock);
        count++;
        pthread_mutex_unlock(&lock);
    }
    return NULL;
}

pthread_rwlock_t rwlock;

void *increment_count_rwlock(void *arg) {
    int iters = *(int *)arg;
    for (int i = 0; i < iters; i++) {
        pthread_rwlock_wrlock(&rwlock);
        count++;
        pthread_rwlock_unlock(&rwlock);
    }
    return NULL;
}

void *increment_count_atomic(void *arg) {
    int iters = *(int *)arg;
    for (int i = 0; i < iters; i++) {
        __atomic_add_fetch(&count, 1, __ATOMIC_SEQ_CST);
    }
    return NULL;
}

double get_time(struct timespec *tic, struct timespec *toc){
    return (toc->tv_sec - tic->tv_sec) + (toc->tv_nsec - tic->tv_nsec) / 1e9;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <iterations> <thread_count>\n", argv[0]);
        return 1; }
    int iterations = atoi(argv[1]);
    if (iterations <= 0) {
        fprintf(stderr, "Number of iterations must be positive\n");
        return 1;
    
    }
    int thread_count = atoi(argv[2]);
    if (thread_count <= 0) {
        fprintf(stderr, "Number of threads must be positive\n");
        return 1;
    }

    struct timespec tic, toc;
    count = 0;
    pthread_t threads[thread_count];
    pthread_mutex_init(&lock, NULL);
    int iters_per_thread = iterations / thread_count;
    int final_iters = iters_per_thread + (iterations % thread_count);

    clock_gettime(CLOCK_MONOTONIC, &tic);
    for (int i = 0; i < thread_count; i++) {
        if (pthread_create(&threads[i], NULL, increment_count_mutex, (void*)( i != thread_count - 1 ? &iters_per_thread : &final_iters)) != 0) {
            fprintf(stderr, "Thread %d creation error\n", i);
            return 1;
        }
    }
    for (int i = 0; i < thread_count; i++) {
        pthread_join(threads[i], NULL);
    }
    clock_gettime(CLOCK_MONOTONIC, &toc);
    pthread_mutex_destroy(&lock);

    printf("Mutex - Count: %d, Time: %.6f seconds\n", count, get_time(&tic, &toc));

    count = 0;
    pthread_rwlock_init(&rwlock, NULL);

    clock_gettime(CLOCK_MONOTONIC, &tic);
    for (int i = 0; i < thread_count; i++) {
         if (pthread_create(&threads[i], NULL, increment_count_rwlock, (void*)( i != thread_count - 1 ? &iters_per_thread : &final_iters)) != 0) {
            fprintf(stderr, "Thread %d creation error\n", i);
            return 1;
        }
    }
    for (int i = 0; i < thread_count; i++) {
        pthread_join(threads[i], NULL);
    }
    clock_gettime(CLOCK_MONOTONIC, &toc);
    pthread_rwlock_destroy(&rwlock);

    printf("RWLock - Count: %d, Time: %.6f seconds\n", count, get_time(&tic, &toc));

    count = 0;

    clock_gettime(CLOCK_MONOTONIC, &tic);
    for (int i = 0; i < thread_count; i++) {
        if (pthread_create(&threads[i], NULL, increment_count_atomic, (void*)( i != thread_count - 1 ? &iters_per_thread : &final_iters)) != 0) {
            fprintf(stderr, "Thread %d creation error\n", i);
            return 1;
        }
    }
    for (int i = 0; i < thread_count; i++) {
        pthread_join(threads[i], NULL);
    }
    clock_gettime(CLOCK_MONOTONIC, &toc);

    printf("Atomic - Count: %d, Time: %.6f seconds\n", count, get_time(&tic, &toc));
    return 0;
}
