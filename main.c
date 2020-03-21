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

void mut_down_accts(struct job *j, int *arr);

void mut_up_accts(struct job *j, int *arr);

//Global variables shared by threads
pthread_mutex_t *ACCT_muts, queue_mut;

pthread_cond_t consumer_cv;

struct queue job_queue;

int end_signaled;

//for testing reason
FILE *fptr;

int main(int argc, char* argv[]) {
    pthread_t *tid_workers; //Record thread IDs for all workers

    struct job first_job, *j;
    char *p, *params[PARAM_SIZE], *command;
    char filename[PARAM_SIZE], buffer[BUFFER_SIZE];
    int n_workers, n_accounts, ret, i, request_id, account_id, amount, dst_account;
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
    pthread_cond_init(&consumer_cv, NULL); //Init consumer cv

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
    end_signaled = 0;
   
    //THREAD CREATION
    for(i = 0; i < n_workers; i++) {
        pthread_create(&tid_workers[i], NULL, worker, NULL);
    }

    // Ready to run bank server and take requests
    while(end_signaled == 0) {
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
        if(end_signaled) {
            printf("Sorry, we closed during your user input\n");
            break;
        }
        j = new_job(request_id);
        command = params[0];
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
        errno = 0;
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
    int ret, job_IDs[10];
    pthread_mutex_lock(&queue_mut);
    while(end_signaled == 0 || job_queue.num_jobs > 0) {
        while(job_queue.num_jobs == 0 && end_signaled == 0) {
            fflush(stdout);
            pthread_cond_wait(&consumer_cv, &queue_mut); //Yield mut to main for more jobs
        }
        if(job_queue.num_jobs > 0) {
            j = pop(&job_queue);
            pthread_mutex_unlock(&queue_mut);
            //TODO: MAKE THIS IN SORTED ORDER
            mut_down_accts(j, job_IDs); //Check all counts not in use
            flockfile(fptr); //ensures order to file output
            ret = process_job(j, fptr);
            funlockfile(fptr);
            mut_up_accts(j, job_IDs); //Unlock all acounts worked with in process
            if(ret == -1) {//call to END
                end_signaled = 1;
            }
            fr_job(j); //DEALLOCATE JOBS AND TRANSACTIONS
        }
    }
    pthread_mutex_unlock(&queue_mut);
    return NULL;
}

void mut_down_accts(struct job *j, int *arr) {
    if(j->type == TRANS) {
        struct trans *t = j->transactions;
        int i, k, acc_id, n = j->num_trans;
        for(i = 0; i < n; i++) {
            acc_id = t[i].acc_id;
            k = i - 1; //Insertion sort
            while(k >= 0 && arr[k] > acc_id) {
                arr[k+1] = arr[k];
                k = k - 1;
            }
            arr[k+1] = acc_id;
        }
        //Remember, if dont lock in correct order there will be deadlocks
        for(i = 0; i < n; i++) {
            pthread_mutex_lock(&ACCT_muts[arr[i]]);
        }
    }

}

void mut_up_accts(struct job *j, int *arr) {
    if(j->type == TRANS) {
        struct trans *t = j->transactions;
        int i, k, acc_id, n = j->num_trans;
        for(i = 0; i < n; i++) {
            acc_id = t[i].acc_id;
            k = i - 1; //Insertion sort
            while(k >= 0 && arr[k] > acc_id) {
                arr[k+1] = arr[k];
                k = k - 1;
            }
            arr[k+1] = acc_id;
        }
        //Remember, if dont lock in correct order there will be deadlocks
        for(i = 0; i < n; i++) {
            pthread_mutex_unlock(&ACCT_muts[arr[i]]);
        }
    }
}
