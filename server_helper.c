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
