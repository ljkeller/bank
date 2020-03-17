appserver : main.o Bank.o server_helper.o
	cc -o appserver main.o Bank.o server_helper.o

main.o : main.c Bank.h server_helper.h
	cc -c main.c
Bank.o : Bank.c Bank.h 
	cc -c Bank.c
server_helper.o : server_helper.c Bank.h server_helper.h
	cc -c server_helper.c
clean: 
	rm *output.txt appserver main.o Bank.o server_helper.o
