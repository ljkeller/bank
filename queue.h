#ifndef QUEUE_H
#define QUEUE_H

#include <stdint.h>
#include <sys/time.h>

#define CHECK 1
#define TRANS 2
#define END 3

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
    struct job *head, *tail; //The start and and of the queue
    int num_jobs; //The number of jobs present in the queue
};

/*
 *  Initiate the given struct with size, head, tail, ect...
 *  Input:  struct queue - The queue struct interested in
 */
void init_queue(struct queue *q);

/*
 *  Enter a new job into the queue at the end of the queue
 *  Input:  struct queue - The queue struct interested in
 *  Input:  struct job - The job to be added to the queue
 */
void push(struct queue *q, struct job *j);
 
/*
 *  Return the head of the queue, removing it from the list
 *  Input:  struct queue - The queue struct interested in
 *  Return: Job at head of queue 
 */
struct job* pop(struct queue *q);

/*
 *  Return the number of jobs in the queue
 *  Input:  struct queue - The queue struct interested in
 *  Return: Number of queue elements (aka number of jos)
 */
int q_size(struct queue *q);

struct job* new_job(int req_id);

struct trans* new_trans(int acc_id, int amount);

#endif /*end guard*/
