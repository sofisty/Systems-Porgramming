#ifndef TRANSACTIONS_H
#define TRANSACTIONS_H

#include "balances.h"

typedef struct tr_buck{
	int numOftransact;
	transaction* trctions;
	struct tr_buck* next;

}tr_buck;

typedef struct tr_hashT{
	int size;
	int buck_s;
	tr_buck** buckets;
}tr_hashT;




tr_hashT* load_transactions(char* transactions, sr_hashT* sender, sr_hashT* receiver, struct tm* last_date, int* maxId);

int find_transactId(tr_hashT *tr_ht, char* id);
int valid_amount(wallet* w, int amount);
transact_list* transactListOfWalletId(char* key, sr_hashT* ht, int* found);

transact_list* add_trasnact(transact_list* list, transaction* tr);
int insert_newTransaction(char* entry, sr_hashT* sender, sr_hashT* receiver, tr_hashT* tr_ht,struct tm* last_date, int* maxId );
tr_hashT* addToHash_tr(char descript[128] ,char* id, int amount,struct tm date, tr_hashT* tr_ht, transaction** new_tr);
sr_hashT* update_transactList(sr_hashT* sr_ht, transaction* new_tr, char* walletId);


bitcoin_list* swap_listNode(bitcoin_list* list, bitcoin_list* node ,int val,  bitcoin_list** new_curr);
bitcoin_list* receive_bitc(bitcoin_list* list, bitcoin* bitc, int val);
tree_node* insert_treeNode(tree_node* tree, int amount, transaction* tr, wallet* w);
void update_tree(tree_node** root,tree_node *node, int send_val, int rec_val, int init_val, wallet* sender, wallet* receiver, transaction* tr, int* done); 
wallet* exec_transaction( wallet* sender , wallet** receiver, transaction* tr );

void print_trHash(tr_hashT* tr_ht);

void free_trBucket(tr_buck* buck);
void free_trHash(tr_hashT* ht);

#endif