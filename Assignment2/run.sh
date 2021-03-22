# Job script to run the complete assignment 
#!bin/bash

# Compile the source code
make clean
make

for execution in {1..10} do
   for P in 4 16 do
      for ppn in 1 8 do
	 for D in 16 256 2048 do
	    for option in {1..4} do
		python script.py
		for optimized in 0 1 do
		   data_file="data_${option}.txt"
		   mpirun -np $((P*ppn)) -f hostfile ./code D option optimized | tee -a $data_file
		done
	    done
	done
      done
   done
done

echo "All configurations done! Generating plots"
python plot.py

