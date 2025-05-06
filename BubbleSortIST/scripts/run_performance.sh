#!/bin/bash
mkdir -p ../build
cd ../build
cmake ..
make

# Test configurations
N_VALUES="4 5 6 7"
NP_VALUES="1 2 4"
THREADS_VALUES="1 2 4"

for n in $N_VALUES; do
    # Update main.cpp with n
    sed -i "s/const int n = [0-9]*;/const int n = $n;/" ../BubbleSortIST/src/main.cpp
    make

    for np in $NP_VALUES; do
        for threads in $THREADS_VALUES; do
            echo "Running n=$n, np=$np, threads=$threads"
            export OMP_NUM_THREADS=$threads
            /usr/bin/time -v mpirun -np $np --oversubscribe ./bubble_sort_ist 2>&1 | tee run_n${n}_np${np}_threads${threads}.log
            # Run with gprof
            mpirun -np $np --oversubscribe ./bubble_sort_ist
            gprof ./bubble_sort_ist gmon.out > gprof_n${n}_np${np}_threads${threads}.txt
            # Optional: Run with mpip
            # mpirun -np $np --oversubscribe mpip ./bubble_sort_ist
        done
    done
done