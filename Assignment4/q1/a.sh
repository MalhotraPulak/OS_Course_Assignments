#!/bin/bash
echo "compiling and storing input"
g++ input.cpp -o input
./input "$1" > input.txt
gcc q1.c -pthread -o q1
./q1 < input.txt
rm input
rm q1
