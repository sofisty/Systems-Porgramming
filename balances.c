#include "balances.h"
#define AVERAGE_BITCS 64
#define BITC_BUCK 64

int isPrime(int n) {// assuming n > 1
    int i;
    int root;
	 if (n%2 == 0 || n%3 == 0)return 0;
	root = (int)sqrt(n);

    for (i=5; i<=root; i+=6){if (n%i == 0)return 0;}

    for (i=7; i<=root; i+=6){if (n%i == 0)return 0;}

    return 1;
}

//returns next prime of value
int next_prime(int value){
	if(value==0)return 1;
	if(value==1)return 2;

	if(	(value+1)%2==0){value+=2;}
	else {value+=1;}

	while(isPrime(value)!=1)value++;
	return value;		
}

int bitC_hashFunc(int bitC, int hash_size){
	return bitC%hash_size;
}


int store_arguments(int argc, char** argv, char** balances, char** transactions, int* bit_val, int* h1, int* h2, int* buck_s){
	int i, size ;
	int fl_a=0, fl_t=0, fl_v=0, fl_h1=0, fl_h2=0, fl_b=0;
	char* wrong_args="Incorrect arguments supplied";

	if(argc!=13){
		fprintf(stderr, "%s\n",wrong_args);
		return -1;
	}
	else{
		i=0;
		while(i<12){
			if( strcmp(argv[i], "-a") == 0){
				fl_a++;
				if(fl_a>1){fprintf(stderr, "%s\n",wrong_args); return -1;} //an dothei 2 fores to idio argument
				
				size=strlen(argv[i+1])+1;
				*balances=malloc( size* sizeof(char));
				memset( *balances, '\0', size*sizeof(char));
				strcpy(*balances, argv[i+1]); //to onoma tou arxeiou

				
			}
			else if(strcmp(argv[i], "-t") == 0){
				fl_t++;
				if(fl_t>1){fprintf(stderr, "%s\n",wrong_args); return -1;}
				
				size=strlen(argv[i+1])+1;
				*transactions=malloc( size* sizeof(char));
				memset( *transactions, '\0', size*sizeof(char));			
				strcpy(*transactions, argv[i+1]);
				
			}
			else if(strcmp(argv[i], "-v") == 0){
				fl_v++;
				if(fl_v>1){fprintf(stderr, "%s\n",wrong_args); return -1;}
				
				*bit_val=strtol(argv[i+1], NULL, 10);
			

			}
			else if(strcmp(argv[i], "-h1") == 0){
				fl_h1++;
				if(fl_h1>1){fprintf(stderr, "%s\n",wrong_args); return -1;}

				*h1=strtol(argv[i+1], NULL, 10);
			
			}
			else if(strcmp(argv[i], "-h2") == 0){
				fl_h2++;
				if(fl_h2>1){fprintf(stderr, "%s\n",wrong_args); return -1;}

				*h2=strtol(argv[i+1], NULL, 10);
				
			}
			else if(strcmp(argv[i], "-b") == 0){
				fl_b++;
				if(fl_b>1){fprintf(stderr, "%s\n",wrong_args); return -1;}
				
				*buck_s=strtol(argv[i+1], NULL, 10);
			
			}
			i++;
		}

		if( (fl_a+fl_t+fl_v+fl_h1+fl_h2+fl_b)!=6 ){ //exun dothei ta swsta arguments
			fprintf(stderr, "%s\n",wrong_args );
			return -1;
		}

		return 0;
	}  

}

//_____________________________________BITCOINS WALLETS________________________________________________
 
 //epistrefei ton arithmo twn grammwn tou arxeiou 
 int findnumOfwallets(char* balances){
 	FILE *b_fp;
 	ssize_t nread;
 	int numOfusers=0;
 	char *line = NULL;
    size_t len = 0;
 	b_fp = fopen (balances,"r");
    if (b_fp == NULL) {fprintf(stderr, "Cannot open balances file\n" ); exit(0);}

    while ((nread = getline(&line, &len,b_fp)) != -1) {numOfusers+=1;}
    fclose(b_fp);
    free(line);
    return numOfusers;
 }

