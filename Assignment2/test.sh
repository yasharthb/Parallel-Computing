i=1
N=5
sum=0.0
D=$1
O=$2
M=$3
while [ $i -le $N ]
do
  mpirun -np 16 -f hostfile ./code $D $O $M
  num=$(< data.tmp)
  sum=$(echo "scale=6;$sum +$num" | bc -l)
  i=$((i + 1))
done
avg=$(echo $sum / $N | bc -l)
printf "Option: %d Mode: %d Data: %d Avg_Time: %.6lf\n" $O $M $D $avg
