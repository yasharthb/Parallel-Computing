# Job script to run the complete assignment 
#!bin/bash

# Compile the source code
make clean
make

touch data.tmp> /dev/null 2>&1
for j in {1..4}
do
  rm "data_${j}.csv" > /dev/null 2>&1
  touch "data_${j}.csv"
  printf "D,P,ppn,mode,time\n" >> "data_${j}.csv"
done
for execution in {1..10}
do
   for P in 16
   do
      for ppn in 8
      do
	 for D in 16 256 2048
         do
	    for option in {1..4}
            do
                echo "Generating fresh hostfile...."
		python script.py 4 $((P/4)) $ppn
                sum_d=0.0
                sum_o=0.0
                tmp=0.0
                N=2
                data_file="data_${option}.csv"
                for i in {1..5}
                do
	           for optimized in 0 1
                   do
                      mpirun -np $((P*ppn)) -f hostfile ./code $D $option $optimized
                      tmp=$(<data.tmp)
                      if [[ $optimized -eq 1 ]]
		      then
			  sum_o=$(echo "scale=6;$sum_o +$tmp" | bc -l)
		      else
                          sum_d=$(echo "scale=6;$sum_d +$tmp" | bc -l)
		      fi
                   done
		done
		avg_d=$(echo $sum_d / $N | bc -l)
                printf "%d, %d, %d, 0, %.6lf\n" $D $P $ppn $avg_d >>$data_file
		avg_o=$(echo $sum_o / $N | bc -l)
		printf "%d, %d, %d, 1, %.6lf\n" $D $P $ppn $avg_o >>$data_file
	    done
	done
      done
   done
done
rm data.tmp > /dev/null 2>&1
echo "All configurations done! Generating plots"
python plot.py
mv plot_1.jpg plot_Bcast.jpg
mv plot_2.jpg plot_Reduce.jpg
mv plot_3.jpg plot_Gather.jpg
mv plot_4.jpg plot_Alltoallv.jpg
echo "Plots generated! Exiting"

