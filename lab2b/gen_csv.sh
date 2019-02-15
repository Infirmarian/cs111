#NAME: Robert Geil
#EMAIL: rgeil@ucla.edu
#ID: 104916969

# Generate data for plot 1
iterations=1000
threads=(1 2 4 8 12 16 24)
# Mutex
for thread in ${threads[@]}; do
    ./lab2_list --threads=$thread --iterations=$iterations --sync=m
done
# Spin Lock
for thread in ${threads[@]}; do
    ./lab2_list --threads=$thread --iterations=$iterations --sync=s
done

