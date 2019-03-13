# the compiler: gcc for C program, define as g++ for C++
CC = gcc

# compiler flags:
#  -g    adds debugging information to the executable file
#  -Wall turns on most, but not all, compiler warnings
CFLAGS  = -g -Wall

all: ece361

test: test.o
	gcc test.o -o tester

ece361: client.o
	gcc client.o -o ece361

client.o: client.c 
	gcc -c client.c 

test.o: test.c 
	gcc -c test.c 

clean:
	rm -rf *o ece361