//anoigei to arxeio balances shmiourgei enan array apo wallets kai epistrefei ena hash table me ta bitcoins
 bitC_hashT* load_balances( wallet** wallet_arr,char* balances, int numOfusers , int bit_val,
 sr_hashT** sender_ht,sr_hashT** receiver_ht ){
	FILE *b_fp;
	char *line = NULL;
    size_t len = 0;
    ssize_t nread;
    int bitCs_hashSize;
    
    int i, r;
    wallet* wallets=NULL;
    bitC_hashT* bitC_ht=NULL;
	
    b_fp = fopen (balances,"r");
    if (b_fp == NULL) {fprintf(stderr, "Cannot open balances file\n" ); exit(0);}

    //apofasizei to megethos tou hash table
    bitCs_hashSize=1.3*numOfusers*AVERAGE_BITCS; //load factor 70%
    if(bitCs_hashSize%2==0)bitCs_hashSize+=1;
    bitCs_hashSize=next_prime(bitCs_hashSize);
   
    bitC_ht=malloc(sizeof(bitC_hashT));
    if(bitC_ht==NULL){fprintf(stderr, "Malloc bitcoin's hash table failed\n" ); exit(0);}
    bitC_ht->size=bitCs_hashSize;
    bitC_ht->buck_s=BITC_BUCK;
    bitC_ht->buckets=malloc(bitCs_hashSize* sizeof(bitC_buck*));
    for(i=0; i<bitCs_hashSize; i++){ //arxikopoiei to hash table twn bitcoins
    	bitC_ht->buckets[i]=malloc( sizeof(bitC_buck));
    	bitC_ht->buckets[i]->numOfbitCoins=0;
    	bitC_ht->buckets[i]->bitCoins=NULL;
    	bitC_ht->buckets[i]->next=NULL;

    }
    
    wallets=malloc(numOfusers* sizeof(wallet));
    if(wallets==NULL){fprintf(stderr, "Malloc wallets failed\n"); exit(0);}
    for(i=0; i<numOfusers; i++){wallets[i].bitCs=NULL; wallets[i].numOfbitCoins=0;}

   
    i=0;
    while ((nread = getline(&line, &len,b_fp)) != -1){ //gia kathe grammh tou arxeiou kanei eisagwgh to antistoixo wallet 
    	
    	r=insert_newWallet( line, wallets, bit_val, i, bitC_ht, *sender_ht);
    	if(r==-1){ fclose(b_fp); free(line); free_bitcHash(bitC_ht); *wallet_arr=wallets; return NULL;} //apotuxia eisagwghs
    	
    	*sender_ht= addToHash_walletId( &(wallets[i]), *sender_ht); //enhmerwnei ta hash tables me ta kainourgia wallets
		*receiver_ht= addToHash_walletId( &(wallets[i]), *receiver_ht);
  	 	i++;
    
    }
 
    free(line);
    fclose(b_fp);
    *wallet_arr=wallets;

    return bitC_ht;

}

//vriskei to bitcoin me id=val kai to epistrefei
bitcoin* find_bitCoinId(bitC_hashT* bitC_ht, int val){
	int i,j, numOfbitCoins;
	if(bitC_ht==NULL){fprintf(stderr, "Bit coin's hash table does not exist\n" ); exit(0);}
	int size=bitC_ht->size;

	i=bitC_hashFunc(val, size);

	bitC_buck* curr= bitC_ht->buckets[i];
	while(curr!=NULL){
		numOfbitCoins=curr->numOfbitCoins;
		for(j=0; j<numOfbitCoins; j++){
			if(val == curr->bitCoins[j]->id) {return curr->bitCoins[j];}
		}
		curr=curr->next;
	}

	return NULL;
}

//anazhta sto hash tbale twn wallets(sender, receiver) an uparxei kapoios me to idio walletId
//an vrethei epistrefei to wallet tou, diaforetika NULL
wallet* find_walletId(char* key, sr_hashT* ht){
	int size,i, numOfentries,j;
	if(ht==NULL){fprintf(stderr, "Hash Table does not exist\n" ); return NULL; }
	size=ht->size;
	i=hash_func(key, size);
	sr_buck* curr = ht->buckets[i];
	if(ht->buckets[i]==NULL){fprintf(stderr, "S/R Bucket %d does not exist\n",i ); return NULL;}

	while(curr!=NULL){
		numOfentries=curr->numOfentries;
		for(j=0; j<numOfentries; j++){
			if(strcmp (curr->wallets[j]->walletId, key)==0) { return curr->wallets[j];}
		}
		curr=curr->next;
	}

	return NULL;

}

