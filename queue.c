#include "queue.h"
#include <errno.h>
#include <stdio.h>
#include <sys/time.h>
#include <stdlib.h>
#include <stdint.h>

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
    struct job jobs[10], *ret;
    struct trans transactions[10];

    printf("Initializing 10 accounts and putting into queue");
    for(i = 0; i<10; i++) {
        jobs[i].num_trans = 1;
        gettimeofday(&jobs[i].start_time, NULL);
        jobs[i].request_id = i;
        transactions[i].acc_id = i;
        transactions[i].amount = i * 10;
        jobs[i].transactions = &transactions[i];
        push(&q, &jobs[i]);
    }

    for(int i = 0; i<10; i++) {
        ret = pop(&q);
        printf("Getting rid of %d, the next reqID is %d.", ret->request_id, 
                ret->next->request_id);
        printf("This node was recieved at %ld.", ret->start_time.tv_sec);
        printf("There are currently %d items in queue.\n", q_size(&q));
    }
    return 1;
}
*/
