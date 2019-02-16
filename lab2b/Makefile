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

graphs: lab2b_list_plot

tests: lab2b_list.csv

#$(HOME)/local/lib/libprofiler.so:$(HOME)/local/lib/libtcmalloc.so
profile: $(exe)
	rm -f raw.gperf
	LD_PRELOAD="/usr/lib64/libprofiler.so" CPUPROFILE=./raw.gperf ./$(exe) --iterations=1000 --threads=12 --sync=s
	pprof --text $(exe) ./raw.gperf > profile.out
	pprof -list=add_to_list $(exe) ./raw.gperf >> profile.out

lab2b_list.csv: $(exe) gen_csv.sh
	./gen_csv.sh > lab2b_list.csv 2>/dev/null

lab2b_list_plot: lab2b_list.csv lab2b_list.gp
	gnuplot lab2b_list.gp

$(exe): lab2_list.o SortedList.o utils.o
	$(CC) $(FLAGS) -o $(exe) lab2_list.o SortedList.o utils.o -lprofiler

lab2_list.o: lab2_list.c
	$(CC) $(FLAGS) -c lab2_list.c

SortedList.o: SortedList.c
	$(CC) $(FLAGS) -c SortedList.c

utils.o: utils.c
	$(CC) $(FLAGS) -c utils.c

debug: lab2_list.c SortedList.c utils.c
	$(CC) $(FLAGS) -g -o $(exe) lab2_list.c SortedList.c utils.c

clean:
	$(RM) $(exe)
	$(RM) $(tarname)
	$(RM) lab2_list.o
	$(RM) SortedList.o
	$(RM) utils.o