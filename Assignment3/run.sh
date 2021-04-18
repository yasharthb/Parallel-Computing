# Job script to run the complete assignment 
#!bin/bash

# Compile the source code
make clean
make


data_file="data.csv"

rm "output.txt" > /dev/null 2>&1
touch "output.txt"
rm $data_file > /dev/null 2>&1
touch $data_file
rm "data.tmp" > /dev/null 2>&1
touch "data.tmp"

for execution in {1..5}
do
  for P in 1 2
    do
      for ppn in 1 2 4
        do
	  tmp=0.0
	  printf "Running Configuration: P=%d, PPN=%d\n" $P $ppn 
          python script.py 1 $P $ppn
          mpirun -np $((P*ppn)) -ppn $ppn -f hostfile ./code "tdata.csv"
	  tmp=$(<data.tmp)
	  printf "%d, %d, %.6lf\n" $P $ppn $tmp >>$data_file
      done
  done
done

rm "data.tmp" > /dev/null 2>&1
python plot.py
echo "All configurations done! Generating plots"
