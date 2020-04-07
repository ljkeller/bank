#include <errno.h>
#include <limits.h>
#include <sys/select.h>
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

void mut_down_accts(struct job *j);

void mut_up_accts(struct job *j);

//Global variables shared by threads
pthread_mutex_t queue_mut, acct_mut;

pthread_cond_t consumer_cv;

struct queue job_queue;

int end_signaled;

//for testing reason
FILE *fptr;

int main(int argc, char* argv[]) {
    pthread_t *tid_workers; //Record thread IDs for all workers
    fd_set rfds;
    struct job first_job, *j;
    char *p, *params[PARAM_SIZE], *command;
    char filename[PARAM_SIZE], buffer[BUFFER_SIZE];
    int n_workers, n_accounts, ret, i, request_id, account_id, amount, dst_account;
    int src_account;
    errno = 0;
    long ID = 0;
    struct timeval tv;

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
    pthread_mutex_init(&queue_mut, NULL); //Initializae the queue
    pthread_mutex_init(&acct_mut, NULL); //Initializae the queue
    pthread_cond_init(&consumer_cv, NULL); //Init consumer cv

    //File handling for output file
    strcpy(filename, argv[3]);
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
    end_signaled = 0;
   
    //THREAD CREATION
    for(i = 0; i < n_workers; i++) {
        pthread_create(&tid_workers[i], NULL, worker, NULL);
    }

    tv.tv_sec = 5;
    tv.tv_usec = 0;
    printf("Accepting user input: ");
    // Ready to run bank server and take requests
    while(end_signaled == 0) {
        errno = 0;
        FD_ZERO(&rfds);
        FD_SET(0, &rfds);

        ret = select(1, &rfds, NULL, NULL, &tv);
        if(ret) {
            p = fgets(buffer, BUFFER_SIZE, stdin);
        } else {
            continue;
        }
        if(p == NULL){
            errno = EINVAL;
            request_id--;
            perror("Looks like there was problems taking input.\n");
            continue;
        }
        request_id++;
        ret = read_command(buffer, params);
        if(end_signaled) {
            printf("Sorry, we closed during your user input\n");
            break;
        }
        j = new_job(request_id);
        ret = params_to_job(params, ret, j, n_accounts);
        if(ret != 0) {
            fr_job(j); //DEALLOCATE JOBS AND TRANSACTIONS
            errno = ret;
            printf("Resetting request id\n");
            request_id--;
        } else { //If successful parameterization, push
            pthread_mutex_lock(&queue_mut);
            push(&job_queue, j);
            pthread_mutex_unlock(&queue_mut);
            pthread_cond_signal(&consumer_cv); //for the workers blocked on cv
        }
    }

    //Wait for all threads to join
    pthread_cond_broadcast(&consumer_cv); //Frees all waiting for more input
    for(i = 0; i < n_workers; i++) {
        pthread_join(tid_workers[i], NULL);
    }

    fclose(fptr);
    return 0; 
}

void* worker() { 
    struct job *j;
    int ret;
    pthread_mutex_lock(&queue_mut);
    while(end_signaled == 0 || job_queue.num_jobs > 0) {
        while(job_queue.num_jobs == 0 && end_signaled == 0) {
            pthread_cond_wait(&consumer_cv, &queue_mut);
        } //Once out, has access to mut
        j = pop(&job_queue);
        if(j != NULL) {
            pthread_mutex_unlock(&queue_mut);
            pthread_mutex_lock(&acct_mut);
            ret = process_job(j, fptr);
            pthread_mutex_unlock(&acct_mut);
            if(ret == -1) {
                end_signaled = 1;
                break;
            }
        }
        if(end_signaled == 0) {
            pthread_mutex_lock(&queue_mut);
        }

        while(job_queue.num_jobs == 0 && end_signaled == 0) {
            pthread_cond_wait(&consumer_cv, &queue_mut);
        }
    }
    pthread_mutex_unlock(&queue_mut);
    return NULL;
}