//eisagei ena kainourgio wallet ston array wallets sthn thesh i
int insert_newWallet( char* entry, wallet* wallets, int val,int  i, bitC_hashT* bitC_ht, sr_hashT* sender_ht){
	int  numOfbitCoins;
	char *token=NULL;
	bitcoin_list* curr=NULL;
	wallet* new_wallet=&wallets[i];

	token=strtok(entry, " \t\r\n");
	if(token==NULL){fprintf(stderr, "Not apropirate format of entry\n" ); return -1;}

	//eksetazei an uparxei hdh to walletId
	if(find_walletId(token, sender_ht)!=NULL) {
		fprintf(stderr,"---Wallet %s rejected---\n", token);
		fprintf(stderr, "Found doublicate walletId\n" ); return -1;
	} 
	memset(new_wallet->walletId, '\0', 128*sizeof(char));
	strcpy(new_wallet->walletId, token);
	
	new_wallet->bitCs=NULL;
	curr=new_wallet->bitCs;
	numOfbitCoins=0;
	while( token != NULL) {
  		token = strtok(NULL, " \t\r\n");
  		
  		if(token!=NULL){
			curr= add_bitCoin(	&(new_wallet->bitCs), curr, strtol(token, NULL, 10), val);
			//eksetazei an kapoio apo ta dothenta bitcoins uparxei ksana
			if(find_bitCoinId(bitC_ht, curr->bitc->id)!=NULL){
				printf("Warning! Found doublicate bitcoin id : %d \n",curr->bitc->id );
				printf("---Wallet %s rejected---\n", new_wallet->walletId);
		  		memset(new_wallet->walletId, '\0', 128*sizeof(char));
		  		free_failedwallet(new_wallet->bitCs);
		  		new_wallet->bitCs=NULL;
		  		return -1;
			}			  			
   			numOfbitCoins+=1;				
  		}
  		
	}
	
	//ena ena ta bitcoins tou wallet apothhkeuontai kai sto hash table twn bitcoins
  	curr=new_wallet->bitCs;
  	while(curr!=NULL){		
  		bitC_ht =addToHash_bitC( curr->bitc, bitC_ht);
  		curr=curr->next;
  	}
  			 
	new_wallet->numOfbitCoins=numOfbitCoins;
	new_wallet->total=numOfbitCoins*val;

	

	return 0;
}

//prosthetei ena bitcoin* sthn katallhlh thesh tou hash table 
bitC_hashT* addToHash_bitC(bitcoin* bitc, bitC_hashT* bitC_ht){
	int i,j;
	bitC_buck* curr;
	i=bitC_hashFunc(bitc->id, bitC_ht->size);
	if(bitC_ht==NULL){fprintf(stderr, "Something went wrong, bitc_ht = NULL\n" ); exit(0);}
	if(bitC_ht->buckets==NULL){fprintf(stderr, "Something went wrong, bitc_ht->buckets = NULL\n" ); exit(0);}
	if(bitC_ht->buckets[i]==NULL){fprintf(stderr, "Malloc bucket i, in Bitcoin's hash table \n"); exit(0);}
	if(bitC_ht->buckets[i]->bitCoins==NULL){	
		
		bitC_ht->buckets[i]->bitCoins=malloc(BITC_BUCK* sizeof(bitcoin*));
		if(bitC_ht->buckets[i]->bitCoins==NULL){fprintf(stderr, "Malloc bucket's bitCoin array failed\n"); exit(0);}
		bitC_ht->buckets[i]->bitCoins[0]=bitc;
		bitC_ht->buckets[i]->numOfbitCoins=1;
	
		return  bitC_ht;
	}
	else{
		curr=bitC_ht->buckets[i];
		while(curr->next!=NULL){curr=curr->next;}

		if(curr->numOfbitCoins < BITC_BUCK){
			j=curr->numOfbitCoins;
			curr->bitCoins[j]=bitc;
			(curr->numOfbitCoins)+=1;
		}
		else{

			curr->next=malloc(sizeof(bitC_buck));
			if(curr->next==NULL){fprintf(stderr, "Malloc bitC_buck failed\n" ); exit(0);}
			curr->next->bitCoins=malloc(BITC_BUCK* sizeof(bitcoin*));
			if(curr->next->bitCoins==NULL){fprintf(stderr, "Malloc bucket's bitCoin array failed\n"); exit(0);}
			curr->next->next=NULL;
			curr->next->bitCoins[0]=bitc;
			curr->next->numOfbitCoins=1;

		}
	}

	
	return bitC_ht;

}

