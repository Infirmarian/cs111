#NAME: Robert Geil
#EMAIL: rgeil@ucla.edu
#ID: 104916969

exe=lab3a
CC=g++
FLAGS=-Wall -Wextra -g
distName=lab3a-104916969.tar.gz

all: $(exe)

dist: Makefile lab3.cpp utils.cpp utils.h README
	tar -czf $(distName) Makefile lab3.cpp utils.cpp utils.h README

$(exe): lab3.o utils.o
	$(CC) $(FLAGS) -o $(exe) lab3.o utils.o

lab3.o: lab3.cpp
	$(CC) $(FLAGS) -c lab3.cpp

utils.o: utils.cpp
	$(CC) $(FLAGS) -c utils.cpp

clean:
	$(RM) $(distName)
	$(RM) $(exe)
	$(RM) lab3.o
	$(RM) utils.o
