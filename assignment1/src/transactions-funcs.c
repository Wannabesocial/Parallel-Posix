#include "transactions.h"

/* Global variable definitions */
int accounts_count;
int queries;
int transfers;
int thread_count;
int *accounts;
int initial_sum;
pthread_t *threads;
pthread_mutex_t mutex;
pthread_mutex_t *cell_mutex;
pthread_rwlock_t rwlock;
pthread_rwlock_t *cell_rwlock;

int hello(int argc, char *argv[]) {
    if (argc != 6) {
        fprintf(stderr, "Usage: %s <accounts> <transfers> <percentage-of-queries> <policy> <thread-count>\n", argv[0]);
        return 5;
    }
    accounts_count = atoi(argv[1]);
    int transaction_count = atoi(argv[2]);
    int query_percentage = atoi(argv[3]);
    int policy = atoi(argv[4]);
    thread_count = atoi(argv[5]);
    accounts = (int *)malloc(sizeof(int) * accounts_count);
    if (accounts == NULL) {
        fprintf(stderr, "Failed to allocate accounts array\n");
        return 5;
    }
    srand(time(NULL));
    initial_sum = 0;
    for (int i = 0; i < accounts_count; i++) {
        accounts[i] = 10000 + (rand() % 10001);
        initial_sum += accounts[i];
    }
    queries = (query_percentage / 100) * transaction_count;
    transfers = transaction_count - queries;
    threads = (pthread_t *)malloc(sizeof(pthread_t) * thread_count);
    if (threads == NULL) {
        fprintf(stderr, "Failed to allocate accounts array\n");
        free(accounts);
        return 5;
    }
    return policy;
}



void *coarse_mutex(void *arg) {
    unsigned int seed = *(unsigned int *)arg;
    int local_queries = queries;
    int local_transfers = transfers;
    int local_sum = 0;
    int transfer_amount;
    int source_account;
    int dest_account = 0;
    for (int i = 0; i < queries + transfers; i++) {
        source_account = rand_r(&seed) % (accounts_count - 1);
        pthread_mutex_lock(&mutex);
        local_sum += accounts[source_account];
        if (local_transfers == 0 || (rand_r(&seed) % 2 == 0 && local_queries > 0)) {
            local_queries--;
            pthread_mutex_unlock(&mutex);
            continue;
        }
        dest_account = rand_r(&seed) % (accounts_count - 1);
        while (dest_account == source_account) {
            dest_account = rand_r(&seed) % accounts_count;
        }
        local_sum += accounts[dest_account];
        transfer_amount = rand_r(&seed) % accounts[source_account];
        accounts[source_account] -= transfer_amount;
        accounts[dest_account] += transfer_amount;
        pthread_mutex_unlock(&mutex);
        local_transfers--;
    }

    if (local_transfers != 0 || local_queries != 0) {
        fprintf(stderr, "Not all transactions where completed correctly\n");
    }
    return NULL;
}

void *fine_mutex(void *arg) {
    unsigned int seed = *(unsigned int *)arg;
    int local_queries = queries;
    int local_transfers = transfers;
    int local_sum = 0;
    int transfer_amount;
    int source_account;
    int dest_account;
    for (int i = 0; i < queries + transfers; i++) {
        source_account = rand_r(&seed) % accounts_count;
        if (local_transfers == 0 || (rand_r(&seed) % 2 == 0 && local_queries > 0)) {
            pthread_mutex_lock(&cell_mutex[source_account]);
            local_sum += accounts[source_account];
            local_queries--;
            pthread_mutex_unlock(&cell_mutex[source_account]);
            continue;
        }
        dest_account = rand_r(&seed) % accounts_count;
        while (dest_account == source_account) {
            dest_account = rand_r(&seed) % accounts_count;
        }
        if (dest_account < source_account) {
            pthread_mutex_lock(&cell_mutex[dest_account]);
            pthread_mutex_lock(&cell_mutex[source_account]);
        } else {
            pthread_mutex_lock(&cell_mutex[source_account]);
            pthread_mutex_lock(&cell_mutex[dest_account]);
        }
        local_sum += accounts[source_account];
        local_sum += accounts[dest_account];
        transfer_amount = rand_r(&seed) % accounts[source_account];
        accounts[source_account] -= transfer_amount;
        accounts[dest_account] += transfer_amount;
        pthread_mutex_unlock(&cell_mutex[dest_account]);
        pthread_mutex_unlock(&cell_mutex[source_account]);
        local_transfers--;
    }

    if (local_transfers != 0 || local_queries != 0) {
        fprintf(stderr, "Not all transactions where completed correctly\n");
    }
    return NULL;
}

