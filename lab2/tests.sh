iterations=(10 20 40 80 100 1000 10000 100000)
threads=(2 4 8 12)
# Without yields
for it in ${iterations[@]}; do
        # Run a single thread as well, just for add-none
        ./lab2 --threads=1 --iterations=$it
    for thr in ${threads[@]}; do
        ./lab2 --threads=$thr --iterations=$it
    done
done
# With yields
for it in ${iterations[@]}; do
    for thr in ${threads[@]}; do
        ./lab2 --threads=$thr --iterations=$it --yield
    done
done