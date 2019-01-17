#NAME: Robert Geil
#EMAIL: rgeil@ucla.edu
#ID: 104916969

CC=gcc
FLAGS=-Wall -Wextra
pname=simplesh
all: lab1.c
	$(CC) $(FLAGS) -o $(pname) lab1.c
dist: lab1.c README Makefile
	tar -zcf lab1-104916969.tar.gz lab1.c README Makefile
clean:
	$(RM) $(pname)