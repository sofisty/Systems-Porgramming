CC = gcc
CFLAGS  = -Wall -g -O3
LIBS += -lm 

bitcoin:  main.o balances.o transactions.o commands.o
	$(CC) $(CFLAGS) -o bitcoin main.o balances.o transactions.o commands.o $(LIBS)

balances.o: balances.c balances.h
	$(CC) $(CFLAGS) -c balances.c

transactions.o: transactions.c transactions.h
	$(CC) $(CFLAGS) -c transactions.c

commands.o: commands.c commands.h
	$(CC) $(CFLAGS) -c commands.c

clean: 
	-rm -f *.o 
	-rm -f rhj