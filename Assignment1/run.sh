# Job script to run the complete assignment 
#!bin/bash

# Compile the source code
make clean
make

for P in 16 36 49 64
do
    bash hostfile.sh 8 $(((P+7)/8))
    data_file="data${P}.txt"
    if [ -e "$data_file" ]
    then
        rm -f $data_file
    fi
    echo ""
    echo "#Processes:$P"
    for (( N=16; N<=1024; N=N*2 ))
        do
            for opt in {1..3}
            do
                for execution in {1..5}
                do
                    mpirun -np $P -f hostfile ./halo $N 50 $opt | tee -a $data_file
                done
            done
        done
done
