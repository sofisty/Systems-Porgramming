#include "commands.h"

int requestTransaction(char* entry, sr_hashT* sender_ht, sr_hashT* receiver_ht, tr_hashT* tr_ht, struct tm* last_date, int* maxId, int flag ){
	struct tm tm;
	char* token, temp_date[20];
	char* id=NULL;
	char descript[128];
	wallet* sender, *receiver;
	int amount;
	transaction* new_tr;
	time_t now = time(0);

	if(entry==NULL){fprintf(stderr, "Something went worng, transaction line is NULL\n" ); exit(0);}
  	if(flag==0)entry = strchr(entry, ' ');
	//___________________________ID_________________________________
	//dhmiourgei to kainourgio id pairnontas to max mexri twra kai auksanontas to kata 1	
	*maxId+=1;
	id=malloc(10*sizeof(char));
	memset(id, '\0', 10*sizeof(char));
	sprintf(id, "%d",*maxId); 	
	
	if( find_transactId(tr_ht, id)){
		fprintf(stderr, "Warning! Transaction %s was rejected. Transaction Id already exists. \n",id );
		free(id);
		return -1;
	}
	//_______________________SENDER___________________________________________
	token=strtok(entry, " ;\t\r\n");
	if(token==NULL){fprintf(stderr,"Not apropirate format of entry\n"); free(id);return -1; }
	
	sender=find_walletId(token, sender_ht);
	if( sender==NULL ){		
		fprintf(stderr,"Transaction %s was rejected. Sender does not exist\n", id);
		free(id);
		return -1;
	}
	
	//______________________RECEIVER_______________________________________________
	token=strtok(NULL, " ;\t\r\n");
	
	if(token==NULL){ fprintf(stderr,"Not apropirate format of entry\n"); free(id); return -1; }
	if(strcmp(token,sender->walletId)==0){fprintf(stderr, "Transaction %s was rejected. Sender = Receiver\n",id ); free(id); return-1;}

	receiver=find_walletId(token, receiver_ht);
	if( receiver==NULL ){
		 
		fprintf(stderr,"Transaction %s was rejected. Receiver does not exist\n",id);
		free(id);
		return -1;
	}

	//______________________AMOUNT__________________________________________________
	token=strtok(NULL, " ;\t\r\n");
	if(token==NULL){fprintf(stderr,"Not apropirate format of entry\n"); free(id); return -1; }
	amount=strtol(token, NULL,10);

	//eksetazei an to poso pou exei sunolika sto wallet tou o sender einai arketo gia thn sunallagh
	if( !valid_amount(sender,amount)){fprintf(stderr,"Transaction %s was rejected. Not enough bitcoins for %d\n",id, amount);free(id); return -1; }

	
	//_____________________DATE_______________________________________________________
	
	token=strtok(NULL, ";\t\r\n");
	
	//an den exei dothei wra apothhkeuetai h trexousa
	memset(temp_date,'0',20*sizeof(char));
	if(token==NULL){ 
    	tm = *localtime(&now);
    	strftime(temp_date,20, "%d-%m-%Y %H:%M:%S", &tm);
    	printf("%s \n",temp_date );
	}
	else{
		memset(&tm, 0, sizeof(struct tm));
		if(strptime(token, "%d-%m-%Y %H:%M", &tm)==NULL){			
			fprintf(stderr, "Transaction %s was rejected. Wrong date format\n",id );
			free(id);
			return -1;
		}
		strftime(temp_date,20, "%d-%m-%Y %H:%M", &tm);
	}
	
	time_t new_t = mktime(&tm), last_t=mktime(last_date);
	
	//elegxei an h mheromhnia pou dothhke einai metagenesterh ths teleutaias pou exei katagrafei sunallagh
	if(last_t>=new_t){
		char er[20];
		strftime(er,20,"%d-%m-%Y %H:%M", last_date);
	 	fprintf(stderr, "Transaction %s was rejected. Date should be greater than %s\n", id,er );
		free(id);
		return -1;
	}
	*last_date=tm;	
	
	//_____________________DESCRIPT___________________________________________________	
	//dhmiourgei to string me tis plhrofories ths sunallaghs
	memset(descript, '\0', 128*sizeof(char));
	strcat(descript, id);
	strcat(descript," ");
	strcat(descript,sender->walletId);
	strcat(descript," ");
	strcat(descript,receiver->walletId);
	strcat(descript," ");
	char temp_amount[5];
	sprintf(temp_amount,"%d",amount);
	strcat(descript,temp_amount );
	strcat(descript," ");
	strcat(descript,temp_date);

	//sprintf(descript,"%s %s %s %d %s",id,sender->walletId,receiver->walletId, amount ,temp_date);
	//printf("Descr %s \n",descript );
	
	//_______________________________________________________________________________

	*last_date=tm;
	tr_ht= addToHash_tr( descript,id,amount,tm,tr_ht, &new_tr); //prosthetei thn sunallagh sto hash_table twn transactions
	sender_ht= update_transactList(sender_ht, new_tr, sender->walletId); //enhmerwnei thn lista tou sender kai receiver me to neo transaction
	receiver_ht=update_transactList(receiver_ht, new_tr, receiver->walletId);
	sender=exec_transaction( sender , &receiver, new_tr); //ektelei thn sunallagh

	printf("> Succesful transaction %s! Sender %s: %d. Receiver %s: %d.\n",id,sender->walletId, sender->total, receiver->walletId, receiver->total );
	return 0;

}

