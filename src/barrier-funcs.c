#include "barrier.h"
#include <unistd.h>

// ---------------- SECURITY ---------------------

void security_user_input(int argc, char **argv){

    int N, threads_num , policy;

    if(argc != 4){ // Problem: Not enough arguments
        HANDLE_ERROR("Usage: ./ex5 <loops number> <threads number> <policy number>");
    }

    N = atoi(argv[1]), threads_num = atoi(argv[2]), policy = atoi(argv[3]);

    if(N < 0 || threads_num < 0 || policy < 0){ // Problem: Negative values
        HANDLE_ERROR("You must not give negatives values");
    }

    if(N == 0 || threads_num == 0){ // Problem: Zero values
        HANDLE_ERROR("Program does not have any meaning with zero Loops or Threads");
    }

    if(policy > 3){ // Problem: No such policy. Policy range [0,3]
        HANDLE_ERROR("No such Policy. Give Policy number 0-3");
    }
}


// ---------------- BARRIER ----------------------

void *Barrier_thread(void *args){

    _shared_data_barrier *sdb = (_shared_data_barrier *)args;
    int N = sdb->N;

    for(int i = 0; i < N; i++){
        pthread_barrier_wait(&sdb->barrier);
    }

    return NULL;
}

_shared_data_barrier *create_shared_data_barrier(const int N, const int threads_num){

    // Take memory
    _shared_data_barrier *sdb = (_shared_data_barrier *) malloc(sizeof(_shared_data_barrier));
    if(sdb == NULL){
        HANDLE_ERROR("In function (init_shared_data_barier), malloc");
    }

    // Initialization
    sdb->N = N;
    PTHREAD_CHECK(
        pthread_barrier_init(&(sdb->barrier), NULL, threads_num),
        "In function (init_shared_data_barier), pthread_barrier_init"
    );

    return sdb;
}

void destroy_shared_data_barrier(_shared_data_barrier *sdb){

    PTHREAD_CHECK(
        pthread_barrier_destroy(&(sdb->barrier)),
        "In function (destroy_shared_data_barier), pthread_barrier_destroy"
    );
    free(sdb);
}


// -------------- CONDITION ----------------------

void *Condition_thread(void *args){

    _shared_data_condition *sdc = (_shared_data_condition *)args;
    int N = sdc->N;
    
    for(int i = 0; i < N; i++){
        thread_condition_barrier_wait(sdc);
    }

    return NULL;
}

void thread_condition_barrier_wait(_shared_data_condition *sdc){

    // Mutex Lock
    PTHREAD_CHECK(
        pthread_mutex_lock(&(sdc->mutex)),
        "In function (thread_condition_barrier_wait), pthread_mutex_lock"
    );

    sdc->threads_waiting++;

    // Last Thread. Wake up all the athers
    if(sdc->threads_waiting == sdc->threshold){
        sdc->threads_waiting = 0;
        PTHREAD_CHECK(
            pthread_cond_broadcast(&(sdc->cond)),
            "In function (thread_condition_barrier_wait), pthread_cond_broadcast"    
        );
    }
    // W8 until all threads come. On w8, Unlcok->Sleep, On wakeup WakeUp->Lock
    else{
        PTHREAD_CHECK(
            pthread_cond_wait(&(sdc->cond), &(sdc->mutex)),
            "In function (thread_condition_barrier_wait), pthread_cond_wait"
        );
    }
    
    // Mutex Unlock
    PTHREAD_CHECK(
        pthread_mutex_unlock(&(sdc->mutex)),
        "In function (thread_condition_barrier_wait), pthread_mutex_unlock"
    );    
}

_shared_data_condition *create_shared_data_condition(const int N, const int threads_num){

    // Take memory
    _shared_data_condition *sdc = (_shared_data_condition *) malloc(sizeof(_shared_data_condition));
    if(sdc == NULL){
        HANDLE_ERROR("In function (create_shared_data_condition), malloc");
    }

    // Initialization
    sdc->N = N;
    sdc->threads_waiting = 0;
    sdc->threshold = threads_num;
    PTHREAD_CHECK(
        pthread_mutex_init(&(sdc->mutex), NULL),
        "In function (create_shared_data_condition), pthread_mutex_init"
    );
    PTHREAD_CHECK(
        pthread_cond_init(&(sdc->cond), NULL),
        "In function (create_shared_data_condition), pthread_cond_init"
    );

    return sdc;
}

void destroy_shared_data_condition(_shared_data_condition *sdc){

    PTHREAD_CHECK(
        pthread_mutex_destroy(&(sdc->mutex)),
        "In function (destroy_shared_data_condition), pthread_mutex_destroy"
    );
    PTHREAD_CHECK(
        pthread_cond_destroy(&(sdc->cond)),
        "In function (destroy_shared_data_condition), pthread_cond_destroy"
    );
    free(sdc);
}


// ------------------ MY BARRIER ------------------

