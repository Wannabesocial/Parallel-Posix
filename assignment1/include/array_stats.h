#ifndef ARRAY_STATS_H 
#define ARRAY_STATS_H 

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

/**
 *
 *  We also provide a padded
 *  array struct assuming
 *  64-byte cacheline in order
 *  to try and combat the false
 *  sharing phenomenon in the
 *  original struct implementation.
 *
 *  To use just add -DIMPROVED
 *  during compilation.
 *
 **/

#if defined(IMPROVED)

struct array_stats {
    long long int info_array_0;
    long long int padding0[7]; 
    long long int info_array_1;
    long long int padding1[7];
    long long int info_array_2;
    long long int padding2[7];
    long long int info_array_3;
    long long int padding3[7];
};

#else

struct array_stats {
    long long int info_array_0; 
    long long int info_array_1;
    long long int info_array_2;
    long long int info_array_3;
};

#endif

extern struct array_stats parallel_stats;
extern struct array_stats serial_stats;


extern int *array0;
extern int *array1;
extern int *array2;
extern int *array3;

extern int array_size;
extern pthread_t threads[4];

typedef struct {
    int array_index;
} thread_args_t;


/**
 *
 *  Macros for analyzing and
 *  initializing arrays
 *
 *  other methods would lead
 *  to constant dereferencing,
 *  branches within the hot
 *  path or severe code
 *  repetition.
 *
 **/
#define ANALYZE_PARALLEL(INDEX) \
    for (int i = 0; i < array_size; i++) { \
        if (array##INDEX[i] != 0) { \
            parallel_stats.info_array_##INDEX++; \
        } \
    }

#define ANALYZE_SERIAL(INDEX) \
    for (int i = 0; i < array_size; i++) { \
        if (array##INDEX[i] != 0) { \
            serial_stats.info_array_##INDEX++; \
        } \
    }

#define INIT_SINGLE_ARRAY(ARRAY_PTR, INDEX) \
    ARRAY_PTR = (int *)malloc(array_size * sizeof(int)); \
    if (ARRAY_PTR == NULL) { \
        fprintf(stderr, "Error allocating array%d\n", INDEX); \
        return 1; \
    } \
    for (int j = 0; j < array_size; j++) { \
        ARRAY_PTR[j] = rand() % 10; \
    }

#define INITIALIZE_ALL_ARRAYS() \
    do { \
        INIT_SINGLE_ARRAY(array0, 0); \
        INIT_SINGLE_ARRAY(array1, 1); \
        INIT_SINGLE_ARRAY(array2, 2); \
        INIT_SINGLE_ARRAY(array3, 3); \
    } while(0)


void *analyze_array_parallel(void *arg);
double get_time(struct timespec *tic, struct timespec *toc);

#endif
