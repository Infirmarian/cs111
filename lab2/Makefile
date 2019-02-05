#NAME: Robert Geil
#EMAIL: rgeil@ucla.edu
#ID: 104916969

CC=gcc
FLAGS=-Wall -Wextra
pname=lab2
tarname=lab2-104916969.tar.gz


build: $(pname)
dist: lab2_add.c Makefile 
	tar -zcf $(tarname) lab2_add.c Makefile
check: $(pname) check.sh
	echo "TODO: write checks"
$(pname): lab2_add.o
	$(CC) $(FLAGS) -o $(pname) lab2_add.o
lab2_add.o: lab2_add.c
	$(CC) $(FLAGS) -c lab2_add.c
debug:
	$(CC) $(FLAGS) -g -o $(pname) lab2_add.c
clean:
	$(RM) $(pname)
	$(RM) $(tarname)
	$(RM) lab2_add.o
