#NAME: Robert Geil
#EMAIL: rgeil@ucla.edu
#ID: 104916969

CC=gcc
FLAGS=-Wall -Wextra -pthread
exe1=lab2_add
exe2=lab2_list
tarname=lab2a-104916969.tar.gz


build: $(exe1) $(exe2)

dist: lab2_add.c Makefile README tests.sh lab2_add.gp SortedList.h SortedList.c lab2_list.c
	tar -zcf $(tarname) lab2_add.c Makefile README tests.sh lab2_add.gp SortedList.h SortedList.c lab2_list.c

graphs: tests lab2_add.gp
	gnuplot lab2_add.gp

tests: $(exe1) tests.sh
	./tests.sh > lab2_add.csv

$(exe1): lab2_add.o
	$(CC) $(FLAGS) -o $(exe1) lab2_add.o

$(exe2): lab2_list.o SortedList.o
	$(CC) $(FLAGS) -o $(exe2) lab2_list.o SortedList.o


lab2_add.o: lab2_add.c
	$(CC) $(FLAGS) -c lab2_add.c

lab2_list.o: lab2_list.c
	$(CC) $(FLAGS) -c lab2_list.c
SortedList.o: SortedList.c
	$(CC) $(FLAGS) -c SortedList.c

debug: lab2_add.c lab2_list.c SortedList.c
	$(CC) $(FLAGS) -g -o $(exe1) lab2_add.c
	$(CC) $(FLAGS) -g -o $(exe2) lab2_list.c SortedList.c
clean:
	$(RM) $(exe1)
	$(RM) $(exe2)
	$(RM) $(tarname)
	$(RM) lab2_add.o
	$(RM) lab2_list.o
	$(RM) SortedList.o
	$(RM) lab2_add.csv
	$(RM) lab2_add-*.png
