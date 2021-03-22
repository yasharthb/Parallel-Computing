// Main source file for Assignment2

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include "mpi.h"

void mpi_bcast_default(int D){

double avg_time=0.0;
}
void mpi_reduce_default(int D){

double avg_time=0.0;

}
void mpi_gather_default(int D){
double avg_time=0.0;

}
void mpi_alltoallv_default(int D){
double avg_time=0.0;

}


void mpi_bcast_optimized(int D){
double avg_time=0.0;

}
void mpi_reduce_optimized(int D){
double avg_time=0.0;

}
void mpi_gather_optimized(int D){
double avg_time=0.0;

}
void mpi_alltoallv_optimized(int D){
double avg_time=0.0;

}

int main( int argc, char *argv[])
{

    int D = atoi(argv[1]); // Data Size in KBs
    int option = atoi (argv[2]); // part to run
    int optimized = atoi(argv[3]); //Optimized == 1, Default = 0

    if(!optimized){
	if (option == 1)
       		 mpi_bcast_default(D);
   	else if (option == 2)
       		 mpi_reduce_default(D);
	else if (option == 3)
                 mpi_gather_default(D);
	else if (option == 4)
                 mpi_alltoallv_default(D);
	else
		printf("Unsupported default option\n");
   }
   else{
       if (option == 1)
                 mpi_bcast_optimized(D);
       else if (option == 2)
                 mpi_reduce_optimized(D);
       else if (option == 3)
                 mpi_gather_optimized(D);
       else if (option == 4)
                 mpi_alltoallv_optimized(D);
       else
                printf("Unsupported optimized option\n");
   }

    return 0;
}

