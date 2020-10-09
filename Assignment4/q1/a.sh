#!/bin/bash
echo "compiling and storing input"
g++ input.cpp -o input
./input > input.txt

echo "Running OG multiprocess"
gcc q1_og.c -o q1
time (./q1 > out.txt < input.txt)

echo "Running multi-process"
gcc q1_2.c -o q1_2
time (./q1_2 > out2.txt < input.txt)

echo "Running efficient multi-process"
gcc q1_3.c -o q1_3
time (./q1_3 > out3.txt < input.txt)

gcc normal_msort.c -o normal
echo "Running normal"
time (./normal > out3.txt < input.txt)

echo "diff start"
diff out.txt out3.txt > diff.txt
echo "******" >> diff.txt
diff out2.txt out3.txt >> diff.txt
echo "******" >> diff.txt
diff out3.txt out3.txt >> diff.txt
echo "diff complete"
