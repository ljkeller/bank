#include "queue.h"
#include <errno.h>
#include <stdio.h>
#include <sys/time.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h> //for usleep

struct job* new_job(int req_id) {
    struct job *j = malloc(sizeof(struct job));
    struct timeval placeholder;
    j->type = 0;
    j->next = NULL;
    j->request_id = req_id;
    j->check_acc_id = 0; //Accs start at 0 goto n
    j->transactions = NULL;
    j->num_trans = 0;
    gettimeofday(&(j->start_time), NULL);
    j->end_time = placeholder;
    return j;
}

void update_endt(struct job *j) {
    if(j != NULL) {
        gettimeofday(&(j->end_time), NULL);
    }
}

struct trans* new_trans(int acc_id, int amount) {
    struct trans *t = malloc(sizeof(struct trans));
    t->acc_id = acc_id;
    t->amount = amount;
    return t;
}

void init_trans(struct trans *t, int acc_id, int amount) {
    t->acc_id = acc_id;
    t->amount = amount;
}

void init_queue(struct queue *q) {
    q->head = NULL;
    q->tail = NULL;
    q->num_jobs = 0;
}

void push(struct queue *q, struct job *j) {
    if(j == NULL) {
        return;
    }
    if(q->num_jobs < 1){
        q->head = j;
    } else {
        q->tail->next = j;
    }
    q->tail = j;
    q->num_jobs++;
    printf("Just put %d into queue for %d items\n", j->request_id, q->num_jobs);
}

struct job* pop(struct queue *q) {
    if(q->num_jobs < 1) {
        errno = EINVAL;
        return NULL;
    }
    struct job *j = q->head;
    q->head = q->head->next;
    q->num_jobs--;
    return j;
}

int q_size(struct queue *q) {
    return q->num_jobs;
}

/*
int main() {
    int i;
    struct queue q;  
    init_queue(&q);
    struct job jobs[10], *ret, *j;
    struct trans transactions[10], *t;

    printf("Initializing 10 accounts and putting into queue");
    for(i = 0; i<10; i++) {
        j = new_job(i);
        j->num_trans = 1;
        t = new_trans(i, i*10);
        j->transactions = t;
        push(&q, j);
        usleep(i);
    }

    for(int i = 0; i<10; i++) {
        ret = pop(&q);
        printf("Getting rid of %d, the next reqID is %d.", ret->request_id, 
                ret->next->request_id);
        printf("This node was recieved at %ld.%06ld.", 
                ret->start_time.tv_sec, ret->start_time.tv_usec);
        printf("There are currently %d items in queue.\n", q_size(&q));
    }
    return 1;
}
*/
