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


#define ARG_MAX 25
#define PARAM_SIZE 64
#define BUFFER_SIZE 2048

void* worker();

void append(struct queue *queue);

void pop(struct queue *queue);

//Global variables shared by threads
pthread_mutex_t *ACCT_muts, queue_mut;

pthread_cond_t end_main;

//for testing reason
FILE *fptr;

int main(int argc, char* argv[]) {
    pthread_t *tid_workers; //Record thread IDs for all workers

    struct job first_job;
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


    //Account initialization
    request_id = 0;
    ret = initialize_accounts(n_accounts);
    if(ret == 0) {
        errno = EHOSTUNREACH;
        perror("Unsuccessful account initializations.\n");
    }

    //THREAD CREATION
    for(i = 0; i < n_workers; i++) {
        pthread_create(&tid_workers[i], NULL, worker, NULL);
    }

    // Ready to run bank server and take requests
    while(running) {
        printf("Accepting user input: ");
        p = fgets(buffer, ARG_MAX, stdin);
        gettimeofday(&time_start, NULL);

        if(p == NULL){
            errno = EINVAL;
            request_id--;
            perror("Looks like there was problems taking input.\n");
        }
        request_id++;
        ret = read_command(buffer, params);
        command = params[0];
        ret = params_to_job(params, ret, &first_job, request_id);
        //TODO: Dynamically allocate new processes
        if(ret != 0) {
            //TODO: WIll need to deallocate or something
            errno = ret;
            perror("Invalid input");
            request_id--;
            break;
        }
        print_job(&first_job);
        break;
        
        /*
        if(strcmp(command, "END") == 0) {
            break;
        } else if(strcmp(command, "TRANS") == 0) {
            imm_response(request_id);
            if(ret < 4 || (ret - 2) % 2 != 0) {
                errno = EINVAL;
                perror("Looks like insufficient or bad parameters");
                request_id--;
                break;
            }
            amount = strtol(params[2], &p, 10);
            //TODO: input validation on all int to char conversions
            //TODO: Write a modified version of transfer that takes a series adcounts/amounts.
            if(amount >= 0) { //consider money movement with positive vs negative amount
                src_account = strtol(params[1], &p, 10);
                dst_account = strtol(params[3], &p, 10);
            } else {
                src_account = strtol(params[3], &p, 10);
                dst_account = strtol(params[1], &p, 10);
            }
            ret = transfer_balance(src_account, dst_account, amount);

            //Bad user input, bad transfer
            if(ret != 0) {
                errno = ret;
                printf("There was an improper TRANSFER request\n");
                gettimeofday(&time_end, NULL);
                fprintf(fptr, "%d ISF %d %ld.%06.ld %ld.%06.ld", 
                    request_id, src_account, 
                    time_start.tv_sec, time_start.tv_usec,
                    time_end.tv_sec, time_end.tv_usec);
                request_id--;
                
                break;
            } 

            gettimeofday(&time_end, NULL);
            fprintf(fptr, "%d OK TIME %ld.%06.ld %ld.%06.ld", 
                request_id,
                time_start.tv_sec, time_start.tv_usec,
                time_end.tv_sec, time_end.tv_usec);


        } else if(strcmp(command, "CHECK") == 0) {
            //Bad user input
            if(ret != 2) {
                errno = EINVAL;
                perror("This call should take an CHECK <account_id>. Too many or too few args");
                request_id--;
                break;
            } 

            imm_response(request_id); 
            account_id = strtol(params[1], &p, 10);
            printf("The account id was %d\n", account_id);
            ret = read_account(account_id);
            gettimeofday(&time_end, NULL);
            fprintf(fptr, "%d BAL %d TIME %ld.%06.ld %ld.%06.ld", 
                    request_id, ret, 
                    time_start.tv_sec, time_start.tv_usec,
                    time_end.tv_sec, time_end.tv_usec);
        } else {
            errno = EINVAL;
            request_id--;
            perror("Failure to provide valid input");
        }
        */
        break;
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
    fprintf(fptr, "Hello\n");
    funlockfile(fptr);
    printf("Hello!\n");
    fflush(stdout);
    pthread_mutex_unlock(&queue_mut);
}

void append(struct queue *queue) {
}

void pop(struct queue *queue) {
}
