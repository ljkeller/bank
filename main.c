#include <errno.h>
#include <limits.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <stdint.h>
#include <pthread.h>
#include "Bank.h"
#include "server_helper.h"
#include "queue.h"


#define PARAM_SIZE 32
#define BUFFER_SIZE 1024

void* worker();

//Global variables shared by threads
pthread_mutex_t *ACCT_muts, queue_mut;

pthread_cond_t end_main;

//for testing reason
FILE *fptr;

int main(int argc, char* argv[]) {
    pthread_t *tid_workers; //Record thread IDs for all workers

    struct job first_job, *j;
    struct queue job_queue;
    char *p, *params[PARAM_SIZE], *command;
    char filename[PARAM_SIZE], buffer[BUFFER_SIZE];
    int n_workers, n_accounts, running, ret, i, request_id, account_id, amount, dst_account;
    int src_account;
    errno = 0;
    long ID = 0;
    struct timeval time_start, time_end;

    //Program initialization and input checking
    if(argc != 4) {
        errno = EINVAL;
        perror("It looks like there were insufficient inputs");
    }

    //THREAD ARRAY ALLOCATION
    long arg = strtol(argv[1], &p, 10);
    if( validate_input(p, arg, errno) != 0) {
        errno = EINVAL;
        perror("It looks like there was in invalid input");
    }
    n_workers = arg; //Number of worker threads
    tid_workers = (pthread_t *) malloc(sizeof(pthread_t) * n_workers);
    
    //MUTEX INITIALIZATION
    arg = strtol(argv[2], &p, 10);
    if(validate_input(p, arg, errno) != 0) {
        errno = EINVAL;
        perror("It looks like there was in invalid input.\n");
    } 
    n_accounts = arg;
    ACCT_muts = (pthread_mutex_t *) malloc(sizeof(pthread_mutex_t) * n_accounts);
    for(i = 0; i < n_accounts; i++) {
        pthread_mutex_init(&ACCT_muts[i], NULL); //Initialize all account mutexes
    }
    pthread_mutex_init(&queue_mut, NULL); //Initializae the queue
    pthread_cond_init(&end_main, NULL);

    //File handling for output file
    strcpy(filename, argv[3]);
    strcat(filename, ".txt");
    fptr = fopen(filename, "w+");
    if(fptr == NULL) {
        errno = EINVAL;
        perror("Bad file handling\n");
        exit(1);
    }

    printf("Launched %d worker threads with %d accounts. "
            "The output will be %s.\n",
            n_workers, n_accounts, filename);


    //Account initialization, queue initialization
    request_id = 0;
    ret = initialize_accounts(n_accounts);
    if(ret == 0) {
        errno = EHOSTUNREACH;
        perror("Unsuccessful account initializations.\n");
    }
    init_queue(&job_queue);
    running = 1;
   
    //THREAD CREATION
    for(i = 0; i < n_workers; i++) {
        pthread_create(&tid_workers[i], NULL, worker, NULL);
    }

    // Ready to run bank server and take requests
    while(running) {
        printf("Accepting user input: ");
        p = fgets(buffer, BUFFER_SIZE, stdin);
        gettimeofday(&time_start, NULL);

        if(p == NULL){
            errno = EINVAL;
            request_id--;
            perror("Looks like there was problems taking input.\n");
        }
        request_id++;
        ret = read_command(buffer, params);
        j = new_job(request_id);
        command = params[0];
        ret = params_to_job(params, ret, j, n_accounts);
        //TODO: Dynamically allocate new processes
        if(ret != 0) {
            fr_job(j); //DEALLOCATE JOBS AND TRANSACTIONS
            errno = ret;
            printf("Resetting request id\n");
            request_id--;
        }
        ret = process_job(j, fptr);
        //TODO: Add this shit to queue
        if(ret == -1) { //End was input
            running = 0;
        }
        errno = 0;
    }

    //Wait for all threads to join
    for(i = 0; i < n_workers; i++) {
        pthread_join(tid_workers[i], NULL);
    }

    fclose(fptr);
    return 0; 
}

void* worker() { 
    //Wait for more input, unless end_m== 1
    pthread_mutex_lock(&queue_mut);
    flockfile(fptr);
    //DO WRITING HERE
    funlockfile(fptr);
    //fflush(stdout);
    pthread_mutex_unlock(&queue_mut);
    //Deallocate job, transactions by now
}
