#include "queue.h"
#include <stdio.h>
#include <sys/time.h>
#include <stdlib.h>
#include <stdint.h>

int main() {
    int i;
    struct queue q;  
    init_queue(&q);
    struct job jobs[10];
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
    return 1;
}
