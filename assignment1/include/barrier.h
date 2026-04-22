#ifndef MY_PTHREAD_H
#define MY_PTHREAD_H

#define _GNU_SOURCE

#include <pthread.h>

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <stdbool.h>

/**
 * Deferent question (i), (ii), (iii) will have there one Thread Functions
**/

/* Macro to find lower between 2 numbers. Keep address */
#define MAX(a, b) ((a) < (b) ? &(b) : &(a))

/* Macro to find higher between 2 numbers. Keep address */
#define MIN(a, b) ((a) < (b) ? &(a) : &(b))

/* Macro to handle errors (print and exit) */
#define HANDLE_ERROR(msg){  \
    perror(msg);            \
    exit(EXIT_FAILURE);     \
}

/** 
 * Macro to handle error when tring to work with Pthread functions (mutex_lock, pthread_creat, ect).
 * All return 0 on success 
 * Save return value to "errno" so i can see the problem
**/
#define PTHREAD_CHECK(function, msg){   \
    int res = function;                 \
    errno = res;                        \
    if(res != 0) {                      \
        HANDLE_ERROR(msg)               \
    }                                   \
}

/* (i) barrier, (ii) condition, (iii) mybarrier */
typedef enum _policy{barrier, condition, mybarrier, sense_reversal_barrier} _policy;
// -------------------------------------------------------


/**
 * Security. Make sure the user give input in right order and is valid.
 * Execute with ./<executable> <loops number> <threads number> <policy number>
 * Policy:
 *  (0) barrier
 *  (1) condition variables
 *  (2) my custom barrier
 *  (3) sense-reversal barrier
**/
/* Check that given right number of arguments */
void security_user_input(int agrc, char **argv);



/* Question (i) implemetetion */
typedef struct _shared_data_barrier{
    int N;  // for loops
    pthread_barrier_t barrier;
}_shared_data_barrier;

/* Create-Malloc barrier shared data. Do not forget to free */
_shared_data_barrier *create_shared_data_barrier(const int N, const int threads_num);

/* Free barrier shared data. Destroy what initialized */
void destroy_shared_data_barrier(_shared_data_barrier *sdb);

/* The Barrier thread. Does all the work */
void *Barrier_thread(void *args);
// -------------------------------------------------------


/* Question (ii) implemetetion */
typedef struct _shared_data_condition{
    int N;                  // for loops
    int threshold;          // Number of threads that must w8 in "barrier" aka. total threads
    int threads_waiting;    // Number of threads that w8ing in "barrier"
    pthread_mutex_t mutex;
    pthread_cond_t cond;
}_shared_data_condition;

/* Create-Malloc condition shared data. Do not forget to free */
_shared_data_condition *create_shared_data_condition(const int N, const int threads_num);

/* Free condition shared data. Destroy what initialized */
void destroy_shared_data_condition(_shared_data_condition *sdc);

/* The Condition thread. Does all the work */
void *Condition_thread(void *args);

/* The "barrier" function. Does all the synchronization */
void thread_condition_barrier_wait(_shared_data_condition *sdc);
// -------------------------------------------------------


/* Question (iii) implemetetion (MY WAY) */

/**
 * --------------- KEY IDEA ---------------
 * We have 2 counter for threshold. aka a,b and lets say 4 total threads we must barrier.
 * At the start (init) a = 0 and b = 4. Then in sincronization we do this policy.
 * All exept last thread:
 *  (all below (a,b) with a pointer)
 *  (1) take the lower of (a,b) do it ++
 *  (2) go to while lower of (a,b) is less than threshold (beasy w8ting)
 * Last thread:
 *  (all below (a,b) with a pointer)
 *  (1) take the higher of (a,b) do it 0
 *  (2) take the higher of (a,b) do it ++
 *  (3) do not go to while loop
 * 
 * The above policy is resuable becuse we ensure that when we set the higher to 0, there are not
 * ather threads leftover in the previus barier. If was then (condition < threshhold) would not work
 * couse contition will go from 4->0 
 *  (4 < 4) false, so we continue (leave "barrier")
 *  (0 < 4) true, so we keep do (beasy w8ting in "barrier")
 **/
typedef struct _shared_data_mybarrier{
    int N;                  // for loops
    int threshold;          // Number of threads that must w8 in "barrier" aka. total threads
    int threads_waiting_1;  // Number of threads that w8ing in "first barrier"
    int threads_waiting_2;  // Number of threads that w8ing in "secont barrier"
    pthread_mutex_t mutex;
}_shared_data_mybarrier;

/* Create-Malloc mybarrier shared data. Do not forget to free */
_shared_data_mybarrier *create_shared_data_mybarrier(const int N, const int threads_num);

/* Free mybarrier shared data. Destroy what initialized */
void destroy_shared_data_mybarrier(_shared_data_mybarrier *sdmb);

/* The MyBarrier thread. Does all the work */
void *MyBarrier_thread(void *args);

/* The "barrier" function. Does all the synchronization */
void thread_mybarrier_wait(_shared_data_mybarrier *sdmb);
// -------------------------------------------------------



/* Question (iii) implemetetion (sense-reversal) */
typedef struct _shared_data_sense_reversal{
    int N;
    int threshold;
    int threads_waiting;
    bool flag;
    pthread_mutex_t mutex;
}_shared_data_sense_reversal;

/* Create-Malloc sense-reversal shared data. Do not forget to free */
_shared_data_sense_reversal *create_shared_data_sense_reversal(const int N, const int threads_num);

/* Free sense-reversal shared data. Destroy what initialized */
void destroy_shared_data_sense_reversal(_shared_data_sense_reversal *sdsr);

/* The MyBarrier thread. Does all the work */
void *SenseReversal_thread(void *args);

/* The "barrier" function. Does all the synchronization */
void thread_sense_reversal_wait(_shared_data_sense_reversal *sdsr);

#endif