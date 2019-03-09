#NAME: Robert Geil
#EMAIL: rgeil@ucla.edu
#ID: 104916969

exe=lab4b
dist_name=lab4b-104916969.tar.gz
CC=gcc
FLAGS=-Wall -Wextra


all: lab4b.c
	$(CC) $(FLAGS) lab4b.c -o $(exe)

dist: lab4b.c Makefile
	tar -czf $(dist_name) lab4b.c Makefile

clean:
	$(RM) $(exe)
	$(RM) $(dist_name)