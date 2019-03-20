#NAME: Robert Geil
#EMAIL: rgeil@ucla.edu
#ID: 104916969

FLAGS=-Wall -Wextra -lmraa -lm
CC=gcc
tarname=lab4c-104916969.tar.gz
exe1=lab4c_tcp
exe2=lab4c_tls


all: lab4c.c
	$(CC) $(FLAGS) lab4c.c -o $(exe1)
	$(CC) $(FLAGS) -lssl -lcrypto lab4c.c -o $(exe2)

dist: lab4c.c Makefile
	tar -czf $(tarname) lab4c.c Makefile

clean:
	$(RM) $(exe1)
	$(RM) $(exe2)
	$(RM) $(tarname)