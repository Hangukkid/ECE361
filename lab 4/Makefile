# the compiler: gcc for C program, define as g++ for C++
CC = gcc

# compiler flags:
#  -g    adds debugging information to the executable file
#  -Wall turns on most, but not all, compiler warnings
CFLAGS  = -g -Wall

all: chatclient chatserver 

chatserver: server.o
	gcc -pthread server.o -o chatserver

chatclient: client.o
	gcc -pthread client.o -o chatclient

client.o: client.c 
	gcc -c client.c 

server.o: server.c
	gcc -c server.c

clean:
	rm -rf *o ece361