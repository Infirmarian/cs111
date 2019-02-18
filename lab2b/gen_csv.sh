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

# Generate data for multi-list implementation, make sure that all tests pass
iterations=(1 2 4 8 16)
threads=(1 4 8 12 16)
lists=4
yield_opt=id
# No synchronization
for it in ${iterations[@]}; do
    for thread in ${threads[@]}; do
        ./lab2_list --threads=$thread --iterations=$it --lists=$lists --yield=$yield_opt
    done
done

iterations=(10 20 40 80)
# Mutex locking
for it in ${iterations[@]}; do
    for thread in ${threads[@]}; do
        ./lab2_list --threads=$thread --iterations=$it --lists=$lists --yield=$yield_opt --sync=m
    done
done
# Spin locking
for it in ${iterations[@]}; do
    for thread in ${threads[@]}; do
        ./lab2_list --threads=$thread --iterations=$it --lists=$lists --yield=$yield_opt --sync=s
    done
done

# Throughput timing with multiple lists
threads=(1 2 4 8 12)
lists=(1 4 8 16)
iterations=1000
# Mutex Locking
for thread in ${threads[@]}; do
    for listcount in ${lists[@]}; do
        ./lab2_list --threads=$thread --iterations=$iterations --lists=$listcount --sync=m
    done
done

# Spin Locking
for thread in ${threads[@]}; do
    for listcount in ${lists[@]}; do
        ./lab2_list --threads=$thread --iterations=$iterations --lists=$listcount --sync=s
    done
done