//dhmiourgei ena bitcoin kai to apothhkeuei sthn lista tou wallet katoxou
bitcoin_list* add_bitCoin(bitcoin_list** head, bitcoin_list* curr,int bitc, int val){
	
	if(*head==NULL){
		(*head)=malloc(sizeof(bitcoin_list));
		if( (*head)==NULL){fprintf(stderr, "Malloc bitcoin_list failed\n" ); exit(0);}
		(*head)->bitc=malloc(sizeof(bitcoin));
		(*head)->bitc->id=bitc;
		(*head)->value=val;
		(*head)->bitc->numOftransacts=0;
		(*head)->bitc->tree=NULL;
		(*head)->next=NULL;
		return (*head);
	}
	else{
		curr->next=malloc(sizeof(bitcoin_list));
		if(curr->next==NULL){fprintf(stderr, "Malloc bitcoin_list failed\n" ); exit(0);}
		curr->next->bitc=malloc(sizeof(bitcoin));
		curr->next->bitc->id=bitc;
		curr->next->value=val;
		curr->next->bitc->numOftransacts=0;
		curr->next->bitc->tree=NULL;
		curr->next->next=NULL;
		return curr->next;
	}
	
	return NULL;


}

//_______________________________________________SENDER RECEIVER HASH TABLE__________________________________________________

int hash_func(char* s, int hash_size){
	//apo K&R 
	unsigned hashval; //den uparxoun arnhtikes times

	for (hashval = 0; *s != '\0'; s++)
	    hashval = *s + 31*hashval; //exei shmasia h seira: ab!=ba
	return hashval % hash_size;
    

}

//arxikopoiei kai desmeuei mnhmh gia receiver/sender hash table
sr_hashT* create_hashT( int numOfwallts, int h, int buck_s){
	int i;
	sr_hashT* sr_ht=NULL;
	
	sr_ht=malloc(sizeof(sr_hashT));
    if(sr_ht==NULL){fprintf(stderr, "Malloc senders/receivers hash table failed\n" ); exit(0);}
    sr_ht->size=h;
    sr_ht->buck_s=buck_s;
    sr_ht->buckets=malloc(h* sizeof(sr_buck*));
    for(i=0; i<h; i++){
    	sr_ht->buckets[i]=malloc( sizeof(sr_buck));
    	sr_ht->buckets[i]->numOfentries=0;
    	sr_ht->buckets[i]->wallets=NULL;
    	sr_ht->buckets[i]->tr_list=NULL;
    	sr_ht->buckets[i]->next=NULL;

    }		
	
	return sr_ht;
}

