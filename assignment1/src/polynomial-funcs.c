#include "polynomial.h"

#include <time.h>
#include <pthread.h>
#include <unistd.h>


// ----------- GENERAL PURPOSE ------------

int valid_coefficient(bool is_rand, unsigned int *seedp){

    int number; bool is_positive;

    number = (is_rand) ? rand() : rand_r(seedp);                // parallel->rand_r or serial->rand
    is_positive = ((number & LCBIT_MASK) == 0) ? true : false;  // least significant bit = 0 '+' else '-'
    number = number % 9 + 1;                    // [1,9]
    number = (is_positive) ? number : -number;  // [-9,-1] or [1,9]

    return number;
}

void destroy_polynomials(_polynomials *pol){
    free(pol->a);
    free(pol->b);
    free(pol);
}

double get_time(struct timespec *tic, struct timespec *toc){
    return (toc->tv_sec - tic->tv_sec) + (toc->tv_nsec - tic->tv_nsec) / 1e9;
}

bool is_equal(const int *prod_parallel, const int *prod_serial, int power){

    for(int i = 0; i < PRODUCT_POLYNOMIAL_SIZE(power); i++){
        if(prod_parallel[i] != prod_serial[i]){
            return false;
        }
    }

    return true;
}

void security_user_input(int argc, char **argv){

    // Bad number of arguments
    if(argc != 3){
        HANDLE_ERROR("Usage: ./<executable> <loops number> <threads number>");
    }

    int loops = atoi(argv[1]), threads_num = atoi(argv[2]);

    if(loops <= 0 || threads_num <= 0){
        HANDLE_ERROR("Negative values are rejected");
    }
}


// ---------------- SERIAL ----------------

_polynomials *serial_creat_polynomials(const int power){

    srand(time(NULL));

    // Take memory for my struct
    _polynomials *pol = (_polynomials *) malloc(sizeof(_polynomials));
    if(pol == NULL){
        HANDLE_ERROR("In function (serial_creat_polynomials), malloc _polynomials");
    }

    char *a = (char *) malloc(POLYNOMIAL_SIZE(power) * sizeof(char));
    if(a == NULL){
        HANDLE_ERROR("In function (serial_creat_polynomials), malloc a");
    }

    char *b = (char *) malloc(POLYNOMIAL_SIZE(power) * sizeof(char));
    if(b == NULL){
        HANDLE_ERROR("In function (serial_creat_polynomials), malloc b");
    }

    // Initialization
    for(int i = 0; i < POLYNOMIAL_SIZE(power); i++){
        a[i] = valid_coefficient(true, NULL);
        b[i] = valid_coefficient(true, NULL);
    }

    pol->a = a;
    pol->b = b;
    pol->power = power;

    return pol;
}

void serial_polynomial_product(const _polynomials *pol, int *prod_array){

    // i and j represent the power of the x in polynomials

    int magic_size = 1000;
    int komati = POLYNOMIAL_SIZE(pol->power) / magic_size;
    int last;

    for(int i = 0; i < POLYNOMIAL_SIZE(pol->power); i++){

        // Loop unroling
        for(int k = 0; k < magic_size; k++){

            last = (k+1 == magic_size) ? POLYNOMIAL_SIZE(pol->power) : k*komati+komati;

            for(int j = k*komati; j < last; j++){
                prod_array[i + j] += pol->a[i] * pol->b[j];
            }
        }
    }
}


// --------------- PARALLEL ---------------

void *Initialize_polynomials_thread(void *args){

    _init_pol_chunk *ipc = (_init_pol_chunk *)args;

    for(int i = ipc->range.start; i < ipc->range.end; i++){
        ipc->pol[i] = valid_coefficient(false, &ipc->seedp);
    }

    free(args);

    return NULL;
}

/* Helpfull function. Create deferent threads values and spown thread  */
void spown_threads(char *array, pthread_t *id, unsigned int *seed, int power, int i, int slice){

    _init_pol_chunk *ipc;

    ipc = (_init_pol_chunk *) malloc(sizeof(_init_pol_chunk));
    if(ipc == NULL){
        HANDLE_ERROR("In function (spown_threads), malloc");
    }

    ipc->seedp = *seed;
    ipc->pol = array;
    ipc->range.start = i * slice;
    ipc->range.end = (i+1 == LINUX_CORES/2) ? POLYNOMIAL_SIZE(power) : ipc->range.start + slice;

    PTHREAD_CHECK(
        pthread_create(id, NULL, &Initialize_polynomials_thread, (void *)ipc),
        "In funtion (spown_threads), pthread_create"
    );

    (*seed)++;
}

