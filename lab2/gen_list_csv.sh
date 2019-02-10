#NAME: Robert Geil
#EMAIL: rgeil@ucla.edu
#ID: 104916969

exe=./lab2_list

############## LISTS ##############
# List with just 1 thread
for it in 10 100 1000 10000 20000; do
    $exe --threads=1 --iterations=$it
done

# List with a few threads and iterations
for it in 1 10 100 1000; do
    for thr in 2 4 8 12; do
        $exe --threads=$thr --iterations=$it
    done
done

############### YIELDS #############
# Yield insert
for it in 1 10 100 1000; do
    for thr in 2 4 8 12; do
        $exe --threads=$thr --iterations=$it --yield=i
    done
done
# Yield delete
for it in 1 10 100 1000; do
    for thr in 2 4 8 12; do
        $exe --threads=$thr --iterations=$it --yield=d
    done
done
# Yield insert and lookup
for it in 1 10 100 1000; do
    for thr in 2 4 8 12; do
        $exe --threads=$thr --iterations=$it --yield=il
    done
done
# Yield delete and lookup
for it in 1 10 100 1000; do
    for thr in 2 4 8 12; do
        $exe --threads=$thr --iterations=$it --yield=dl
    done
done

####### PROTECTION AND YIELD #########
threads=(1 2 4 6 8 10 12)
iterations=(1 2 4 8 16 32)

#### MUTEX ####
# Yield insert
for it in ${iterations[@]}; do
    for thr in ${threads[@]}; do
        $exe --threads=$thr --iterations=$it --yield=i --sync=m
    done
done
# Yield delete
for it in ${iterations[@]}; do
    for thr in ${threads[@]}; do
        $exe --threads=$thr --iterations=$it --yield=d --sync=m
    done
done
# Yield insert and lookup
for it in ${iterations[@]}; do
    for thr in ${threads[@]}; do
        $exe --threads=$thr --iterations=$it --yield=il --sync=m
    done
done
# Yield delete and lookup
for it in ${iterations[@]}; do
    for thr in ${threads[@]}; do
        $exe --threads=$thr --iterations=$it --yield=dl --sync=m
    done
done

#### SPIN ####
# Yield insert
for it in ${iterations[@]}; do
    for thr in ${threads[@]}; do
        $exe --threads=$thr --iterations=$it --yield=i --sync=s
    done
done
# Yield delete
for it in ${iterations[@]}; do
    for thr in ${threads[@]}; do
        $exe --threads=$thr --iterations=$it --yield=d --sync=s
    done
done
# Yield insert and lookup
for it in ${iterations[@]}; do
    for thr in ${threads[@]}; do
        $exe --threads=$thr --iterations=$it --yield=il --sync=s
    done
done
# Yield delete and lookup
for it in ${iterations[@]}; do
    for thr in ${threads[@]}; do
        $exe --threads=$thr --iterations=$it --yield=dl --sync=s
    done
done



######### NO YIELD ##########
threads=(1 2 4 8 12 16 24)
# Spin lock
for thr in ${threads[@]}; do
        $exe --threads=$thr --iterations=1000 --sync=s
done

# Spin lock
for thr in ${threads[@]}; do
        $exe --threads=$thr --iterations=1000 --sync=m
done