appserver : main.o Bank.o server_helper.o queue.o 
	cc -o appserver main.o Bank.o server_helper.o queue.o -lpthread

main.o : main.c Bank.h server_helper.h queue.h
	cc -c main.c 
Bank.o : Bank.c Bank.h 
	cc -c Bank.c
server_helper.o : server_helper.c server_helper.h queue.h
	cc -c server_helper.c
queue.o : queue.h
	cc -c queue.c
clean: 
	rm *output.txt appserver main.o Bank.o server_helper.o queue.o
