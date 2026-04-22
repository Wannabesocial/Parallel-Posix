#include "array_stats.h" 

struct array_stats parallel_stats = {0}; 
struct array_stats serial_stats = {0};

int *array0;
int *array1;
int *array2;
int *array3;

int array_size;

/**
 *
 *  To analyze the arrays we use macros
 *  in order to directly access the fields
 *  and avoid code repetition.
 *
 *  We also make use of macros for the
 *  initialization step.
 *
 *  arrays and stats are kept global.
 *
 *  Timers work with clock_gettime(CLOCK_MONOTONIC).
 *
 **/
void *analyze_array_parallel(void *arg) {
    int idx = *(int*)arg;
    switch (idx) {
        case 0: ANALYZE_PARALLEL(0); break;
        case 1: ANALYZE_PARALLEL(1); break;
        case 2: ANALYZE_PARALLEL(2); break;
        case 3: ANALYZE_PARALLEL(3); break;
    }
    return NULL;
}

double get_time(struct timespec *tic, struct timespec *toc){
    return (toc->tv_sec - tic->tv_sec) + (toc->tv_nsec - tic->tv_nsec) / 1e9;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <array_size>\n", argv[0]);
        return 1;
    }
    array_size = atoi(argv[1]);
    if (array_size <= 0) {
        fprintf(stderr, "Array size must be positive\n");
        return 1;
    }
    
    struct timespec tic, toc;
    clock_gettime(CLOCK_MONOTONIC, &tic);
    srand(time(NULL));
    INITIALIZE_ALL_ARRAYS();
    clock_gettime(CLOCK_MONOTONIC, &toc);

    printf("Array Initialization time: %.6f seconds\n\n", get_time(&tic, &toc));

      
    pthread_t threads[4];
    clock_gettime(CLOCK_MONOTONIC, &tic);
    for (int i = 0; i < 4; i++) {
        if (pthread_create(&threads[i], NULL, analyze_array_parallel, (void*)&i) != 0) {
            fprintf(stderr, "Thread %d creation error\n", i);
            return 1;
        }
    }
    for (int i = 0; i < 4; i++) {
        pthread_join(threads[i], NULL);
    }

    clock_gettime(CLOCK_MONOTONIC, &toc);
    printf("Parallel stat count time: %.6f seconds\n\n", get_time(&tic, &toc));
  
    serial_stats = (struct array_stats){0};
    clock_gettime(CLOCK_MONOTONIC, &tic);

    ANALYZE_SERIAL(0);
    ANALYZE_SERIAL(1);
    ANALYZE_SERIAL(2);
    ANALYZE_SERIAL(3);

    clock_gettime(CLOCK_MONOTONIC, &toc);
    printf("Serial stat count time: %.6f seconds\n\n", get_time(&tic, &toc));

    if (serial_stats.info_array_0 == parallel_stats.info_array_0 &&
        serial_stats.info_array_1 == parallel_stats.info_array_1 &&
        serial_stats.info_array_2 == parallel_stats.info_array_2 &&
        serial_stats.info_array_3 == parallel_stats.info_array_3) {
        printf("Correct results\n");
    } else {
        printf("Incorrect results\n");
    }

    free(array0);
    free(array1);
    free(array2);
    free(array3);
    return 0;
}