//prosthetei enan wallet* sthn katallhlh thesh tou hash array
sr_hashT* addToHash_walletId(wallet*  wallet, sr_hashT* sr_ht){
	int i,j,k, buck_s;
	sr_buck* curr;
	i=hash_func(wallet->walletId, sr_ht->size);
	if(sr_ht==NULL){fprintf(stderr, "Something went wrong, sr_ht = NULL\n" ); exit(0);}
	buck_s=sr_ht->buck_s;

	if(sr_ht->buckets==NULL){fprintf(stderr, "Something went wrong, sr_ht->buckets = NULL\n" ); exit(0);}
	if(sr_ht->buckets[i]==NULL){fprintf(stderr, "Malloc bucket i, in Sender/Receiver's hash table \n"); exit(0);}
	if(sr_ht->buckets[i]->wallets==NULL){	
		
		sr_ht->buckets[i]->wallets=malloc( buck_s * sizeof( struct wallet*));
		sr_ht->buckets[i]->tr_list=malloc(buck_s* sizeof (transact_list*));
		if(sr_ht->buckets[i]->wallets==NULL){fprintf(stderr, "Malloc bucket's wallets array failed\n"); exit(0);}
		if(sr_ht->buckets[i]->tr_list==NULL){fprintf(stderr, "Malloc bucket's transactions list failed\n"); exit(0);}
		for( k=0; k<buck_s; k++){sr_ht->buckets[i]->tr_list[k]=NULL;}
		sr_ht->buckets[i]->wallets[0]=wallet;
		sr_ht->buckets[i]->numOfentries=1;
	
		return  sr_ht;
	}
	else{
		curr=sr_ht->buckets[i];
		while(curr->next!=NULL){curr=curr->next;}

		if(curr->numOfentries < buck_s){
			j=curr->numOfentries;
			curr->wallets[j]=wallet;
			(curr->numOfentries)+=1;
		}
		else{

			curr->next=malloc(sizeof(sr_buck));
			if(curr->next==NULL){fprintf(stderr, "Malloc sr_buck failed\n" ); exit(0);}
			curr->next->wallets=malloc(buck_s* sizeof( struct wallet*));
			if(curr->next->wallets==NULL){fprintf(stderr, "Malloc wallets  array failed\n"); exit(0);}
			curr->next->tr_list=malloc(buck_s* sizeof (transact_list *));
			if(curr->next->tr_list==NULL){fprintf(stderr, "Malloc transactions list failed\n"); exit(0);}
			for( k=0; k<buck_s; k++){curr->next->tr_list[k]=NULL;}
			curr->next->next=NULL;
			curr->next->wallets[0]=wallet;
			curr->next->numOfentries=1;

		}
	}

	
	return sr_ht;

}

//_________________________________________________PRINT______________________________________________________


void printTree(tree_node *root){ 
   
    if(root==NULL)return;  
    
    if(root->left==NULL && root->right==NULL) { 		
       if(root->visited==0)printf("\t\t leaf: %s with %d \n", root->w->walletId,root->amount);
        return; 
    }   

    if(root->left!=NULL){ 
    	if(root->visited==0)printf("\t\t  %s with %d \n\t from: %s\n",root->w->walletId,root->amount,root->tr->descript );
        printf("\t Go left\n");
       printTree(root->left); 
     }
          
    if(root->right!=NULL){ 
        if(root->visited==0)printf("\t\t %s with %d \n\t from: %s\n",root->w->walletId,root->amount,root->tr->descript  );
        printf("\t Go right\n");
       printTree(root->right); 
    }
}  


void print_bitCoinHash(bitC_hashT* bitC_ht){
	int i, j, size;
	if(bitC_ht==NULL){printf("Bitcoin hash table does not exist\n"); return;}
	size=bitC_ht->size;
	bitC_buck* curr;
	printf("--- Bit Coin Hash Table ---\n");
	for(i=0; i<size; i++){
		curr=bitC_ht->buckets[i];
		while(curr!=NULL){
			//printf("Bucket [%d] :",i );
			for(j=0; j<curr->numOfbitCoins; j++){
				if(j==0){printf("\nBucket [%d] :",i );}
				printf("> %d\n ",curr->bitCoins[j]->id );
				printTree(curr->bitCoins[j]->tree) ;
			}

			curr=curr->next;
		}
			
		
	}
	printf("\n");
}



void print_srHash(sr_hashT* sr_ht, int flag){
	int i, j, size;
	if(sr_ht==NULL){printf("S/R hash table does not exist\n"); return;}
	size=sr_ht->size;
	sr_buck* curr;
	if(flag){printf("--- Sender's  Hash Table ---\n");}
	else{printf("--- Receiver's  Hash Table ---\n");}
	
	for(i=0; i<size; i++){
		curr=sr_ht->buckets[i];
		
		while(curr!=NULL){
			
			for(j=0; j<curr->numOfentries; j++){
				if(j==0){printf("\nBucket [%d] :",i );}
				print_wallet(curr->wallets[j]);
				print_transactList(curr->tr_list[j]);
			}

			curr=curr->next;
		}
			
		
	}
	printf("\n");
}


