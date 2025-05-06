#!/bin/bash
mkdir -p ../build
cd ../build
cmake ..
make
mpirun -np 2 --oversubscribe ./bubble_sort_ist