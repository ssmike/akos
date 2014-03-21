#!/bin/env bash
N=100000000

for F in "-O0" "-O1" "-O2" "-O3" "-g" 
do
    FLAGS=$F make all 2>/dev/null >/dev/null
    echo флаги : $F -ansi -pedantic
    for step in 1 4 50 17 192 1001 2000 2020 2030
    do
        echo -n с шагом $step :\ ; ./main -step $step -n $N; 
    done;

    echo -n в случайном порядке :\ ; ./main -r -n $N

    make clean 2>/dev/null >/dev/null
done
