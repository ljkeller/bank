#include <errno.h>
#include <limits.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

int validate_input(char* p, long arg, int err);

int main(int argc, char* argv[]) {
    char *p;
    char filename[64];
    int n_workers, n_accounts, running;
    errno = 0;
    FILE *fptr;

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
        perror("It looks like there was in invalid input");
    } 
    n_accounts = arg;

    //File handling for output file
    strcpy(filename, argv[3]);
    strcat(filename, ".txt");
    fptr = fopen(filename, "w+");
    if(fptr == NULL) {
        perror("Bad file handling");
        exit(1);
    }

    printf("Launched %d worker threads with %d accounts. "
            "The output will be %s\n",
            n_workers, n_accounts, filename);


    // Ready to run bank server and take requests
    while(running) {
        break;
    }

    fclose(fptr);
    return 0; 
}

int validate_input(char* p, long arg, int err) {
    if(*p != '\0' || err != 0) {
        return 1;
    } else if(arg < INT_MIN || arg > INT_MAX) {
        return 2;
    } else { return 0; } }
