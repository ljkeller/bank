#include "Bank.h"
#include <sys/time.h>
#include <errno.h>

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
    } else { 
        return 0; 
    } 
}

void imm_response(int request_id){
    printf("ID %d\n", request_id);
}

int proc_trans(){
    return 0;
}

void write_file(FILE *fptr, char* params[], int argc){
    int i = 0;
    for(i = 0; i<argc; i++) {
        fprintf(fptr, " %s", params[i]);
    }
}
