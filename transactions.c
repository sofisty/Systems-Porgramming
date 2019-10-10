#include "transactions.h"
#define TR_BUCK 64
#define TR_HASHS 255

//diavazei to arxeio transactions apothhkeuei thn kathe sunallagh stis katallhles domes kai thn ektelei
//epistrefei ena hash table me transactions
tr_hashT* load_transactions(char* transactions, sr_hashT* sender, sr_hashT* receiver, struct tm* last_date, int* maxId){
	FILE *t_fp;
	char *line = NULL;
    size_t len = 0;
    ssize_t nread;
    int numOftransact, tr_hashSize;
    tr_hashT* tr_ht=NULL;
    int i, r;
  
	t_fp = fopen (transactions,"r");
	if (t_fp == NULL) {fprintf(stderr, "Cannot open transactions file\n" ); exit(0);}

	numOftransact=0;
    while ((nread = getline(&line, &len,t_fp)) != -1) {numOftransact+=1;}
    rewind(t_fp);

    tr_hashSize=next_prime(TR_HASHS); //defined

    tr_ht=malloc(tr_hashSize* sizeof(tr_hashT));
    if(tr_ht==NULL){fprintf(stderr, "Malloc failed :Transaction's hash table\n" ); exit(0);}
    tr_ht->size=tr_hashSize;
    tr_ht->buck_s=TR_BUCK;

    tr_ht->buckets=malloc(tr_hashSize* sizeof(tr_buck*));
    for(i=0; i<tr_hashSize; i++){
    	tr_ht->buckets[i]=malloc( sizeof(tr_buck));
    	tr_ht->buckets[i]->numOftransact=0;
    	tr_ht->buckets[i]->trctions=NULL;
    	tr_ht->buckets[i]->next=NULL;

    }

    i=0;
    while ((nread = getline(&line, &len,t_fp)) != -1){
    	r=insert_newTransaction( line, sender, receiver, tr_ht, last_date, maxId);
    	if(r==-1){numOftransact-=1;}
    	
    }

    free(line);
    fclose(t_fp);
    return tr_ht;


}


//vriskei kai epistrefei thn lista me eta transactions enos wallet 
transact_list* transactListOfWalletId(char* key, sr_hashT* ht, int* found){
	int size,i, numOfentries,j;
	if(ht==NULL){fprintf(stderr, "Hash Table does not exist\n" ); *found=0;return NULL; }
	size=ht->size;
	i=hash_func(key, size);
	sr_buck* curr = ht->buckets[i];
	if(ht->buckets[i]==NULL){fprintf(stderr, "S/R Bucket %d does not exist\n",i ); *found=0; return NULL;}

	while(curr!=NULL){
		numOfentries=curr->numOfentries;
		for(j=0; j<numOfentries; j++){
			if(strcmp (curr->wallets[j]->walletId, key)==0) {
			 
			 *found=1;
			  return curr->tr_list[j];
			}
		}
		curr=curr->next;
	}
	*found=0;
	return NULL;
}

//vriskei to to transaction id sto hash table me ta transactions
int find_transactId(tr_hashT *tr_ht, char* id){
	int i, size,j;
	tr_buck* curr=NULL;

	if(tr_ht==NULL){fprintf(stderr, "Transaction's hash table does not exist\n" ); exit(0);}
	size=tr_ht->size;
	i=hash_func( id, size);
	
	curr=tr_ht->buckets[i];
	if(tr_ht->buckets[i]->numOftransact==0)return 0;	
	while(curr!=NULL){
		for(j=0; j<curr->numOftransact; j++){

			if( strcmp(curr->trctions[j].id ,id)==0 )return 1;
		}
		curr=curr->next;
	}
		
	return 0;
}

//elegxei an uparxoun arketa bitcoins gia sunallagh amount posou
int valid_amount(wallet* w, int amount){
	if(w->total>=amount)return 1 ;
	else return 0 ;
}

