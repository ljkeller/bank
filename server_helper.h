#include "Bank.h"
#include <sys/time.h>
#include <errno.h>

int deduct_balance(int account_id, int amount);

int add_balance(int account_id, int amount);

int transfer_balance(int src_acct_id, int dst_acct_id, int amount);

int has_balance(int account_id, int amount);

void write_file(FILE *fptr, char* params[], int argc;

int proc_trans();

void imm_response();

int validate_input(char *p, long arg, int err);

int read_command(char command[], char *params[]);
