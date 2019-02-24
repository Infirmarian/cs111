#NAME: Robert Geil
#EMAIL: rgeil@ucla.edu
#ID: 104916969

exe=lab3
CC=g++
FLAGS=-Wall -Wextra
distName=lab3a-104916969.tar.gz

all: $(exe)

dist: Makefile lab3.cpp
	tar -czf $(distName) Makefile lab3.cpp

$(exe): lab3.cpp
	$(CC) $(FLAGS) -o $(exe) lab3.cpp

clean:
	$(RM) $(distName)
	$(RM) $(exe)
