#include "server_helper.h"

/*
 * Deduct a balance from a bank account
 * Input: int account_id - ID of bank account to deduct from
 * Input: int amount - amount to deduct
 * return: 0 on success or errno on error
 */
int deduct_balance(int account_id, int amount) {
    int balance = read_account(account_id);
    if(balance < amount) {
        return EBADE; //Bad exchange error
    } else {
        balance = balance - amount;
        write_account(account_id, balance - amount);
        return 0;
    }
}

/*
 * Add a balance to a bank account
 * Input: int account_id - ID of bank account to add value to
 * Input: int amount - amount to add
 * return: 0 on success 
 */
int add_balance(int account_id, int amount) {
    int balance = read_account(account_id);
    write_account(account_id, balance + amount);
    return 0;
}
    
/*
 * Check to see that an account has a specified amount of money
 * Input: int account_id - ID of bank account check balance of
 * Input: int amount - Minimal balance that account must have to return positive number
 * return: Positive number for supporting the balance, negative number if not meeting balance
 */
int has_balance(int account_id, int amount){
    int balance = read_account(account_id);
    return balance - amount;
}

/*
 * Deduct a balance from one bank account and add to a destination account
 * Input: int src_acct_id - ID of bank account to deduct from 
 * Input: int dst_acct_id - ID of bank account to add balance to
 * Input: int amount - amount to deduct from source and add to destination
 * return: 0 on success, errno on failure 
 */
int transfer_balance(int src_acct_id, int dst_acct_id, int amount) {
    //TODO: Think on necessity of checker for negative balance    
    int ret = has_balance(src_acct_id, amount);
    if(ret < 0) {
        return EBADE;
    }
    deduct_balance(src_acct_id, amount);
    add_balance(dst_acct_id, amount);
    return 0;
}

/*
 * Parses user input to return array of (char) argv
 * Input: char command[] - contigous char array of user input
 * Input:  char *params[]  - argv structure for parsed command into its parameters
 * return: argc, the number of parameter items present including command
 */
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


/*
 * Ensures that a given argument is a valid sized integer converted from a char
 * Input: char char* p - the char pointer representing the end of the converted arg
 * Input: long arg - the long variable converted from a char array. Check to see if int sized
 * Input: int err - the error code returned from strtol
 * return: 0 on success, or errno on error
 */
int validate_input(char* p, long arg, int err) {
    if(*p != '\0' || err != 0) {
        return EINVAL;
    } else if(arg < INT_MIN || arg > INT_MAX) {
        return ERANGE;
    } else { 
        return 0; 
    } 
}

/*
 * Posts an immediate response to stdout to let customer know request is being processed
 * Input: int request_id - the ID that corresponds to this user request
 */
void imm_response(int request_id){
    printf("ID %d\n", request_id);
}

int proc_trans(){
    return 0;
}

/*
 * Writes given parameters to file to log behaviour
 * Input: FILE *fptr - the file being written
 * Input: char* params[] - given parameters being logged
 * Input: int argc - the number of parameters being written
 */
void write_file(FILE *fptr, char* params[], int argc){
    int i = 0;
    for(i = 0; i<argc; i++) {
        fprintf(fptr, " %s", params[i]);
    }
}

void print_job(struct job *job) {
    printf("Type: %d\tReq_id: %d\tCheck_acct: %d\tnum_trans: %d\tTime start: %ld.%06ld\n",
            job->type,
            job->request_id,
            job->check_acc_id,
            job->num_trans,
            job->start_time.tv_sec, job->start_time.tv_usec);
}

int params_to_job(char *params[], int argc, struct job *request, int req_id) {
    char *p;
    struct trans *transactions;
    long ret_amount, ret_acct_id, ret;
    int i = 0, num_trans;
    struct timeval cur_time;
    if(argc < 1) { //Insufficient args
        return EINVAL;
    }
     
    //TODO: THis time might need fixing
    ret = gettimeofday(&cur_time, NULL);
    if(ret != 0) {
        perror("Time-related issue.");
        return ret;
    }

    request->start_time = cur_time;
    request->request_id = req_id;
    if(strcmp(params[0], "END") == 0) {
        request->type = END;
    } else if(strcmp(params[0], "CHECK") == 0) {
        request->type = CHECK;
        if(argc != 2) {
            errno = EINVAL;
            perror("It looks like there were an invalid number of args");
            return EINVAL;
        }
        ret_acct_id = strtol(params[1], &p, 10); 
        errno = validate_input(p, ret_acct_id, errno); //errno is thread safe/thread specific!
        if(errno == 0) {
            request->check_acc_id = ret_acct_id;
        } else {
            errno = EINVAL;
            perror("It looks like there was an invalid account ID");
            return EINVAL;
        }
    } else if(strcmp(params[0], "TRANS") == 0) {
        request->type = TRANS;
        if((argc - 1) % 2 != 0) { //Need account_ID and amount for every transaction
            errno = EINVAL;
            perror("It looks like there were an invalid amount of transfers.");
            return EINVAL;
        }
        
        num_trans = (argc - 1) / 2;
        request->num_trans = num_trans; //Dont update transactions here, just number
        transactions = (struct trans*) malloc(sizeof(struct trans) * num_trans);
        
        for(i = 1; i <= num_trans; i++) {
            ret_acct_id = strtol(params[i], &p, 10);
            errno = validate_input(p, ret_acct_id, errno);
            if(errno != 0) {
                perror("It looks like there was an error with acct_id");
                free(transactions);
                return EINVAL;
            }
            ret_amount = strtol(params[i + 1], &p, 10);
            errno = validate_input(p, ret_amount, errno);
            if(errno != 0) {
                perror("It looks like there was an error with an amount");
                free(transactions);
                return EINVAL;
            }
	        transactions[i].acc_id = ret_acct_id;
	        transactions[i].amount = ret_amount;
        }
        request->transactions = transactions;
    } else {
        return EINVAL; //invalid input
    }
    return 0;
     
}