_polynomials *parallel_creat_polynomials(const int power){

    unsigned int seed = time(NULL);

    // Take memory for my struct
    _polynomials *pol = (_polynomials *) malloc(sizeof(_polynomials));
    if(pol == NULL){
        HANDLE_ERROR("In function (serial_creat_polynomials), malloc _polynomials");
    }

    char *a = (char *) malloc(POLYNOMIAL_SIZE(power) * sizeof(char));
    if(a == NULL){
        HANDLE_ERROR("In function (serial_creat_polynomials), malloc a");
    }

    char *b = (char *) malloc(POLYNOMIAL_SIZE(power) * sizeof(char));
    if(b == NULL){
        HANDLE_ERROR("In function (serial_creat_polynomials), malloc b");
    }

    pthread_t pol_a_id[LINUX_CORES/2], pol_b_id[LINUX_CORES/2];
    int slice = POLYNOMIAL_SIZE(power) / (LINUX_CORES/2); // how much range will work a thread

    // Create threads for first polynomial
    for(int i = 0; i < LINUX_CORES/2; i++){
        spown_threads(a, &pol_a_id[i], &seed, power, i, slice);
    }

    // Create threads for secont polynomial
    for(int i = 0; i < LINUX_CORES/2; i++){
        spown_threads(b, &pol_b_id[i], &seed, power, i, slice);
    }


    // Join all the threads for first polynomial
    for(int i = 0; i < LINUX_CORES/2; i++){
        PTHREAD_CHECK(
            pthread_join(pol_a_id[i], NULL),
            "In function (parallel_creat_polynomials), pthread_join"
        );
    }

    // Join all the threads for secont polynomial
    for(int i = 0; i < LINUX_CORES/2; i++){
        PTHREAD_CHECK(
            pthread_join(pol_b_id[i], NULL),
            "In function (parallel_creat_polynomials), pthread_join"
        );
    }

    pol->a = a;
    pol->b = b;
    pol->power = power;

    return pol;
}

void *Product_thread(void *args){

    _pol_prod_chunk *ppc = (_pol_prod_chunk *)args;

    // Take memory "orivate" only for the current thread
    int priv_size = ppc->range.end + POLYNOMIAL_SIZE(ppc->hd->pol->power) + 1 - ppc->range.start;
    int *priv_prod = (int *) calloc(priv_size, sizeof(int));
    if(priv_prod == NULL){
        HANDLE_ERROR("In function (Product_thread), calloc");
    }

    int magic_size = 1000;
    int komati = POLYNOMIAL_SIZE(ppc->hd->pol->power) / magic_size;
    int last;

    for(int i = ppc->range.start; i < ppc->range.end; i++){

        // Loop unroling
        for(int k = 0; k < magic_size; k++){

            last = (k+1 == magic_size) ? POLYNOMIAL_SIZE(ppc->hd->pol->power) : k*komati+komati;

            for(int j = k*komati; j < last; j++){
                priv_prod[i - ppc->range.start + j] += ppc->hd->pol->a[i] * ppc->hd->pol->b[j];
            }
        }
    }

    // Combine the result serially
    
    // Mutex Lock
    PTHREAD_CHECK(
        pthread_mutex_lock(&(ppc->hd->mutex)),
        "In function (Product_thread), pthread_mutex_lock"
    );

    for(int i = 0; i < priv_size; i++){
        ppc->hd->prod[i + ppc->range.start] += priv_prod[i];
    }

    // Mutex Unlock
    PTHREAD_CHECK(
        pthread_mutex_unlock(&(ppc->hd->mutex)),
        "In function (Product_thread), pthread_mutex_unlock"
    );

    free(ppc);
    free(priv_prod);
    return NULL;
}

_hot_data *create_hot_data(_polynomials *pol, int *prod){

    // Take space
    _hot_data *hd = (_hot_data *) malloc(sizeof(_hot_data));
    if(hd == NULL){
        HANDLE_ERROR("In function (create_hot_data), malloc");
    }

    // Initialization
    hd->pol = pol;
    hd->prod = prod;
    PTHREAD_CHECK(
        pthread_mutex_init(&(hd->mutex), NULL),
        "In function (create_hot_data), pthread_mutex_init"
    );

    return hd;
}

void destroy_hot_data(_hot_data *hd){
    PTHREAD_CHECK(
        pthread_mutex_destroy(&(hd->mutex)),
        "In function (destroy_hot_data), pthread_mutex_destroy"
    );
    free(hd);
}
