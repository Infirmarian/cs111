#NAME: Robert Geil
#EMAIL: rgeil@ucla.edu
#ID: 104916969
CC=gcc
exename=lab0
flags=-Wall -Wextra

all: lab0.c
	$(CC) $(flags) -o $(exename) lab0.c
check: lab0.c $(exename)
	$(CC) $(flags) -o $(exename) lab0.c
	./smoke.sh
dist: lab0.c Makefile README smoke.sh
	tar -zcf lab0-104916969.tar.gz lab0.c Makefile README smoke.sh
clean: 
	$(RM) $(exename)
	$(RM) lab0-104916969.tar.gz
debug: lab0.c
	$(CC) $(flags) -g -o $(exename) lab0.c
