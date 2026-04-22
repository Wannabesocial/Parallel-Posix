#ifndef MY_FUN_H
#define MY_FUN_H

#define LCBIT_MASK 0x1  // least significant bit
#define LINUX_CORES 8   // Total cores of linux02 2 (4*2 hardware threads)

#define _GNU_SOURCE

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <stdbool.h>

//  Usefull shits for both SERIAL and PARALLEL implementations

/* Macro to compute the size of the polynomials tha needed base on the power */
#define POLYNOMIAL_SIZE(power) ((power) + 1)

/* Macro to compute the product polynamial size */
#define PRODUCT_POLYNOMIAL_SIZE(power) (2 * (power) + 1)

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


/**
 * The representation of the polynomial is in ascending order base on x power 
 * +-----+-----+-----+-----+
 * |a1x^0|a2x^1|a3x^2|a4x^3| 3 degree polynomial representation
 * +-----+-----+-----+-----+
 *    0     1     2     3    <-- potition in the array 
**/
typedef struct _polynomials{
    char *a;    // first polynomial
    char *b;    // secont polynomial
    int power;  // the power of polynomials
}_polynomials;


/* Return a number in range [-9,9] - {0}. Works with rand,rand_r.
If rand_r pass NULL in pointer */
int valid_coefficient(bool is_rand, unsigned int *seedp);

/* Free polynomials. Destroy what initialized */
void destroy_polynomials(_polynomials *pol);

/* Get the wall cpu time */
double get_time(struct timespec *tic, struct timespec *toc);

/* Check if the two implemetation is equal */
bool is_equal(const int *prod_parallel, const int *prod_serial, int power);

/**
 * Security. Make sure the user give input in right order and is valid.
 * Execute with ./<executable> <loops number> <threads number>
**/
/* Check that given right number of arguments */
void security_user_input(int argc, char **argv);

// ---------------- SERIAL ----------------

/* Create-Malloc the polynomials. Do not forget to free */
_polynomials *serial_creat_polynomials(const int power);

/* Procudt of 2 polynomials */
void serial_polynomial_product(const _polynomials *pol, int *prod_array);


// --------------- PARALLEL ---------------

/* Diterminate what part of an array a thread can work with.
We do a loop starting in start and end in < end (aka. end-1) */
typedef struct _range{
    int start;
    int end; 
}_range;

/* Initialize polynomials thread parameter */
typedef struct _init_pol_chunk{
    unsigned int seedp;
    char *pol;
    _range range;
}_init_pol_chunk;

/* Hot shared data for all Polynomials product threads. (They sare the same values).
No malloc in create needed for inside prameters */
typedef struct _hot_data{
    const _polynomials *pol;
    int *prod;
    pthread_mutex_t mutex; // At the end, all workers will serially combine their results.
}_hot_data;

/* Polynomials product thread parameter.
No malloc in creat needed for inside prameters */
typedef struct _pol_prod_chunk{
    _hot_data *hd;  // shared data among all threads
    _range range;   // every thread has diferent range of work  
}_pol_prod_chunk;

/* Thread function that initialize the array. Parts of it */
void *Initialize_polynomials_thread(void *args);

/* Create-Malloc the polynomials. Do not forget to free */
_polynomials *parallel_creat_polynomials(const int power);


/* Thread function for polynomial product computation */
void *Product_thread(void *args);

/* Create and initialize "hot data" */
_hot_data *create_hot_data(_polynomials *pol, int *prod);

/* Destroy the "hot data" */
void destroy_hot_data(_hot_data *hd);

#endif