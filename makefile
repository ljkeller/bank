appserver : main.o Bank.o
	cc -o appserver main.o Bank.o

main.o : main.c 
	cc -c main.c
Bank.o : Bank.c Bank.h 
	cc -c Bank.c

clean: 
	rm *output.txt appserver main.o Bank.o
