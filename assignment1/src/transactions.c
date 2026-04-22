#include "transactions.h"
#include <time.h>

int main(int argc, char *argv[]) {
    /* initialization */
    int policy = hello(argc, argv);
    int return_code = 0;
    int check_sum = 0;
    unsigned int seed = time(NULL);
    struct timespec tic, toc;

    switch (policy) {
        case 1:
            goto coarse_mutex;
            break;
        case 2:
            goto fine_mutex;
            break;
        case 3:
            goto coarse_rwlock;
            break;
        case 4:
            goto fine_rwlock;
            break;
        default:
            fprintf(stderr, "Invalid policy\n");
            return_code = 1;
            goto cleanup;
            break;
    }

coarse_mutex:
    /* coarse grained mutex */
    pthread_mutex_init(&mutex, NULL);
    clock_gettime(CLOCK_MONOTONIC, &tic);
    for (int i = 0; i < thread_count; i++) {
        if (pthread_create(&threads[i], NULL, coarse_mutex, (void*)&seed) != 0) {
            fprintf(stderr, "Thread %d creation failed\n", i);
            return_code = 1;
            goto cleanup;
        }
    }
    for (int i = 0; i < thread_count; i++) {
        pthread_join(threads[i], NULL);
    }
    clock_gettime(CLOCK_MONOTONIC, &toc);
    pthread_mutex_destroy(&mutex);
    printf("Execution time: %.6f sec\n", get_time(&tic, &toc));
    goto validation;

fine_mutex:
    /* fine grained mutex setup */
    cell_mutex = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t) * accounts_count);
    if (cell_mutex == NULL) {
        fprintf(stderr, "Failed to allocate mutex array\n");
        return_code = 1;
        goto cleanup;
    }
    for(int i = 0; i < accounts_count; i++) {
        pthread_mutex_init(&cell_mutex[i], NULL);
    }
    clock_gettime(CLOCK_MONOTONIC, &tic);
    for (int i = 0; i < thread_count; i++) {
        if (pthread_create(&threads[i], NULL, fine_mutex, (void*)&seed) != 0) {
            fprintf(stderr, "Thread %d creation failed\n", i);
            return_code = 1;
            goto cleanup;
        }
    }
    for (int i = 0; i < thread_count; i++) {
        pthread_join(threads[i], NULL);
    }
    clock_gettime(CLOCK_MONOTONIC, &toc);
    for(int i = 0; i < accounts_count; i++) {
        pthread_mutex_destroy(&cell_mutex[i]);
    }
    free(cell_mutex);
    printf("Execution time: %.6f sec\n", get_time(&tic, &toc));
    goto validation;

coarse_rwlock:
    pthread_rwlock_init(&rwlock, NULL);
    clock_gettime(CLOCK_MONOTONIC, &tic);
    for (int i = 0; i < thread_count; i++) {
        if (pthread_create(&threads[i], NULL, coarse_rwlock, (void*)&seed) != 0) {
            fprintf(stderr, "Thread %d creation failed\n", i);
            return_code = 1;
            goto cleanup;
        }
    }
    for (int i = 0; i < thread_count; i++) {
        pthread_join(threads[i], NULL);
    }
    clock_gettime(CLOCK_MONOTONIC, &toc);
    pthread_rwlock_destroy(&rwlock);
    printf("Execution time: %.6f sec\n", get_time(&tic, &toc));
    goto validation;

fine_rwlock:
    /* fine grained mutex setup */
    cell_rwlock = (pthread_rwlock_t *)malloc(sizeof(pthread_rwlock_t) * accounts_count);
    if (cell_rwlock == NULL) {
        fprintf(stderr, "Failed to allocate rwlock array\n");
        return_code = 1;
        goto cleanup;
    }
    for(int i = 0; i < accounts_count; i++) {
        pthread_rwlock_init(&cell_rwlock[i], NULL);
    }
    clock_gettime(CLOCK_MONOTONIC, &tic);
    for (int i = 0; i < thread_count; i++) {
        if (pthread_create(&threads[i], NULL, fine_rwlock, (void*)&seed) != 0) {
            fprintf(stderr, "Thread %d creation failed\n", i);
            return_code = 1;
            goto cleanup;
        }
    }
    for (int i = 0; i < thread_count; i++) {
        pthread_join(threads[i], NULL);
    }
    clock_gettime(CLOCK_MONOTONIC, &toc);
    for(int i = 0; i < accounts_count; i++) {
        pthread_rwlock_destroy(&cell_rwlock[i]);
    }
    free(cell_rwlock);
    printf("Execution time: %.6f sec\n", get_time(&tic, &toc));

validation:
    /* result validation and cleanup */
    for (int i = 0; i < accounts_count; i++) {
        check_sum += accounts[i];
    }
    if (check_sum != initial_sum) {
        fprintf(stderr, "Incorrect results\n");
        return_code = 1;
        goto cleanup;
    }
    printf("Success!\n");

cleanup:
    free(threads);
    free(accounts);
    return return_code;
}
