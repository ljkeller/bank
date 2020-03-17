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

#define ARG_MAX 25
#define PARAM_SIZE 64
#define BUFFER_SIZE 2048

int read_command(char command[], char *params[]);

int validate_input(char* p, long arg, int err);

void imm_response(int request_id);

int proc_trans();

void write_file(FILE *fptr, char* params[], int argc);

int meets_funds(int acct_id, int amount);


int main(int argc, char* argv[]) {
    char *p, *params[PARAM_SIZE], *command;
    char filename[PARAM_SIZE], buffer[BUFFER_SIZE];
    int n_workers, n_accounts, running, ret, i, request_id, account_id, amount;
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
            account_id = strol(params[1], &p, 10);
            ret = meets_funds(account_id,amount);
            if(ret != 0) {
                printf("There was an improper TRANSFER request");
                break;
            } else {
                dest_accout = strtol(params[3], &p, 10);
                deduct(dest_accout);
                //TODO: write deduce/add balance code
                fprintf(fptr, "%d OK TIME %ld.%06.ld %ld.%06.ld", 
                    request_id, ret, 
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

//Parses User input into a command with parameters. Returns number of params.
int read_command(char command[], char *params[]){
     char *head;
     const char delim[6] = " \r\n";
     int i = 0, tracker = -1;

     head = strtok(command, delim);
     while(head != NULL){
          tracker = -1; // Tracks if & at eof
          params[i] = head;
          if(strcmp(head, "&") == 0){
               tracker = i;
          }
          head = strtok(NULL, delim); // continue through
          i++;
     }
     if(tracker > 0){
          params[i-1] = NULL;
          return i;
     } else {
          params[i] = NULL; // param must be null terminated char**
          return i;
     }
}

int validate_input(char* p, long arg, int err) {
    if(*p != '\0' || err != 0) {
        return 1;
    } else if(arg < INT_MIN || arg > INT_MAX) {
        return 2;
    } else { 
        return 0; 
    } 
}

void imm_response(int request_id){
    printf("ID %d\n", request_id);
}

int proc_trans(){
    return 0;
}

void write_file(FILE *fptr, char* params[], int argc){
    int i = 0;
    for(i = 0; i<argc; i++) {
        fprintf(fptr, " %s", params[i]);
    }
}

int meets_funds(int acct_id, int amount) {
    int balance = read_account(acct_id);
    if(amount > balance) {
        return EBADE; 
    } else {
        return 0;
    }
}
    