//diavazei ena transaction kai to ektelei, enhmerwnontas tis katallhles domes
int insert_newTransaction(char* entry, sr_hashT* sender_ht, sr_hashT* receiver_ht, tr_hashT* tr_ht, struct tm* last_date, int* maxId ){
	struct tm tm;
	char* token;
	char* id;
	char descript[128];
	wallet* sender, *receiver;
	int amount;
	transaction* new_tr;

	if(entry==NULL){fprintf(stderr, "Something went worng, transaction line is NULL\n" ); exit(0);}
	
	memset(descript, '\0', 128*sizeof(char));
	strcpy(descript, entry);
	int last=strlen(entry)-1;
	if(descript[last]=='\n'){descript[last]='\0';}
	//________________________ID_______________________________________
	
	token=strtok(entry, " \t\r\n");
	if(token==NULL){fprintf(stderr, "Not apropirate format of entry(id etc missing)\n" );  return -1 ;}
	id=malloc( (strlen(token)+1)*sizeof(char) );
	memset(id, '\0', (strlen(token)+1)*sizeof(char));
	strcpy(id, token);
	if(*maxId==-1){*maxId=strtol(id,NULL,10);}
	else{
		if( (strtol(id,NULL,10)>*maxId ))*maxId=strtol(id,NULL,10);
	}
	
	//elegxei an uparxei to id hdh
	if( find_transactId(tr_ht, id)){		
		fprintf(stderr, "Warning! Transaction %s was rejected. Transaction Id already exists. \n",id );
		free(id);
		return -1;
	}
	//_______________________SENDER___________________________________________
	token=strtok(NULL, " \t\r\n");
	if(token==NULL){fprintf(stderr,"Not apropirate format of entry (sender missing)\n"); free(id); return -1; }
	
	//elegxei thn uparksh tou sender
	sender=find_walletId(token, sender_ht);
	if( sender==NULL ){
		
		fprintf(stderr,"Transaction %s was rejected. Sender does not exist\n", id);
		free(id);
		return -1;
	}

	//______________________RECEIVER_______________________________________________
	token=strtok(NULL, " \t\r\n");
	if(token==NULL){ fprintf(stderr,"Not apropirate format of entry (receiver missing)\n"); free(id); return -1; }
	if(strcmp(token,sender->walletId)==0){fprintf(stderr, "Transaction %s was rejected. Sender = Receiver\n",id ); free(id); return-1;}

	receiver=find_walletId(token, receiver_ht);
	if( receiver==NULL ){
		 
		fprintf(stderr,"Transaction %s was rejected. Receiver does not exist\n",id);
		free(id);
		return -1;
	}
	

	//______________________AMOUNT__________________________________________________
	token=strtok(NULL, " \t\r\n");
	if(token==NULL){fprintf(stderr,"Not apropirate format of entry (amount missing)\n"); free(id); return -1; }
	amount=strtol(token, NULL,10);

	if( !valid_amount(sender,amount)){fprintf(stderr,"Transaction %s was rejected. Not enough bitcoins for %d\n",id, amount);free(id); return -1; }

	
	//_____________________DATE_______________________________________________________
	token=strtok(NULL, "\t\r\n");
	if(token==NULL){ fprintf(stderr,"Not apropirate format of entry (date missing)\n"); free(id); return -1; }

	//apothhkeuei se mia struct tm thn hmeromhnia
	memset(&tm, 0, sizeof(struct tm));
	if(strptime(token, "%d-%m-%Y %H:%M", &tm)==NULL){
		
		fprintf(stderr, "Transaction %s was rejected. Wrong date format\n",id );
		free(id);
		return -1;
	}

	*last_date=tm;
	tr_ht= addToHash_tr( descript,id,amount,tm,tr_ht, &new_tr); //prosthetei sto hash table to transaction
	
	sender_ht= update_transactList(sender_ht, new_tr, sender->walletId); //apothhkeuei ton pointer tou transaction ston komvo tou sender
	receiver_ht=update_transactList(receiver_ht, new_tr, receiver->walletId); //antistoixa ston komvo tou receiver
	//if(strcmp(sender->walletId,"DawnMcgee")==0){printf("Done\n");}
	//if(strcmp(receiver->walletId,"DawnMcgee")==0){printf("Done\n");}
	sender=exec_transaction( sender , &receiver, new_tr); //ektelei to transaction enhmerwnontas tis domes
	//printf("success\n");
	return 0;

}


