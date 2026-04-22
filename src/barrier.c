#include "barrier.h"

#include <stdio.h>
#include <time.h>


int main(int argc, char **argv){
    
    security_user_input(argc, argv); // Make sure user give right input

    /* User input (argc, argv) */
    int N = atoi(argv[1]), threads_num = atoi(argv[2]);
    _policy policy = atoi(argv[3]);

    void *sd = NULL;
    void *(*ptr_thread_fun)(void *) = NULL;
    struct timespec tic, toc;

    pthread_t *thread_id = (pthread_t *) malloc(sizeof(pthread_t) * threads_num);
    if(thread_id == NULL){
        HANDLE_ERROR("In function (main). malloc");
    }


    /* Create shared data */
    _shared_data_barrier *sdb = create_shared_data_barrier(N, threads_num);
    _shared_data_condition *sdc = create_shared_data_condition(N, threads_num);
    _shared_data_mybarrier *sdmb = create_shared_data_mybarrier(N, threads_num);
    _shared_data_sense_reversal *sdsr = create_shared_data_sense_reversal(N, threads_num);

    /* Depens on policy choose function and shared data */
    switch(policy){

        case barrier:
            sd = sdb;
            ptr_thread_fun = &Barrier_thread;
            break;

        case condition:
            sd = sdc;
            ptr_thread_fun = &Condition_thread;
            break;
        
        case mybarrier:
            sd = sdmb;
            ptr_thread_fun = &MyBarrier_thread;
            break;
        
        case sense_reversal_barrier:
            sd = sdsr;
            ptr_thread_fun = &SenseReversal_thread;
            break;
    }

    clock_gettime(CLOCK_MONOTONIC, &tic);

    /* Create threads */
    for(int i = 0; i < threads_num; i++){
        PTHREAD_CHECK(
            pthread_create(&thread_id[i], NULL, ptr_thread_fun, (void *) sd),
            "In function (main), pthread_create"
        );
    }

    /* Join threads */
    for(int i = 0; i < threads_num; i++){
        PTHREAD_CHECK(
            pthread_join(thread_id[i], NULL),
            "In function (main), pthread_join"
        );
    }

    clock_gettime(CLOCK_MONOTONIC, &toc);

    free(thread_id);

    /* Destroy shared data */
    destroy_shared_data_barrier(sdb);
    destroy_shared_data_condition(sdc);
    destroy_shared_data_mybarrier(sdmb);
    destroy_shared_data_sense_reversal(sdsr);

    printf("%.6f\n", (toc.tv_sec - tic.tv_sec) + (toc.tv_nsec - tic.tv_nsec) / 1e9);

    return 0;
}
