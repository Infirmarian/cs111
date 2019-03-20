#NAME: Robert Geil
#EMAIL: rgeil@ucla.edu
#ID: 104916969

FLAGS=-Wall -Wextra -lmraa -lssl -lcrypto -lm
CC=gcc
tarname=lab4c-104916969.tar.gz
exe1=lab4c_tcp
exe2=lab4c_tls


all: lab4c.c
	$(CC) $(FLAGS) -D TCP lab4c.c -o $(exe1)
	$(CC) $(FLAGS) -D TLS lab4c.c -o $(exe2)

dist: lab4c.c Makefile README
	tar -czf $(tarname) lab4c.c Makefile README

clean:
	$(RM) $(exe1)
	$(RM) $(exe2)
	$(RM) $(tarname)