//dhmiourgei ena transaction kai to apothhkeuei sto hash table
tr_hashT* addToHash_tr(char descript[128] ,char* id, int amount, struct tm date, tr_hashT* tr_ht, transaction** new_tr){
	int i,j;
	tr_buck* curr;

	if(tr_ht==NULL){fprintf(stderr, "Something went wrong, tr_ht = NULL\n" ); exit(0);}
	i=hash_func(id, tr_ht->size);
	
	if(tr_ht->buckets==NULL){fprintf(stderr, "Something went wrong, tr_ht->buckets = NULL\n" ); exit(0);}
	if(tr_ht->buckets[i]==NULL){fprintf(stderr, "Malloc bucket i, in Transaction's hash table \n"); exit(0);}
	if(tr_ht->buckets[i]->trctions==NULL){	
		
		tr_ht->buckets[i]->trctions=malloc(tr_ht->buck_s* sizeof(transaction));
		if(tr_ht->buckets[i]->trctions==NULL){fprintf(stderr, "Malloc failed. Hash transactions\n" ); exit(0);}
		strcpy(tr_ht->buckets[i]->trctions[0].descript, descript);
		tr_ht->buckets[i]->trctions[0].id=id;
		tr_ht->buckets[i]->trctions[0].amount=amount;
		tr_ht->buckets[i]->trctions[0].date=date;
		tr_ht->buckets[i]->numOftransact=1;
		*new_tr= &(tr_ht->buckets[i]->trctions[0]);
		
		return  tr_ht;
	}
	else{
		curr=tr_ht->buckets[i];
		while(curr->next!=NULL){curr=curr->next;}

		if(curr->numOftransact < tr_ht->buck_s){
			j=curr->numOftransact;
			strcpy(curr->trctions[j].descript, descript);
			curr->trctions[j].id=id;
			curr->trctions[j].amount=amount;
			curr->trctions[j].date=date;
			*new_tr= &(curr->trctions[j]);
			(curr->numOftransact)+=1;
		}
		else{

			curr->next=malloc(sizeof(tr_buck));
			if(curr->next==NULL){fprintf(stderr, "Malloc tr_buck failed\n" ); exit(0);}
			curr->next->trctions=malloc(tr_ht->buck_s * sizeof(transaction));
			if(curr->next->trctions==NULL){fprintf(stderr, "Malloc failed. Hash transactions\n" ); exit(0);}
			
			strcpy(curr->next->trctions[0].descript, descript);
			curr->next->trctions[0].id=id;		
			curr->next->trctions[0].amount=amount;
			curr->next->trctions[0].date=date;
			
			*new_tr= &(curr->next->trctions[0]);
			
			curr->next->next=NULL;
			curr->next->numOftransact=1;

		}
	}
	
	return tr_ht;

}

//prosthetei enan transaction* sthn lista me ta transactions enos sender h enos receiver
transact_list* add_trasnact(transact_list* list, transaction* tr){
	transact_list* curr=NULL;
	if(list==NULL){
		list=malloc(sizeof(transact_list));
		if(list==NULL){fprintf(stderr, "Malloc transact_list failed\n"); exit(0);}
		list->tr=tr;
		list->next=NULL;
	}
	else{
		curr=list;
		while(curr->next!=NULL){
			curr=curr->next;
		}
		curr->next=malloc(sizeof(transact_list));
		if(curr->next==NULL){fprintf(stderr, "Malloc transact_list failed\n"); exit(0);}
		curr->next->tr=tr;
		curr->next->next=NULL;

	}
	return list;
}

//vriskei ton komvo tou sender h tou receiver antistoixa kai enhmerwnei thn lista transactions
sr_hashT* update_transactList(sr_hashT* sr_ht, transaction* new_tr, char* walletId){
	int i, size,j;
	sr_buck* curr=NULL;

	if(sr_ht==NULL){fprintf(stderr, "SR hash table does not exist\n" ); exit(0);}
	size=sr_ht->size;
	i=hash_func(walletId, size);
		
	curr=sr_ht->buckets[i];
	if(sr_ht->buckets[i]->numOfentries==0){
		fprintf(stderr, "SR-HASH-T WalletID does not exist\n" );
		exit(0);
	}	
	while(curr!=NULL){
		for(j=0; j<curr->numOfentries; j++){

			if( strcmp(curr->wallets[j]->walletId ,walletId)==0){
				curr->tr_list[j]= add_trasnact(curr->tr_list[j],  new_tr);
			}
		}
		curr=curr->next;
	}

	return sr_ht;
}

