// Main source file for Assignment1

#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"

// Part-1: Uses multiple sends and receives
void multiple_send_receive(int N, int P){
    /*int myrank, size;
    MPI_Status status;
    double start_time;
    char *buf;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank( MPI_COMM_WORLD, &myrank );
    MPI_Comm_size( MPI_COMM_WORLD, &size );

    MPI_Finalize();*/
}

// Part-2: Uses pack/unpack and send/receives
void pack_send_unpack_receive(int N, int P){
  
}

// Part-3; Uses derived datatypes
void derived_datatype_send_receive(int N, int P){

}


int main( int argc, char *argv[])
{

    int N = atoi(argv[1]); // square root of data points per process (size of matrix row/col)
    int P = atoi (argv[2]); // number of processes
    int option = atoi (argv[3]); // part to run
    
    if (option == 1)
        multiple_send_receive(N,P);
    else if (option == 2)
        pack_send_unpack_receive(N,P);
    else if (option == 3)
        derived_datatype_send_receive(N,P);
    else{
        multiple_send_receive(N,P);
        pack_send_unpack_receive(N,P);
        derived_datatype_send_receive(N,P);
    }

    return 0;
}
