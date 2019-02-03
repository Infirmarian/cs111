#NAME: Robert Geil
#EMAIL: rgeil@ucla.edu
#ID: 104916969

CC=gcc
FLAGS=-Wall -Wextra
pname=simpsh
tarname=lab1-104916969.tar.gz


all: $(pname)
dist: lab1.c README Makefile check.sh utils.c utils.h report.pdf
	tar -zcf $(tarname) lab1.c README Makefile check.sh utils.c utils.h report.pdf
check: $(pname) check.sh
	./check.sh $(pname) 2> /dev/null
$(pname): lab1.o utils.o
	$(CC) $(FLAGS) -o $(pname) lab1.o utils.o
lab1.o: lab1.c
	$(CC) $(FLAGS) -c lab1.c
utils.o: utils.c
	$(CC) $(FLAGS) -c utils.c
debug: lab1.c
	$(CC) $(FLAGS) -g -o $(pname) lab1.c utils.c
clean:
	$(RM) $(pname)
	$(RM) $(tarname)
	$(RM) lab1.o
	$(RM) utils.o
