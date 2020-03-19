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

int proc_transactions(struct trans *t, int n_jobs) {
    int i, k, ret = 0;
    for(i = 0; i < n_jobs; i++) {
        ret = proc_transaction(&(t[i]));
        if(ret != 0) {
            for(k = i - 1; k > -1; k--) {
                undo_transaction(&(t[k]));
            }
            return t->acc_id;
        }
    }
    return 0;
}

int proc_transaction(struct trans *t) {
    return 0;
}

void undo_transaction(struct trans *t) {
}

int process_job(struct job *j, FILE *fptr) {
    int ret;
    if(j == NULL || j->type == 0) {
        errno = EINVAL;
        return EINVAL;
    };
    if(j->type == CHECK) { 
        update_endt(j);
        fprintf(fptr, "%d BAL %d TIME %ld.%06.ld %ld.%06.ld", 
                j->request_id, j->check_acc_id, 
                j->start_time.tv_sec, j->start_time.tv_usec,
                j->end_time.tv_sec, j->end_time.tv_usec);

    } else if(j->type == TRANS) {
        ret = proc_transactions(j->transactions, j->num_trans); //Ensure this frees jobs
        update_endt(j);
        if(ret == 0) { //Successfully processed transactions
            fprintf(fptr, "%d OK TIME %ld.%06.ld %ld.%06.ld", 
                    j->request_id,
                    j->start_time.tv_sec, j->start_time.tv_usec,
                    j->end_time.tv_sec, j->end_time.tv_usec);
        } else {
            fprintf(fptr, "%d ISF %d %ld.%06.ld %ld.%06.ld", 
                    j->request_id, ret, 
                    j->start_time.tv_sec, j->start_time.tv_usec,
                    j->end_time.tv_sec, j->end_time.tv_usec);
        } 
    } else { //j->type == END
            return -1;
    }

    return 0;
}


struct trans* new_trans(int acc_id, int amount) {
    struct trans *t = malloc(sizeof(struct trans));
    t->acc_id = acc_id;
    t->amount = amount;
    return t;
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
