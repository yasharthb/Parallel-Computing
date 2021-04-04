# Job script to run the complete assignment 
#!bin/bash

# Compile the source code
make clean
make

rm "output.txt" > /dev/null 2>&1
touch "data_${j}.csv"

for P in 1 2
  do
    for ppn in 1 2 4
      do
        python script.py 1 $P $ppn
        mpicc -np $((P*ppn)) -f hostfile ./code "tdata.csv"
    done
done

echo "All configurations done! Generating plots"