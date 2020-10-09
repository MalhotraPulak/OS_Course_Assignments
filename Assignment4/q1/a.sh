#!/bin/bash
echo "compiling and storing input"
g++ input.cpp -o input
./input > input.txt
gcc q1.c -o q1
echo "Running fake multi-process"
time (./q1 > out1.txt < input.txt)
echo "Running multi-process"
gcc q1_2.c -o q1_n
time (./q1_n > out2.txt < input.txt)
gcc normal_msort.c -o normal
echo "Running normal"
time (./normal > out3.txt < input.txt)
echo "diff start"
diff out1.txt out3.txt > diff.txt
echo "******" >> diff.txt
diff out2.txt out3.txt >> diff.txt
echo "diff complete"
