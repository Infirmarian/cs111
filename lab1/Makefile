#NAME: Robert Geil
#EMAIL: rgeil@ucla.edu
#ID: 104916969

CC=gcc
FLAGS=-Wall -Wextra
pname=simpsh
tarname=lab1-104916969.tar.gz


all: $(pname)
dist: lab1.c README Makefile check.sh
	tar -zcf $(tarname) lab1.c README Makefile check.sh
check: $(pname) check.sh
	./check.sh $(pname)
$(pname): lab1.c
	$(CC) $(FLAGS) -o $(pname) lab1.c
debug: lab1.c
	$(CC) $(FLAGS) -g -o $(pname) lab1.c
clean:
	$(RM) $(pname)
	$(RM) $(tarname)