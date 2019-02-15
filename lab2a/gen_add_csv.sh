#NAME: Robert Geil
#EMAIL: rgeil@ucla.edu
#ID: 104916969

spin_iterations=(10 20 40 80 100 1000)
threads=(1 2 4 8 12)
exe=./lab2_add
# Without yields, no locks
for it in 100 1000 10000 100000; do
    $exe --iterations=$it # just one thread
    for thr in 1 2 4 6 8 10 12 14; do
        $exe --threads=$thr --iterations=$it
    done
done
# With yields, no locks
for it in 10 20 40 80 100 1000 10000 100000; do
    $exe --iterations=$it --yield # just one thread
    for thr in ${threads[@]}; do
        $exe --threads=$thr --iterations=$it --yield
    done
done

# Mutex lock, without yield
for it in 10 20 40 80 100 1000 10000; do
    for thr in ${threads[@]}; do
        $exe --threads=$thr --iterations=$it --sync=m
    done
done

# Mutex lock, with yield
for it in 10 20 40 80 100 1000 10000; do
    for thr in ${threads[@]}; do
        $exe --threads=$thr --iterations=$it --sync=m --yield
    done
done

# Spin lock, without yield
for it in 10 20 40 80 100 1000 10000; do
    for thr in ${threads[@]}; do
        $exe --threads=$thr --iterations=$it --sync=s
    done
done

# Spin lock, with yield
for it in 10 20 40 80 100 1000 10000; do
    for thr in ${threads[@]}; do
        $exe --threads=$thr --iterations=$it --sync=s --yield
    done
done

# Compare lock, without yield                                                                                                                                            
for it in 10 20 40 80 100 1000 10000; do
    for thr in ${threads[@]}; do
        $exe --threads=$thr --iterations=$it --sync=c
    done
done

# Compare lock, with yield                                                                                                                                               
for it in 10 20 40 80 100 1000 10000; do
    for thr in ${threads[@]}; do
        $exe --threads=$thr --iterations=$it --sync=c --yield
    done
done