# Job script to run the complete assignment
  
#!bin/bash

# Compile the source code
make clean
make
bash hostfile.sh 8

for P in 16 36 49 64
do
    for (( N=16; N<=1024; N=N*2 ))
        do
            for execution in {1..5}
                do
                    echo P:$P N:$N Execution:$execution         
                    mpirun -np $P -f hostfile ./halo $N 50 1
                done
        done
done
