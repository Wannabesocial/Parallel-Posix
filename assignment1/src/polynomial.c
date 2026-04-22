#include "polynomial.h"

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>


int main(int argc, char **argv){

    security_user_input(argc, argv);

    /* User input values (argc, argv) */
    int power = atoi(argv[1]), threads_num = atoi(argv[2]);

    int *prod_serial, *prod_parallel;
    struct timespec tic, toc;

    clock_gettime(CLOCK_MONOTONIC, &tic);
    _polynomials *pol = parallel_creat_polynomials(power);
    clock_gettime(CLOCK_MONOTONIC, &toc);

    printf("Create polynomials %.6f sec\n", get_time(&tic, &toc));

    // Create space for serial implementation
    prod_serial = (int *) calloc(PRODUCT_POLYNOMIAL_SIZE(power), sizeof(int));
    if(prod_serial == NULL){
        HANDLE_ERROR("In function (main), calloc(prod_serial)");
    }

    // Create space for parallel implementation
    prod_parallel = (int *) calloc(PRODUCT_POLYNOMIAL_SIZE(power), sizeof(int));
    if(prod_parallel == NULL){
        HANDLE_ERROR("In function (main), calloc(prod_parallel)");
    }
    
    clock_gettime(CLOCK_MONOTONIC, &tic);
    serial_polynomial_product(pol, prod_serial);
    clock_gettime(CLOCK_MONOTONIC, &toc);

    printf("Serial product %.6f sec\n", get_time(&tic, &toc));


    _hot_data *hd = create_hot_data(pol, prod_parallel);

    pthread_t *thread_id = (pthread_t *) malloc(sizeof(pthread_t) * threads_num);
    if(thread_id == NULL){
        HANDLE_ERROR("In function (main), malloc");
    }

    _pol_prod_chunk *ppc;
    int slice = POLYNOMIAL_SIZE(power) / threads_num; // how much range will work a thread


    clock_gettime(CLOCK_MONOTONIC, &tic);

    for(int i = 0; i < threads_num; i++){

        ppc = (_pol_prod_chunk *) malloc(sizeof(_pol_prod_chunk));
        if(ppc == NULL){
            HANDLE_ERROR("In function (main), malloc");
        }

        ppc->hd = hd;
        ppc->range.start = i * slice;
        ppc->range.end = (i+1 == threads_num) ? POLYNOMIAL_SIZE(power) : ppc->range.start + slice;
        
        PTHREAD_CHECK(
            pthread_create(&thread_id[i], NULL, &Product_thread, (void *) ppc),
            "In function (main), pthread_create"
        );
    }

    for(int i = 0; i < threads_num; i++){
        PTHREAD_CHECK(
            pthread_join(thread_id[i], NULL),
            "In function (main), pthread_join"
        );
    }

    clock_gettime(CLOCK_MONOTONIC, &toc);

    printf("Parallel product %.6f sec\n", get_time(&tic, &toc));

    (is_equal(prod_parallel, prod_serial, power)) ? printf("Equal\n") : printf("Not Equal\n");

    free(thread_id);
    destroy_hot_data(hd);
    free(prod_parallel);
    free(prod_serial);
    destroy_polynomials(pol);
    
    return 0;
}