void *coarse_rwlock(void *arg) {
    unsigned int seed = *(unsigned int *)arg;
    int local_queries = queries;
    int local_transfers = transfers;
    int local_sum = 0;
    int transfer_amount;
    int source_account;
    int dest_account;
    for (int i = 0; i < queries + transfers; i++) {
        source_account = rand_r(&seed) % accounts_count;
        if (local_transfers == 0 || (rand_r(&seed) % 2 == 0 && local_queries > 0)) {
            pthread_rwlock_rdlock(&rwlock);
            local_sum += accounts[source_account];
            local_queries--;
            pthread_rwlock_unlock(&rwlock);
            continue;
        }
        dest_account = rand_r(&seed) % accounts_count;
        while (dest_account == source_account) {
            dest_account = rand_r(&seed) % accounts_count;
        }
        pthread_rwlock_wrlock(&rwlock);
        local_sum += accounts[dest_account];
        local_sum += accounts[source_account];
        transfer_amount = rand_r(&seed) % accounts[source_account];
        accounts[source_account] -= transfer_amount;
        accounts[dest_account] += transfer_amount;
        pthread_rwlock_unlock(&rwlock);
        local_transfers--;
    }

    if (local_transfers != 0 || local_queries != 0) {
        fprintf(stderr, "Not all transactions where completed correctly\n");
    }
    return NULL;
}

void *fine_rwlock(void *arg) {
    unsigned int seed = *(unsigned int *)arg;
    int local_queries = queries;
    int local_transfers = transfers;
    int local_sum = 0;
    int transfer_amount;
    int source_account;
    int dest_account;
    for (int i = 0; i < queries + transfers; i++) {
        source_account = rand_r(&seed) % accounts_count;
        if (local_transfers == 0 || (rand_r(&seed) % 2 == 0 && local_queries > 0)) {
            pthread_rwlock_rdlock(&cell_rwlock[source_account]);
            local_sum += accounts[source_account];
            local_queries--;
            pthread_rwlock_unlock(&cell_rwlock[source_account]);
            continue;
        }
        dest_account = rand_r(&seed) % accounts_count;
        while (dest_account == source_account) {
            dest_account = rand_r(&seed) % accounts_count;
        }
        if (dest_account < source_account) {
            pthread_rwlock_wrlock(&cell_rwlock[dest_account]);
            pthread_rwlock_wrlock(&cell_rwlock[source_account]);
        } else {
            pthread_rwlock_wrlock(&cell_rwlock[source_account]);
            pthread_rwlock_wrlock(&cell_rwlock[dest_account]);
        }
        local_sum += accounts[source_account];
        local_sum += accounts[dest_account];
        transfer_amount = rand_r(&seed) % accounts[source_account];
        accounts[source_account] -= transfer_amount;
        accounts[dest_account] += transfer_amount;
        pthread_rwlock_unlock(&cell_rwlock[dest_account]);
        pthread_rwlock_unlock(&cell_rwlock[source_account]);
        local_transfers--;
    }

    if (local_transfers != 0 || local_queries != 0) {
        fprintf(stderr, "Not all transactions where completed correctly\n");
    }
    return NULL;

}

double get_time(struct timespec *tic, struct timespec *toc){
    return (toc->tv_sec - tic->tv_sec) + (toc->tv_nsec - tic->tv_nsec) / 1e9;
}

