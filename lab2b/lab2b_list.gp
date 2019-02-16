#! /usr/bin/gnuplot
#NAME: Robert Geil
#EMAIL: rgeil@ucla.edu
#ID: 104916969

# general plot parameters
set terminal png
set datafile separator ","

### Format of the CSV file ###
# test,#threads,#iterations,List Count, #total operations,total runtime,runtime per operation, average wait time

# List 1: Operations Per Second of Spin and Mutex locks
# Output: lab2b_1.png
set title "List-1: Operations Per Second as a Function of Threads"
set xlabel "Threads"
set ylabel "Operations Per Second"
set logscale y 10
set output 'lab2b_1.png'

plot \
     "< grep 'list-none-s,' lab2b_list.csv" using ($2):(1000000000/$7) \
	title 'Spin' with linespoints lc rgb 'blue', \
     "< grep 'list-none-m,' lab2b_list.csv" using ($2):(1000000000/$7) \
	title 'Mutex' with linespoints lc rgb 'green'

# List 2: 
set title "Operation and Wait Time as a Function of Threads"
set xlabel "Threads"
set output "lab2b_2.png"
set ylabel "Time (Nanoseconds)"
set logscale y 10

plot \
     "< grep 'list-none-m,' lab2b_list.csv" using ($2):($8) \
	title 'Wait for Lock Time' with linespoints lc rgb 'green', \
     "< grep 'list-none-m,' lab2b_list.csv" using ($2):($7) \
	title 'Average Time Per Operation' with linespoints lc rgb 'purple'
