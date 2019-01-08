#NAME: Robert Geil
#EMAIL: rgeil@ucla.edu
#ID: 104916969
CC=gcc
exename=lab0
flags=-Wall -Wextra
all: shell.c
	$(CC) $(flags) -o $(exename) shell.c
dist: shell.c Makefile
	tar 
clean: 
	$(RM) $(exename)