//_________________________________TREE__________________________________________________

//antallagh bitcoin 
bitcoin_list* swap_listNode(bitcoin_list* list, bitcoin_list* node ,int val,  bitcoin_list** new_curr){
	bitcoin_list* curr=list;
	if(list==NULL){ //an den exei lista o receiver o komvos tou sender ginetai o prwtos komvos tou receiver	
		list=node;		
		list->value=val;
		list->next=NULL;
		return list;
	}

	if(curr->bitc->id==node->bitc->id){ //an moirazontai to idio bitcoin paei eksoloklhrou ston receiver
		curr->value+=val;
		*new_curr=curr;
		return list;
	}
	while(curr->next!=NULL){
		if(curr->next->bitc->id==node->bitc->id){//an moirazontai to idio bitcoin paei eksoloklhrou ston receiver
			curr->next->value+=val;
			*new_curr=curr; 
			return list;

		}
		curr=curr->next;
	}
	curr->next=node; //alliws anatithetai o komvos sto telos ths lista tou receiver
	curr->next->value=val;
	curr->next->next=NULL;
	return list;
}

//metaferetai ena poso apo to bitcoin tou receiver
bitcoin_list* receive_bitc(bitcoin_list* list, bitcoin* bitc, int val){
	bitcoin_list* curr=list;
	//dhmiourgeitai o katallhlos komvos bitcoin_list sto telos ths lista tou receiver kai
	//sto bitcoin apothhkeuetai o bitc* kai to poso (val) pou elave apo thn sunallagh
	if(list==NULL){
		
		list=malloc(sizeof(bitcoin_list));
		if(list==NULL){fprintf(stderr, "Malloc bitcoin list failed\n" ); exit(0);}
		list->bitc=bitc;
		list->value=val;
		list->next=NULL;
		return list;
	}

	if(curr->bitc->id==bitc->id){ 
		curr->value+=val; //an to exei hdh
		return list;
	}
	while(curr->next!=NULL){
		if(curr->next->bitc->id==bitc->id){ //an to exei hdh
			curr->next->value+=val;
			return list;
		}
		curr=curr->next;
	}
	curr->next=malloc(sizeof(bitcoin_list));
	if(curr->next==NULL){fprintf(stderr, "Malloc bitcoin list failed\n" ); exit(0);}
	curr->next->bitc=bitc;
	curr->next->value=val;
	curr->next->next=NULL;
	return list;
}

//anadromika prostithetai o komvos sto fullo pou exei wallet idio me tou receiver
void update_tree( tree_node** root,tree_node *node, int send_val, int rec_val, int init_val, wallet* sender, wallet* receiver, transaction* tr, int* done) { 
	if(*done==1) return;

	if( (*root)==NULL){
		(*root)=insert_treeNode( (*root), init_val,  tr, sender);
		(*root)->left=insert_treeNode( (*root)->left, rec_val, NULL, receiver);
		(*root)->right=insert_treeNode( (*root)->right, send_val, NULL,sender);
		
		return ;
	}
   
    if(node->left==NULL && node->right==NULL) { 

       if( strcmp(node->w->walletId, sender->walletId)==0){
			node->left=insert_treeNode( node->left, rec_val, NULL, receiver);
			node->right=insert_treeNode( node->right, send_val, NULL,sender);
			node->tr=tr;
			*done=1; //mhn kanei pollaples eisagwges
			

			
		}
		return ;
		
		
    } 
  	//prwta apo ta deksia gia ton sender 
  	if(node->right!=NULL) update_tree(root, node->right,  send_val,  rec_val, init_val, sender,  receiver,  tr, done) ;
	if(node->left!=NULL) update_tree(root, node->left, send_val,  rec_val, init_val, sender,  receiver,  tr, done) ;
	      	 
	
          
}  

