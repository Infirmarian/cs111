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
     "< grep 'list-none-s,' lab2b_list.csv | head -7" using ($2):(1000000000/$7) \
	title 'Spin' with linespoints lc rgb 'blue', \
     "< grep 'list-none-m,' lab2b_list.csv | head -7" using ($2):(1000000000/$7) \
	title 'Mutex' with linespoints lc rgb 'green'

# List 2: 
set title "Operation and Wait Time as a Function of Threads"
set xlabel "Threads"
set output "lab2b_2.png"
set ylabel "Time (Nanoseconds)"
set logscale y 10

plot \
     "< grep 'list-none-m,' lab2b_list.csv | head -7" using ($2):($8) \
	title 'Wait for Lock Time' with linespoints lc rgb 'green', \
     "< grep 'list-none-m,' lab2b_list.csv | head -7" using ($2):($7) \
	title 'Average Time Per Operation' with linespoints lc rgb 'purple'


# List 3:
set title "Successful List Operations With Yields"
set xlabel "Threads"
set ylabel "Iterations"
unset logscale y
set output "lab2b_3.png"

plot \
     "< grep 'list-id-none,' lab2b_list.csv" using ($2):($3) \
	title 'No Synchronization' with points lc rgb 'red', \
     "< grep 'list-id-s,' lab2b_list.csv" using ($2):($3) \
	title 'Spin Lock' with points pt 1, \
     "< grep 'list-id-m,' lab2b_list.csv" using ($2):($3) \
	title 'Mutex Lock' with points pt 4

# List 4: Throughput for Mutex Locks and multiple lists
set title "Mutex Lock Performance for Multiple Lists and Threads"
set xlabel "Threads"
set ylabel "Operations Per Second"
set output "lab2b_4.png"
set logscale y 10
plot \
     "< grep 'list-none-m,[0-9]*,1000,1,' lab2b_list.csv | tail -5" using ($2):(1e9/$7) \
	title '1 List' with linespoints lc rgb 'blue',\
     "< grep 'list-none-m,[0-9]*,1000,4,' lab2b_list.csv" using ($2):(1e9/$7) \
	title '4 Lists' with linespoints lc rgb 'red', \
     "< grep 'list-none-m,[0-9]*,1000,8,' lab2b_list.csv" using ($2):(1e9/$7) \
	title '8 Lists' with linespoints lc rgb 'purple', \
     "< grep 'list-none-m,[0-9]*,1000,16,' lab2b_list.csv" using ($2):(1e9/$7) \
	title '16 Lists' with linespoints lc rgb 'orange'

# List 5: Throughput for Spin Locks and multiple lists
set title "Spin Lock Performance for Multiple Lists and Threads"
set xlabel "Threads"
set ylabel "Operations Per Second"
set output "lab2b_5.png"
set logscale y 10
plot \
     "< grep 'list-none-s,[0-9]*,1000,1,' lab2b_list.csv | tail -5" using ($2):(1e9/$7) \
	title '1 List' with linespoints lc rgb 'blue',\
     "< grep 'list-none-s,[0-9]*,1000,4,' lab2b_list.csv" using ($2):(1e9/$7) \
	title '4 Lists' with linespoints lc rgb 'red', \
     "< grep 'list-none-s,[0-9]*,1000,8,' lab2b_list.csv" using ($2):(1e9/$7) \
	title '8 Lists' with linespoints lc rgb 'purple', \
     "< grep 'list-none-s,[0-9]*,1000,16,' lab2b_list.csv" using ($2):(1e9/$7) \
	title '16 Lists' with linespoints lc rgb 'orange'