void print_wallet(wallet* wallt){
	
	bitcoin_list* curr;
	if(wallt==NULL)return;
	printf("\t~ Id %s, total: %d ",wallt->walletId, wallt->total );
		printf(", %d bitcoins\n",wallt->numOfbitCoins);
		
		curr=wallt->bitCs;
		printf("\t");
		if(curr==NULL){printf(" 0 bitcoins\n"); return;}
		while(curr!=NULL){
			printf("-> %d:%d ",curr->bitc->id, curr->value );
			 curr=curr->next;
		}
		printf("\n");
}


void print_wallets(wallet* wallets, int numOfwallts){
	int i ;
	bitcoin_list* curr;
	for(i=0; i<numOfwallts; i++){

		printf("~ Id %s ",wallets[i].walletId );
		printf(", %d bitcoins\n",wallets[i].numOfbitCoins);
		
		curr=wallets[i].bitCs;
		printf("\t");
		while(curr!=NULL){
			printf("-> %d:%d ",curr->bitc->id, curr->value );
			 curr=curr->next;
		}
		printf("\n");
	}
}



void print_transactList(transact_list* list){
	transact_list* curr=list;
	if(list==NULL){printf("\t\tNo transactions\n");return;}
	if(list->tr==NULL){printf("\t\tNo transactions\n"); return;}
	while(curr!=NULL){
		printf("\t\t%s\n", curr->tr->descript);
		curr=curr->next;
	}
}

//______________________________________FREE____________________________________________________




void free_bitcHash(bitC_hashT* ht){
	int i,size;
	bitC_buck* curr, *temp;
	if(ht!=NULL){
		size=ht->size;
		for(i=0; i<size; i++){
			curr=ht->buckets[i];
			while(curr!=NULL){
				temp=curr;
				curr=curr->next;
				free_bitCBucket(temp, ht->buck_s);

			}
			
		}
	}
	free(ht->buckets);
	free(ht);

}

void free_tree(tree_node* node){
	if (node == NULL) return;
    free_tree(node->left);
    free_tree(node->right);
    free(node);
    node=NULL;
 
}

void free_bitCBucket(bitC_buck* buck, int buck_s){
	int i;
	if(buck!=NULL){
		for(i=0; i<buck->numOfbitCoins; i++){
			if(buck->bitCoins[i]!=NULL){
				free_tree(buck->bitCoins[i]->tree);
				free(buck->bitCoins[i]);
			}
		}
		if(buck->bitCoins!=NULL)free(buck->bitCoins);
		free(buck);
	}
	
}

//apodesmeuei kai ta bitcoins
void free_failedwallet(bitcoin_list* bitCs){
	bitcoin_list* curr=bitCs, *temp;
	while(curr!=NULL){
		
		if(curr->bitc!=NULL)free(curr->bitc);		
		temp=curr;
		curr=curr->next;
		free(temp);
		temp=NULL;
		
	}
}

void free_bitcoinList(bitcoin_list* bitCs){
	bitcoin_list* curr=bitCs, *temp;
	while(curr!=NULL){	
		temp=curr;
		curr=curr->next;
		free(temp);
		temp=NULL;
		
	}
}



void free_wallets(wallet* wallets, int numOfusers){
	int i;
	if(wallets!=NULL){
		for(i=0; i<numOfusers; i++){
			if(wallets[i].bitCs!=NULL){
				free_bitcoinList(wallets[i].bitCs);
				wallets[i].bitCs=NULL;
			}			
		
		}
		free(wallets);
	}
	
}

void free_transactList(transact_list* list){
	transact_list* curr=list, *temp;
	while(curr!=NULL){
		temp=curr;
		curr=curr->next;
		free(temp);
	}
}

void free_srBucket(sr_buck* buck, int buck_s){
	int i ;
	if(buck!=NULL){
		for(i=0; i<buck->numOfentries; i++){
			if(buck->tr_list!=NULL){free_transactList(buck->tr_list[i]); }
		}
		free(buck->wallets);
		free(buck->tr_list);
		free(buck);
	}
	
}

void free_srHash(sr_hashT* ht){
	int i,size;
	sr_buck* curr, *temp;
	if(ht!=NULL){
		size=ht->size;
		for(i=0; i<size; i++){
			curr=ht->buckets[i];
			while(curr!=NULL){
				temp=curr;
				curr=curr->next;
				free_srBucket(temp, ht->buck_s);

			}
			
		}
	}
	free(ht->buckets);
	free(ht);
}


