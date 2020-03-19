#include <sys/time.h>
#include <errno.h>
#include <limits.h>
#include <string.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "queue.h"
#include "Bank.h"


/*
 * Deduct a balance from a bank account
 * Input: int account_id - ID of bank account to deduct from
 * Input: int amount - amount to deduct
 * return: 0 on success or errno on error
 */
int deduct_balance(int account_id, int amount);

/*
 * Add a balance to a bank account
 * Input: int account_id - ID of bank account to add value to
 * Input: int amount - amount to add
 * return: 0 on success 
 */
int add_balance(int account_id, int amount);

/*
 * Deduct a balance from one bank account and add to a destination account
 * Input: int src_acct_id - ID of bank account to deduct from 
 * Input: int dst_acct_id - ID of bank account to add balance to
 * Input: int amount - amount to deduct from source and add to destination
 * return: 0 on success, errno on failure 
 */
int transfer_balance(int src_acct_id, int dst_acct_id, int amount);

/*
 * Check to see that an account has a specified amount of money
 * Input: int account_id - ID of bank account check balance of
 * Input: int amount - Minimal balance that account must have to return positive number
 * return: Positive number for supporting the balance, negative number if not meeting balance
 */
int has_balance(int account_id, int amount);

/*
 * Writes given parameters to file to log behaviour
 * Input: FILE *fptr - the file being written
 * Input: char* params[] - given parameters being logged
 * Input: int argc - the number of parameters being written
 */
void write_file(FILE *fptr, char* params[], int argc);

int proc_trans();

/*
 * Posts an immediate response to stdout to let customer know request is being processed
 * Input: int request_id - the ID that corresponds to this user request
 */
void imm_response();

/*
 * Ensures that a given argument is a valid sized integer converted from a char
 * Input: char char* p - the char pointer representing the end of the converted arg
 * Input: long arg - the long variable converted from a char array. Check to see if int sized
 * Input: int err - the error code returned from strtol
 * return: 0 on success, or errno on error
 */
int validate_input(char *p, long arg, int err);

/*
 * Parses user input to return array of (char) argv
 * Input: char command[] - contigous char array of user input
 * Input:  char *params[]  - argv structure for parsed command into its parameters
 * return: argc, the number of parameter items present including command
 */
int read_command(char command[], char *params[]);

int params_to_job(char *params[], int argc, struct job *request, int req_id);

void print_job(struct job *job); 
