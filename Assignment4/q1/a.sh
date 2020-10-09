#!/bin/bash
echo "compiling and storing input"
g++ input.cpp -o input
./input > input.txt
gcc q1.c -o q1
echo "Running multi-process"
time (./q1 > out1.txt < input.txt)
gcc normal_msort.c -o normal
echo "Running normal"
time (./normal > out2.txt < input.txt)
echo "diff start"
diff out1.txt out2.txt > diff.txt
echo "diff complete"
