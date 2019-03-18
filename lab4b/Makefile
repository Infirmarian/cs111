#NAME: Robert Geil
#EMAIL: rgeil@ucla.edu
#ID: 104916969

exe=lab4b
dist_name=lab4b-104916969.tar.gz
CC=gcc
FLAGS=-Wall -Wextra -lmraa


all: lab4b.c
	$(CC) $(FLAGS) -lm lab4b.c -o $(exe)

check: sensor_check.c
	$(CC) $(FLAGS) sensor_check.c -o sensor
	./sensor

dist: lab4b.c Makefile README sensor_check.c
	tar -czf $(dist_name) lab4b.c Makefile README sensor_check.c

clean:
	$(RM) $(exe)
	$(RM) $(dist_name)
	$(RM) sensor