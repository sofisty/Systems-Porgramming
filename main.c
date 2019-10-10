#include "balances.h"
#include "transactions.h"
#include "commands.h"


int main ( int argc, char **argv ) {
	int bit_val, h1, h2, buck_s;
	char *balances, *transactions;
	wallet* wallets_arr=NULL;
	bitC_hashT* bitC_ht=NULL;
	sr_hashT* sender_ht=NULL, *receiver_ht=NULL;
	tr_hashT* tr_ht=NULL;
	int numOfwallts;
	struct tm last_date;
	int maxId=-1, i;
	
	
	if(store_arguments(argc,  argv, &balances, &transactions, &bit_val, &h1, &h2, &buck_s)) return -1;
	//printf("bit_val %d , h1 %d h2 %d buck_s %d, bal file %s, trans file %s \n",bit_val,h1,h2,buck_s,balances,transactions );
	
	numOfwallts=findnumOfwallets( balances);
	buck_s= (buck_s -sizeof(int)-sizeof(sr_buck*)) / (sizeof(wallet*)+sizeof(transact_list*)); //upologizei poses eggrafes xwrane sto bucket
	
	sender_ht =create_hashT( numOfwallts, h1, buck_s); //arxikopoiei ta hash_tables sender, receiver 
	receiver_ht =create_hashT( numOfwallts, h2, buck_s);
	
	bitC_ht=load_balances( &wallets_arr, balances, numOfwallts ,bit_val, &sender_ht, &receiver_ht );//fortwnei kai apothhkeuei stis domes ta balances
	if(bitC_ht==NULL){ //an dothei arxeio me lathos termatizei h efarmogh				
		free_srHash(sender_ht);
		free_srHash(receiver_ht);
		free_wallets(wallets_arr, numOfwallts);
		free(balances);
		free(transactions);
		return -1;
	}
	
   	tr_ht= load_transactions( transactions, sender_ht, receiver_ht, &last_date,&maxId ); //fortwnei to arxeio transactions kai ektelei tis sunallages
  
   	char  temp[255];
	char* line = NULL, temp_line[255];
	size_t len=0;
	int flag=0; //anagwrizei h requestTransaction an h eisodos prokeitai gia grammh arxeiou h proerxetai apo entolh
	int start=0;
	
	printf("Enter command\n");
	while(1){
		memset(temp, '\0', 255*sizeof(char));
		memset(temp_line, '\0', 255*sizeof(char));
		if ( (getline(&line, &len, stdin)) == -1){fprintf(stderr, "scanf failed\n" ); free(line);return 1;}
		else{

			i=0;	
			if(line!=NULL)while( line[i]!=' ' && line[i]!='\n' && line[i]!='\0'){temp[i]=line[i];  i++;} //onoma entolhs
			
			if(line!=NULL){ //eksetazei an h entolh exei ; sto telos
				i=0;
				while( line[i]!='\n' && line[i]!='\0'){
					if(line[i]==';')flag=1;
					else flag=0;
					i++;
				}
			}
			
			if(strcmp(temp,"requestTransaction")==0){
				requestTransaction(line, sender_ht, receiver_ht,  tr_ht,  &last_date, &maxId, flag );
				start=0;
			}
			else if(flag==1 && start==1){
				requestTransactions(line, sender_ht, receiver_ht, tr_ht, &last_date, &maxId, flag) ;
			}					
			else if(strcmp(temp,"requestTransactions")==0){
				start=1; //molis to start ginei 1, anagnwrizei grammes me ; sto telos, ws eisodo tou requestTransactions	
						//otan mpei se epomenh entolh apenergopoieitai se 0	
				requestTransactions(line, sender_ht, receiver_ht, tr_ht, &last_date, &maxId, flag) ;
			
			}		 		 	
			else if(strcmp(temp,"findEarnings")==0){
				findEarn_Paym(line, receiver_ht, 0, last_date);
				start=0;
			}
			else if(strcmp(temp,"findPayments")==0){
				findEarn_Paym(line, sender_ht, 1, last_date);
				start=0;
			}
			else if(strcmp(temp,"walletStatus")==0){
				walletStatus(line, sender_ht);
				start=0;
			}
			else if(strcmp(temp,"bitcoinStatus")==0){	
				bitcoinStatus(line, bitC_ht, bit_val);
				start=0;
			}
			else if(strcmp(temp,"traceCoin")==0){			
				traceCoin(line, bitC_ht);
				start=0;
			}
			else if(strcmp(temp,"exit")==0){
				break;
				
			}
			else{
				fprintf(stderr, "Wrong command. Please try again\n" );
				start=0;
			}
		}
	}

	
	free_bitcHash(bitC_ht);
	free_srHash(sender_ht);
	free_srHash(receiver_ht);
	free_trHash(tr_ht);
	free_wallets(wallets_arr, numOfwallts);
	free(balances);
	free(transactions);
	free(line);


	return 0;
}