//desmeuei ena tree node kai apothhkeuei ta dedomena tou
tree_node* insert_treeNode(tree_node* tree, int amount, transaction* tr, wallet* w){

		tree=malloc(sizeof(tree_node));
		tree->amount=amount;
		tree->tr=tr;
		tree->w=w;
		tree->left=NULL;
		tree->right=NULL;
		tree->visited=0;
		
		return tree;
	
}

//ektelei to transaction
wallet* exec_transaction( wallet* sender , wallet** receiver, transaction* tr ){
	int amount=tr->amount, val, send_val, rec_val, init_val, done;
	bitcoin_list* curr=sender->bitCs, *next=NULL, *new_curr=NULL;
	bitcoin* temp;
	//oso to poso einai >0 kai uparxei diathesimo bitcoin
	while(amount>0 && curr!=NULL){
		init_val=curr->value;
		if( (curr->value-amount)<=0 ){	//an to poso einai perissotero apo osa exei to current bitcoin
			amount-=(curr->value);//to poso pou tha meinei na eksoflisoun ta epomena bitcoins
			next=curr->next;
			sender->bitCs=sender->bitCs->next; //feugei to bitcoin apo thn lista tou sender (panta to prwto)
			val=curr->value; //to poso pou tha dothei ston receiver einai oso eixe mesa to bitcoin
			
			(*receiver)->bitCs= swap_listNode((*receiver)->bitCs, curr , val,  &new_curr ); //pairnei to bitcoin node o receiver
			(*receiver)->numOfbitCoins+=1;
			if(new_curr!=NULL){free(curr); curr=new_curr;} //an to eixe hdh ginetai free, kai to curr enhmerwnetai wste na aukshthei
														 //o arithmos twn transactions tou bitcoin 			
			sender->numOfbitCoins-=1; 
			send_val=0;
			rec_val=val;
			
		}
		else{//alliws apla dhmiourgeitai sthn lista tou receiver komvos me to kainourgio bitcoin 
			//h enhmerwnetai to value an uparxei hdh
		
			curr->value-=amount; //meiwnetai h aksia tou bitcoin tou sender kata amount
			val=amount;
			temp=curr->bitc;
			(*receiver)->bitCs= receive_bitc((*receiver)->bitCs, temp, val);
			(*receiver)->numOfbitCoins+=1;
			amount=0; 
			send_val=curr->value;
			rec_val=val;
			next=curr->next;

		}

		curr->bitc->numOftransacts+=1;
		done=0;
		update_tree( &(curr->bitc->tree) ,curr->bitc->tree, send_val, rec_val, init_val,  sender, *receiver,  tr, &done); //enhmerwnetai to dentro

		curr=next;
	}
	sender->total-= (tr->amount); 
	(*receiver)->total+= (tr->amount) ;

	return sender;
}

void print_trHash(tr_hashT* tr_ht){
	int i, j, size;
	if(tr_ht==NULL){printf("Bitcoin hash table does not exist\n"); return;}
	size=tr_ht->size;
	tr_buck* curr;
	printf("--- Transactions Hash Table ---\n");
	for(i=0; i<size; i++){
		curr=tr_ht->buckets[i];
		while(curr!=NULL){
			//printf("Bucket [%d] :",i );
			for(j=0; j<curr->numOftransact; j++){
				if(j==0){printf("\nBucket [%d] :",i );}
				printf("> %s\n ",curr->trctions[j].descript );
			
			}

			curr=curr->next;
		}
			
		
	}
	printf("\n");
}


//___________________________FREE____________________________________________________

void free_trHash(tr_hashT* ht){
	int i;
	tr_buck* curr=NULL, *temp;
	if(ht!=NULL){
		for(i=0; i<ht->size; i++){
			curr=ht->buckets[i];
			while(curr!=NULL){
				temp=curr;
				curr=curr->next;
				free_trBucket(temp);
			}
		}
		free(ht->buckets);
		free(ht);
	}
}

void free_trBucket(tr_buck* buck){
	int i;
	if(buck!=NULL){
		for(i=0; i<buck->numOftransact; i++){
			free(buck->trctions[i].id);
		}
		free(buck->trctions);
		free(buck);
		  buck=NULL;
	}
}