#!/bin/env bash
N=10000000

make all
for step in 1 4 50 17 192 1001 2000 2020 2030
do
    echo step $step; ./main -step $step -n $N; 
done;

echo random order; ./main -r -n $N