void *MyBarrier_thread(void *args){

    _shared_data_mybarrier *sdmb = (_shared_data_mybarrier *)args;
    int N = sdmb->N;

    for(int i = 0; i < N; i++){
        thread_mybarrier_wait(sdmb);
    }

    return NULL;
}

void thread_mybarrier_wait(_shared_data_mybarrier *sdmb){
    int *lower, *higher;

    // Mutex Lock
    PTHREAD_CHECK(
        pthread_mutex_lock(&(sdmb->mutex)),
        "In function (thread_mybarrier_wait), pthread_mutex_lock"
    );

    lower  = MIN(sdmb->threads_waiting_1, sdmb->threads_waiting_2);
    higher = MAX(sdmb->threads_waiting_1, sdmb->threads_waiting_2);

    (*lower)++;

    // Last Thread
    if(*lower == sdmb->threshold){
        (*higher) = 0;

        // Mutex Unlock
        PTHREAD_CHECK(
            pthread_mutex_unlock(&(sdmb->mutex)),
            "In function (thread_mybarrier_wait), pthread_mutex_unlock"
        );
    }
    // All exept last thread
    else{

        // Mutex Unlock
        PTHREAD_CHECK(
            pthread_mutex_unlock(&(sdmb->mutex)),
            "In function (thread_mybarrier_wait), pthread_mutex_unlock"
        );

        while(*lower < sdmb->threshold);            
    }

}

_shared_data_mybarrier *create_shared_data_mybarrier(const int N, const int threads_num){

    // Take memory
    _shared_data_mybarrier *sdmb = (_shared_data_mybarrier *) malloc(sizeof(_shared_data_mybarrier));
    if(sdmb == NULL){
        HANDLE_ERROR("In function (create_shared_data_mybarrier), malloc");
    }

    // Initialization
    sdmb->N = N;
    sdmb->threshold = threads_num;
    sdmb->threads_waiting_1 = 0;
    sdmb->threads_waiting_2 = threads_num;
    PTHREAD_CHECK(
        pthread_mutex_init(&(sdmb->mutex), NULL),
        "In function (create_shared_data_mybarrier), pthread_mutex_init"
    );

    return sdmb;
}

void destroy_shared_data_mybarrier(_shared_data_mybarrier *sdmb){

    PTHREAD_CHECK(
        pthread_mutex_destroy(&(sdmb->mutex)),
        "In function (destroy_shared_data_mybarrier), pthread_mutex_destroy"
    );
    free(sdmb);
}


// -------------- SENSE-REVERSAL -----------------

void *SenseReversal_thread(void *args){

    _shared_data_sense_reversal *sdsr = (_shared_data_sense_reversal *)args;
    int N = sdsr->N;

    for(int i = 0; i < N; i++){
        thread_sense_reversal_wait(sdsr);
    }

    return NULL;
}

void thread_sense_reversal_wait(_shared_data_sense_reversal *sdsr){

    bool local_sense = sdsr->flag;

    // Mutex Lock
    PTHREAD_CHECK(
        pthread_mutex_lock(&(sdsr->mutex)),
        "In function (thread_sense_reversal_wait), pthread_mutex_lock"
    );

    sdsr->threads_waiting++;
    local_sense = !local_sense;

    // Last Thread
    if(sdsr->threads_waiting == sdsr->threshold){
        sdsr->threads_waiting = 0;
        sdsr->flag = local_sense;

        // Mutex Unlock
        PTHREAD_CHECK(
            pthread_mutex_unlock(&(sdsr->mutex)),
            "In function (thread_sense_reversal_wait), pthread_mutex_unlock"
        );        
    }
    // All exept last thread
    else{

        // Mutex Unlock
        PTHREAD_CHECK(
            pthread_mutex_unlock(&(sdsr->mutex)),
            "In function (thread_sense_reversal_wait), pthread_mutex_unlock"
        );
        
        while (sdsr->flag != local_sense);
    }
}

_shared_data_sense_reversal *create_shared_data_sense_reversal(const int N, const int threads_num){

    // Take memory
    _shared_data_sense_reversal *sdsr = (_shared_data_sense_reversal *) malloc(sizeof(_shared_data_sense_reversal));
    if(sdsr == NULL){
        HANDLE_ERROR("In function (create_shared_data_sense_reversal), malloc");
    }

    // Initialization
    sdsr->N = N;
    sdsr->threshold = threads_num;
    sdsr->flag = false;
    sdsr->threads_waiting = 0;
    PTHREAD_CHECK(
        pthread_mutex_init(&(sdsr->mutex), NULL),
        "In function (create_shared_data_sense_reversal), pthread_mutex_init"
    );

    return sdsr;
}

void destroy_shared_data_sense_reversal(_shared_data_sense_reversal *sdsr){

    PTHREAD_CHECK(
        pthread_mutex_destroy(&(sdsr->mutex)),
        "In function (destroy_shared_data_sense_reversal), pthread_mutex_destroy"
    );
    free(sdsr);
}