int requestTransactions(char* entry, sr_hashT* sender_ht, sr_hashT* receiver_ht, tr_hashT* tr_ht, struct tm* last_date, int* maxId, int flag){
	FILE *t_fp=NULL;
	char *line = NULL;
    size_t len = 0;
    ssize_t nread;
	char* token;
	char  temp[20];
	//flag==0 requestTransactions with file
	//flag==1 transaction input line:(requestTransactions) + richard crystalsmith 20 13-02-2018 12:32;

	if(entry==NULL){fprintf(stderr, "Something went worng, transaction line is NULL. Please try again!\n" ); return -1;}
  	int i=0;
  	memset(temp, '\0', 20*sizeof(char));
   	while( entry[i]!=' ' && entry[i]!='\n' && entry[i]!='\0'){temp[i]=entry[i];  i++;}
  	if(strcmp(temp, "requestTransactions")==0){entry = strchr(entry,' ');} //afairei to "requestTransactions apo thn grammh"
	
	if(flag==0){ //einai arxeio
		token=strtok(entry," \t\n\r");
		t_fp = fopen(token,"r");
		flag=1;
		if (t_fp == NULL) {fprintf(stderr, "Cannot open transactions file\n" ); return -1;}
	    while ((nread = getline(&line, &len,t_fp)) != -1) {

	    	requestTransaction(line, sender_ht, receiver_ht,  tr_ht, last_date, maxId, flag); //kaleitai h requestTransaction gia kathe grammh tou arxeiou
	    }
	    free(line);
	    fclose(t_fp);
	    return 0;
	   
	}
	else{
		
		requestTransaction(entry, sender_ht, receiver_ht,  tr_ht, last_date, maxId, flag); //kaleiti h requestTransaction gia thn dotheisa grammh

	}

	return 0;
}


int totalEarn_Paym(struct tm* start, struct tm* end, transact_list* list, int flag, int func, struct tm last_date){
	transact_list *curr=NULL;
	int total=0, s=0, e=0;
	curr=list;
	time_t t1, t2, curr_t;
	//func=1 findPayments
	//func=0 findEarnings

	//flag=1 plhrhs hmeromhnia
	//flag=0 mono wra 

	if(flag){ //exei dothei plhrhs hmeromhnia h katholou
		if(start==NULL && end==NULL){ //den uparxei xroniko perithwrio
			t1=0; 
			t2=mktime(&last_date);
		}
		else{
			t1=mktime(start); //metatrepetai h meromhnia se time_t gia thn sugkrish
			t2=mktime(end);
		}
		printf("Transactions: \n");
		while(curr!=NULL){
			if(curr->tr!=NULL){
				curr_t=mktime( &(curr->tr->date) );
				if(curr_t>=t1 && curr_t<=t2){
					total+=curr->tr->amount;
					printf("- %s\n",curr->tr->descript );
				}
			}
			curr=curr->next;
		}
		if(func==0)printf("Total earnings: %d\n",total);
		else printf("Total payments: %d\n",total);
	}
	else{ //mono h wra
		printf("Transactions: \n");
		while(curr!=NULL){
			s=0;
			e=0;
			if(curr->tr!=NULL){
				if( start->tm_hour <curr->tr->date.tm_hour ){s=1;} //elegxos gia wra kai lepta
				else if((start->tm_hour==curr->tr->date.tm_hour) && (start->tm_min<= curr->tr->date.tm_min)){s=1;}

				if(end->tm_hour > curr->tr->date.tm_hour){e=1;}
				else if( (end->tm_hour == curr->tr->date.tm_hour) && (end->tm_min >= curr->tr->date.tm_min)){e=1;}

				if( s && e){
					total+=curr->tr->amount;
					printf("- %s\n",curr->tr->descript );
				}
			}
			curr=curr->next;
		}
		if(func==0)printf("Total earnings: %d\n",total);
		else printf("Total payments: %d\n",total);
	}

	return 0;
}


