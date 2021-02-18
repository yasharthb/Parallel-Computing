#!bin/bash
if [ $# -ne 1 ]
then
    echo "Error: Argument missing. Enter Processes per node."
    exit -1
fi

ppn=$1
> hostfile

for i in $(seq 1 60)
do
    ping -c2 csews$i.cse.iitk.ac.in > /dev/null
    if [ $? -eq 0 ]
    then
        echo "csews$i:$ppn" >> hostfile
    fi
done
