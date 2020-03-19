#include <stdint.h>
#include <sys/time.h>

struct trans { 
    int acc_id; // account ID
    int amount; // amount for transaction
};

struct job {
    uint8_t type;
    struct job *next; //pointer to next request in the list
    int request_id; //request ID assigned by main thread
    int check_acc_id; //account ID for a CHECK request
    struct trans *transactions; //array of transaction data
    int num_trans; // number of accounts in this transaction
    struct timeval start_time, end_time; //start and end time for TIME
};

struct queue {
    struct job *head, *tail;
    int num_jobs;
};

void init_queue(struct queue *q);

void push(struct queue *q, struct job *j);

struct job* pop(struct queue *q);
