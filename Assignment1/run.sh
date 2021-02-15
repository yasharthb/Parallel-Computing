# Job script to run the complete assignment

#!bin/bash

# Compile the source code
make clean
make

for (( N=16; N<=1024; N=N*2 ))
do
    for P in 16 36 49 64
        do
            for execution in {1..5}
                do 
                    mpirun -np $P ./halo $N $P 1
                done
        done
done

