#ifndef COMMANDS_H
#define COMMANDS_H

#include "transactions.h"

int requestTransaction(char* entry, sr_hashT* sender_ht, sr_hashT* receiver_ht, tr_hashT* tr_ht, struct tm* last_date, int* maxId, int flag );
int requestTransactions(char* entry, sr_hashT* sender_ht, sr_hashT* receiver_ht, tr_hashT* tr_ht, struct tm* last_date, int* maxId, int flag);
int findEarn_Paym(char* entry, sr_hashT* ht, int flag, struct tm last_date);
int totalEarn_Paym(struct tm* start, struct tm* end, transact_list* list, int flag, int func, struct tm last_date);
int walletStatus(char* entry, sr_hashT* ht );
void findUnspent(tree_node *root, char* walletId, int* unspent);
int bitcoinStatus(char* entry, bitC_hashT* ht, int bitCval);
void TransactsOfbitc(tree_node *root);
int traceCoin(char* entry , bitC_hashT* ht);




#endif