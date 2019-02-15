#NAME: Robert Geil
#EMAIL: rgeil@ucla.edu
#ID: 104916969

CC=gcc
FLAGS=-Wall -Wextra -pthread
exe=lab2_list
tarname=lab2a-104916969.tar.gz


build: $(exe)

dist: Makefile README lab2_list.gp SortedList.h SortedList.c lab2_list.c
	tar -zcf $(tarname) Makefile README lab2_list.gp SortedList.h SortedList.c lab2_list.c

graphs: lab2_list_plot

tests: lab2b_list.csv

profile: $(exe)
	CPUPROFILE=profile.out ./$(exe) --iterations=1000 --threads=12 --sync=s

lab2b_list.csv: $(exe) gen_csv.sh
	./gen_csv.sh > lab2b_list.csv 2>/dev/null

lab2_list_plot: lab2_list.csv lab2b_list.gp
	gnuplot lab2b_list.gp

$(exe): lab2_list.o SortedList.o
	$(CC) $(FLAGS) -o $(exe) lab2_list.o SortedList.o -lprofiler

lab2_list.o: lab2_list.c
	$(CC) $(FLAGS) -c lab2_list.c

SortedList.o: SortedList.c
	$(CC) $(FLAGS) -c SortedList.c

debug: lab2_list.c SortedList.c
	$(CC) $(FLAGS) -g -o $(exe2) lab2_list.c SortedList.c

clean:
	$(RM) $(exe)
	$(RM) $(tarname)
	$(RM) lab2_list.o
	$(RM) SortedList.o