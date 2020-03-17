#include <errno.h>
#include <limits.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include "Bank.h"

#define ARG_MAX 25
#define PARAM_SIZE 64
#define BUFFER_SIZE 2048

int read_command(char command[], char *params[]);

int validate_input(char* p, long arg, int err);

int main(int argc, char* argv[]) {
    char *p, *params[PARAM_SIZE], *command;
    char filename[PARAM_SIZE], buffer[BUFFER_SIZE];
    int n_workers, n_accounts, running, ret, i;
    errno = 0;
    FILE *fptr;
    long ID = 0;

    //Program initialization and input checking
    if(argc != 4) {
        perror("It looks like there were insufficient inputs");
    }

    long arg = strtol(argv[1], &p, 10);
    if( validate_input(p, arg, errno) != 0) {
        perror("It looks like there was in invalid input");
    }
    n_workers = arg; //Number of worker threads
    
    arg = strtol(argv[2], &p, 10);
    if( validate_input(p, arg, errno) != 0) {
        perror("It looks like there was in invalid input.\n");
    } 
    n_accounts = arg;

    //File handling for output file
    strcpy(filename, argv[3]);
    strcat(filename, ".txt");
    fptr = fopen(filename, "w+");
    if(fptr == NULL) {
        perror("Bad file handling\n");
        exit(1);
    }

    printf("Launched %d worker threads with %d accounts. "
            "The output will be %s.\n",
            n_workers, n_accounts, filename);


    // Ready to run bank server and take requests
    while(running) {
        ret = initialize_accounts(n_accounts);
        if(ret != 0) {
            perror("Unsuccessful account initializations.\n");
        }
        p = fgets(buffer, ARG_MAX, stdin);
        if(p == NULL){
            perror("Looks like there was problems taking input.\n");
        }
        ret = read_command(buffer, params);
        command = params[0];
        printf("The command was \"%s\" and the argc was: %d\n", command, ret);
        fprintf(fptr, "%s", command);

        //Append user input to file to record
        //TODO: ensure only doing this when no threads are
        for(i = 1; i<ret; i++) {
            fprintf(fptr, " %s", params[i]);
        }
        fprintf(fptr, "\n");

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
    } else { return 0; } }
