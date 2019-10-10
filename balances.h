#ifndef BALANCES_H
#define BALANCES_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#define __USE_XOPEN
#include <time.h>


typedef struct bitcoin{
	int id;
	int numOftransacts;
	struct tree_node* tree;
	
}bitcoin;

typedef struct bitcoin_list{
	bitcoin* bitc;
	int value;
	struct bitcoin_list* next;
}bitcoin_list;


typedef struct bitC_buck{
	int numOfbitCoins;
	bitcoin** bitCoins;
	struct bitC_buck* next;

}bitC_buck;

typedef struct bitC_hashT{
	int size;
	int buck_s;
	bitC_buck** buckets;
}bitC_hashT;

typedef struct wallet{
	char walletId[128];
	int numOfbitCoins;
	bitcoin_list* bitCs;	
	int total;
	
}wallet;

//___________________________________________________

typedef struct transaction{
	char descript[128];
	char* id ;
	struct tm date;
	int amount;

}transaction;

typedef struct transact_list{
	transaction* tr;
	struct transact_list* next;
}transact_list;

typedef struct sr_buck{	
	int numOfentries;
	wallet** wallets;
	transact_list** tr_list;
	//transaction array
	struct sr_buck* next;

}sr_buck;

typedef struct sr_hashT{
	int size;
	int buck_s;
	sr_buck** buckets;
}sr_hashT;

typedef struct tree_node{
	int amount;
	wallet* w;
	transaction* tr;
	struct tree_node* left;
	struct tree_node* right;
	int visited;
}tree_node;



//_______________________________________________________

int store_arguments(int argc, char** argv, char** balances, char** transactions, int* bit_val, int* h1, int* h2, int* buck_s);
int findnumOfwallets(char* balances);

int isPrime(int n);
int next_prime(int value);
int bitC_hashFunc(int bitC, int hash_size);

bitC_hashT* load_balances( wallet** wallet_arr,char* balances, int numOfusers , int bit_val,
 sr_hashT** sender_ht, sr_hashT** receiver_ht );

wallet* find_walletId(char* key, sr_hashT* ht);
bitcoin* find_bitCoinId(bitC_hashT* bitC_ht, int val);
int check_for_dublicates(bitcoin_list* bitC_list, bitC_hashT* bitC_ht);

int insert_newWallet( char* entry, wallet* wallets,int  val,int  i, bitC_hashT* bitC_ht, sr_hashT* sender_ht);
bitC_hashT* addToHash_bitC(bitcoin* bitc, bitC_hashT* bitC_ht );
bitcoin_list* add_bitCoin(bitcoin_list** head, bitcoin_list* curr,int bitc, int val);

//___________________________________________________________________________________

int hash_func(char* s, int hash_size);
sr_hashT* create_hashT( int numOfwallts, int h, int buck_s);
sr_hashT* addToHash_walletId(wallet*  wallet, sr_hashT* sr_ht);

//_____________________________________________________________________________
void print_bitCoinHash(bitC_hashT* bitC_ht);
void print_wallets(wallet* wallets, int numOfwallts);
void print_wallet(wallet* wallt);
void print_srHash(sr_hashT* sr_ht, int flag);
void print_transactList(transact_list* list);
void printTree(tree_node *root) ;

//__________________________________________________

void free_tree(tree_node* node);
void free_bitcoinList(bitcoin_list* bitCs);
void free_wallets(wallet* wallets, int numOfusers);
void free_srBucket(sr_buck* buck, int buck_s);
void free_transactList(transact_list* list);
void free_srHash(sr_hashT* ht);
void free_bitcHash(bitC_hashT* ht);
void free_bitCBucket(bitC_buck* buck, int buck_s);
void free_failedwallet(bitcoin_list* bitCs);


#endif 