#!/bin/bash
for i in {1..1000}
  do 
     echo "$i"
     ./a.sh "$i" > data.txt 2>> cc.txt
 done