int findEarn_Paym(char* entry, sr_hashT* ht, int flag, struct tm last_date){
	struct tm tm1, tm2;
	char* token;
	transact_list* tr_list;
	int found=0, fl;
	char * mess="findEarnings/Payments. Wrong format [time1][year1][time1][year2]\n";

	//flag=1 findPayments
	//flag=0 findEarnings

	if(entry==NULL){fprintf(stderr, "Something went worng, transaction line is NULL. Please try again!\n" ); return -1;}
  	entry = strchr(entry, ' ');

	token=strtok(entry," \t\n\r");

	if(token==NULL){fprintf(stderr,"Not apropirate format of entry\n"); return -1; }
	
	tr_list=transactListOfWalletId(token, ht, &found);
	if(found==0 ){		
		if(flag==0){fprintf(stderr,"Find earnings failed. Receiver %s does not exist\n", token);}
		else{fprintf(stderr,"Find payments failed. Sender %s does not exist\n", token);}
		return -1;
	}
	else{
		if(tr_list==NULL){ //an den exei sunallages
			if(flag==0)printf("Transactions: \n Total earnings: 0\n");
			else{printf("Transactions: \n Total payments: 0\n");}
			return 0;
		}
		else{
			token=strtok(NULL," \t\n\r");
			if(token==NULL){
				//den uparxei xroniko diasthma 
				totalEarn_Paym(NULL, NULL, tr_list, 1, flag, last_date );
				return 0;
			}
			else{
				int y=0, t=0; //number of given variables time and year
				memset(&tm1, 0, sizeof(struct tm));
				memset(&tm2, 0, sizeof(struct tm));
				while(token!=NULL){
					if(y==0){
						if(strptime(token, "%d-%m-%Y", &tm1)!=NULL){y+=1;}
					}
					else if(y==1){
						if(strptime(token, "%d-%m-%Y", &tm2)!=NULL){y+=1;}
					}
					if(t==0){
						if(strptime(token, "%H:%M", &tm1)!=NULL){t+=1;}				
					}
					else if(t==1){
						if(strptime(token, "%H:%M", &tm2)!=NULL){t+=1;}	
					}
					
					token=strtok(NULL," \t\n\r");

				}
				//an den exoun dothei ta katallhla orismata gia time kai year			
				if( !((y==2 && t==0) || (t==2 && y==0)  || (t==2 && y==2) )){fprintf(stderr, "t:%d y:%d %s",t,y,mess); return -1;}
				
				fl=1; //xronikos periorismos
				if(t==2 && y==0){fl=0;}
				totalEarn_Paym(&tm1, &tm2, tr_list, fl, flag, last_date );
				
				return 0;


			}
		}
	}
	return 0;
}


int walletStatus(char* entry, sr_hashT* ht ){
	char* token;
	wallet* w;
	if(entry==NULL){fprintf(stderr, "Something went worng, transaction line is NULL. Please try again!\n" ); return -1;}
	entry= strchr(entry, ' ');
	token=strtok(entry," \t\n\r");
	if(token==NULL){fprintf(stderr, "WalletStatus failed\n"); return -1;}
	w=find_walletId(token, ht);
	if(w==NULL){ fprintf(stderr, "WalletStatus failed. %s does not exist.\n", token); return -1;}
	printf("Wallet Status of %s : %d\n",token , w->total ); //ektupwnei to sunoliko poso apo to wallet
	return 0;

}


//ektupwnei ton arithmo twn transactions pou exei xrhsimopoihthei to bitcoin 
//kai to poso pou den exei xrhsimopoihthei se kamia sunallagh
int bitcoinStatus(char* entry, bitC_hashT* ht, int bitCval){
	bitcoin* bitC=NULL;
	int id, unspent=bitCval;
	char* token;
	tree_node* curr=NULL;
	
	if(entry==NULL){fprintf(stderr, "Something went worng, transaction line is NULL. Please try again!\n" ); return -1;}
	entry= strchr(entry, ' ');
	token=strtok(entry," \t\n\r");
	if(token==NULL){fprintf(stderr, "BitcoinStatus failed\n"); return -1;}
	id=strtol(token,NULL,10);
	
	bitC=find_bitCoinId(ht, id);
	if(bitC==NULL){fprintf(stderr, "Bitcoin %d does not exist\n",id );}
	else{
		//fullo terma deksia 
		if(bitC->tree!=NULL){
			curr=bitC->tree;
			while(curr->right!=NULL){
				curr=curr->right;
			}
			unspent=curr->amount;
		}
		printf("%d %d %d \n",id,bitC->numOftransacts, unspent );
	}
	
	
	return 0;


}

//ektupwnei ola ta transactions tou dentrou
void TransactsOfbitc(tree_node *root){  
    if (root==NULL){ printf("No transactions\n");return;} 

    if (root->left==NULL && root->right==NULL) {return;} 
  
	//an o komvos exei paidia shmainei oti einai transaction  
    if(root->left!=NULL && root->right!=NULL){
    	if(root->visited==0){
    		printf("%s\n",root->tr->descript );
    	}
    }

    if (root->left!=NULL){ TransactsOfbitc(root->left); }
          
    if (root->right!=NULL){ TransactsOfbitc(root->right);}
}  


int traceCoin(char* entry , bitC_hashT* ht){
	bitcoin* bitC=NULL;
	int id;
	char* token;	

	if(entry==NULL){fprintf(stderr, "Something went worng, transaction line is NULL. Please try again!\n" ); return -1;}
	entry= strchr(entry, ' ');
	token=strtok(entry," \t\n\r");
	if(token==NULL){fprintf(stderr, "traceCoin failed\n"); return -1;}
	id=strtol(token,NULL,10);
	
	bitC=find_bitCoinId(ht, id);
	if(bitC==NULL){fprintf(stderr, "Bitcoin %d does not exist\n",id );return -1;}
	printf("Transactions of %d : \n",id);
	TransactsOfbitc(bitC->tree);

	return 0;
	
}
