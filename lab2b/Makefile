#NAME: Robert Geil
#EMAIL: rgeil@ucla.edu
#ID: 104916969

CC=gcc
FLAGS=-Wall -Wextra -lprofiler -g -lpthread
exe=lab2_list
tarname=lab2b-104916969.tar.gz


build: $(exe)

dist: Makefile README lab2b_list.gp SortedList.h SortedList.c lab2_list.c utils.c utils.h gen_csv.sh lab2b_list.csv graphs profile
	tar -zcf $(tarname) Makefile README lab2b_list.gp SortedList.h SortedList.c \
	lab2_list.c utils.c utils.h lab2b_[1-5].png lab2b_list.csv gen_csv.sh profile.out

graphs: lab2b_list_plot

tests: lab2b_list.csv

profile: profile.out

profile.out: $(exe)
	rm -f raw.gperf
	LD_PRELOAD="/usr/lib64/libprofiler.so" CPUPROFILE=./raw.gperf ./$(exe) --iterations=1000 --threads=12 --sync=s
	pprof --text $(exe) ./raw.gperf > profile.out
	pprof -list=add_to_list $(exe) ./raw.gperf >> profile.out
	rm -f raw.gperf

lab2b_list.csv: $(exe) gen_csv.sh
	./gen_csv.sh > lab2b_list.csv 2>/dev/null

lab2b_list_plot: lab2b_list.csv lab2b_list.gp
	gnuplot lab2b_list.gp

$(exe): lab2_list.o SortedList.o utils.o
	$(CC) $(FLAGS) -o $(exe) lab2_list.o SortedList.o utils.o

lab2_list.o: lab2_list.c
	$(CC) $(FLAGS) -c lab2_list.c

SortedList.o: SortedList.c
	$(CC) $(FLAGS) -c SortedList.c

utils.o: utils.c
	$(CC) $(FLAGS) -c utils.c

clean:
	$(RM) $(exe)
	$(RM) $(tarname)
	$(RM) lab2_list.o
	$(RM) SortedList.o
	$(RM) utils.o