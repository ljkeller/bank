#include "queue.h"
#include <errno.h>
#include <stdio.h>
#include <sys/time.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h> //for usleep

struct job* new_job(int req_id) {
    struct job *j = malloc(sizeof(struct job));
    struct timeval placeholder, curtime;
    gettimeofday(&curtime, NULL);
    j->type = 0;
    j->next = NULL;
    j->request_id = req_id;
    j->check_acc_id = 0; //Accs start at 0 goto n
    j->transactions = NULL;
    j->num_trans = 0;
    j->start_time = curtime;
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
