#include <errno.h>
#include <limits.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include "Bank.h"
#include "server_helper.h"

#define ARG_MAX 25
#define PARAM_SIZE 64
#define BUFFER_SIZE 2048

int main(int argc, char* argv[]) {
    char *p, *params[PARAM_SIZE], *command;
    char filename[PARAM_SIZE], buffer[BUFFER_SIZE];
    int n_workers, n_accounts, running, ret, i, request_id, account_id, amount, dst_account;
    int src_account;
    errno = 0;
    FILE *fptr;
    long ID = 0;
    struct timeval time_start, time_end;

    //Program initialization and input checking
    if(argc != 4) {
        errno = EINVAL;
        perror("It looks like there were insufficient inputs");
    }

    long arg = strtol(argv[1], &p, 10);
    if( validate_input(p, arg, errno) != 0) {
        errno = EINVAL;
        perror("It looks like there was in invalid input");
    }
    n_workers = arg; //Number of worker threads
    
    arg = strtol(argv[2], &p, 10);
    if( validate_input(p, arg, errno) != 0) {
        errno = EINVAL;
        perror("It looks like there was in invalid input.\n");
    } 
    n_accounts = arg;

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
    // Ready to run bank server and take requests
    while(running) {
        printf("Accepting user input: ");
        p = fgets(buffer, ARG_MAX, stdin);
        gettimeofday(&time_start, NULL);

        if(p == NULL){
            errno = EINVAL;
            perror("Looks like there was problems taking input.\n");
        }
        request_id++;
        ret = read_command(buffer, params);
        command = params[0];
        
        if(strcmp(command, "END") == 0) {
            break;
        } else if(strcmp(command, "TRANS") == 0) {
            imm_response(request_id);
            //proc_trans();
            if(ret < 4 || (ret - 2) % 2 != 0) {
                errno = EINVAL;
                perror("Looks like insufficient or bad parameters");
                break;
            }
            amount = strtol(params[2], &p, 10);
            //TODO: input validation on all int to char conversions
            //TODO: Write a modified version of transfer that takes a series adcounts/amounts.
            // This will need to work with negative balances...
            src_account = strtol(params[1], &p, 10);
            dst_account = strtol(params[3], &p, 10);
            ret = transfer_balance(src_account, dst_account, amount);
            if(ret != 0) {
                errno = ret;
                printf("There was an improper TRANSFER request\n");
                gettimeofday(&time_end, NULL);
                fprintf(fptr, "%d ISF %d %ld.%06.ld %ld.%06.ld", 
                    request_id, src_account, 
                    time_start.tv_sec, time_start.tv_usec,
                    time_end.tv_sec, time_end.tv_usec);
                
                break;
            } else {
                gettimeofday(&time_end, NULL);
                fprintf(fptr, "%d OK TIME %ld.%06.ld %ld.%06.ld", 
                    request_id,
                    time_start.tv_sec, time_start.tv_usec,
                    time_end.tv_sec, time_end.tv_usec);


            }
        } else if(strcmp(command, "CHECK") == 0) {
            if(ret == 1) {
                errno = EINVAL;
                perror("Missing <accountid>");
                request_id--;
                break;
            } 
            imm_response(request_id); 
            //TODO: Track request time.
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
        //printf("The command was \"%s\" and the argc was: %d\n", command, ret);
        /*fprintf(fptr, "%s", command);

        //Append user input to file to record
        //TODO: ensure only doing this when no threads are
        for(i = 1; i<ret; i++) {
            fprintf(fptr, " %s", params[i]);
        }
        fprintf(fptr, "\n");
        */

        break;
    }

    fclose(fptr);
    return 0; 
}
