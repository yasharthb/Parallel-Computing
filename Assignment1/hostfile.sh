#!bin/bash
if [ $# -ne 2 ]
then
    echo "Error: Argument missing. Enter Processes per node and number of nodes."
    exit -1
fi
ppn=$1
num_nodes=$2
> hostfile
touch hostfile.tmp
for i in $(seq 22 34)   #4 core nodes beyond this range. Avoiding check
do
    ping -c2 csews$i.cse.iitk.ac.in > /dev/null
    if [ $? -eq 0 ]
    then
        echo "csews$i:$ppn" >> hostfile.tmp
    fi
done
head -$num_nodes hostfile.tmp >> hostfile
rm hostfile.tmp
