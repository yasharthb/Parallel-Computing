// Main source file for Assignment2

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include "mpi.h"
#define KB 1024

int get_group_id(char *name, int length){
    
    FILE* fp;
    char  line[255];
    int line_no = -1;
    fp = fopen("nodefile.txt" , "r");
    while (fgets(line, sizeof(line), fp) != NULL)
    { 
        line_no++; 
        char* tok = strtok(line, ",");
        while(tok!=NULL)
        {
            if(strncmp(name, tok, length)==0)
                return line_no;
            tok=strtok(NULL,"\n ,+=");
        }

    } 
    return line_no; 
}

void mpi_bcast_default(int D){

//  printf("Inside Bcast Default\n");

  int myrank, size;
  double *buf;
  int count = (D*KB)/sizeof(double);
  buf = (double *)malloc(D*KB);

  MPI_Init(NULL, NULL);
  MPI_Comm_rank( MPI_COMM_WORLD, &myrank);
  MPI_Comm_size( MPI_COMM_WORLD, &size);

  // Initialize buffer to random values
  srand(time(NULL));
  double high = 2021.0;
  for (int i=1; i<=count; i++)
     buf[i] = (high*(double)rand())/(double)RAND_MAX;

  // has to be called by all processes
  double sTime = MPI_Wtime();
  MPI_Bcast(buf, count, MPI_DOUBLE, 1, MPI_COMM_WORLD);
  double eTime = MPI_Wtime();

  // simple check
  printf ("bcast default %d %lf \n", myrank, eTime - sTime);

  MPI_Finalize();
  free(buf);
}
void mpi_reduce_default(int D){

  int myrank, size, count;
  double *buf = (double *)malloc(D*KB);
  double *recvBuf = (double *)malloc(D*KB);
  count = (D*KB)/sizeof(double);

  MPI_Init(NULL, NULL);
  MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  // Initialize buffer to random values
  srand(time(NULL));
  double high = 2021.0;
  for (int i=1; i<=count; i++)
     buf[i] = (high*(double)rand())/(double)RAND_MAX;
 
  double sTime = MPI_Wtime();
  MPI_Reduce(buf, recvBuf, count, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD); 
  double eTime = MPI_Wtime();

  // simple check
  printf ("reduce default %d %lf \n", myrank, eTime - sTime);

  // finalize
  MPI_Finalize();
  free(buf);
  free(recvBuf);
}
void mpi_gather_default(int D){

  int myrank, size, count;
  double *buf = (double *)malloc(D*KB);;
  count = (D*KB)/sizeof(double);

  // Setup
  MPI_Init(NULL, NULL);
  MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  // Initialize buffer to random values
  srand(time(NULL));
  double high = 2021.0;
  for (int i=1; i<=count; i++)
     buf[i] = (high*(double)rand())/(double)RAND_MAX;

  double *recvBuf = (double *)malloc(D*KB*size); //significant at the root process
 
  double sTime = MPI_Wtime();
  MPI_Gather(buf, count, MPI_DOUBLE, recvBuf, count, MPI_DOUBLE, 0, MPI_COMM_WORLD);
  double eTime = MPI_Wtime();

  // simple check
  printf ("gather default %d %lf \n", myrank, eTime - sTime);

  // finalize
  MPI_Finalize();
  free(buf);
  free(recvBuf);
}
void mpi_alltoallv_default(int D){
printf("Inside A2Av Default\n");
}

void mpi_bcast_optimized(int D){

  int myrank, size, length;
  double *buf;
  int count = (D*KB)/sizeof(double);
  buf = (double *)malloc(D*KB);
  char name[MPI_MAX_PROCESSOR_NAME];
  
  MPI_Init(NULL, NULL);
  MPI_Comm_rank( MPI_COMM_WORLD, &myrank);
  MPI_Comm_size( MPI_COMM_WORLD, &size);
  MPI_Get_processor_name(name, &length); //Used to later to identify the group to which the node belongs 


  // Creating Intra Groups for all set of nodes in action
  int intra_color = get_group_id(name,length);
  int intra_rank, intra_size;

  MPI_Comm intra_comm;
  MPI_Comm_split (MPI_COMM_WORLD, intra_color, myrank, &intra_comm);

  MPI_Comm_rank(intra_comm,&intra_rank);
  MPI_Comm_size(intra_comm,&intra_size);


  // Creating an inter communication picking the 0th ranked element in each intra group
  int inter_color = 0;
  int inter_rank, inter_size;

  MPI_Comm inter_comm;
  if(intra_rank == 0)
  	MPI_Comm_split(MPI_COMM_WORLD, inter_color, myrank, &inter_comm);
  else
	MPI_Comm_split(MPI_COMM_WORLD, MPI_UNDEFINED, myrank,&inter_comm);

  MPI_Comm_rank(inter_comm,&inter_rank);
  MPI_Comm_size(inter_comm,&inter_size);

  
 /* 
  * printf("My Rank: %d My Node %s My Group ID: %d\n", myrank, name, get_group_id(name,length));
 */

  // Initialize buffer to random values
  srand(time(NULL));
  double high = 2021.0;
  for (int i=1; i<=count; i++)
     buf[i] = (high*(double)rand())/(double)RAND_MAX;

  // has to be called by all processes
  double sTime = MPI_Wtime();
  MPI_Bcast(buf, count, MPI_DOUBLE, 0, inter_comm);
  MPI_Bcast(buf, count, MPI_DOUBLE, 0, intra_comm);
  double eTime = MPI_Wtime();

  // simple check
  printf ("bcast default %d %lf \n", myrank, eTime - sTime);

  MPI_Finalize();
  free(buf);
}
void mpi_reduce_optimized(int D){

  int myrank, size, length;
  double *buf, *recvBuf, *recvBuf2;
  int count = (D*KB)/sizeof(double);
  buf = (double *)malloc(D*KB);
  recvBuf = (double *) malloc(D*KB);
  recvBuf2 = (double *) malloc(D*KB);
  char name[MPI_MAX_PROCESSOR_NAME];
  
  MPI_Init(NULL, NULL);
  MPI_Comm_rank( MPI_COMM_WORLD, &myrank);
  MPI_Comm_size( MPI_COMM_WORLD, &size);
  MPI_Get_processor_name(name, &length); //Used to later to identify the group to which the node belongs 


  // Creating Intra Groups for all set of nodes in action
  int intra_color = get_group_id(name,length);
  int intra_rank, intra_size;

  MPI_Comm intra_comm;
  MPI_Comm_split (MPI_COMM_WORLD, intra_color, myrank, &intra_comm);

  MPI_Comm_rank(intra_comm,&intra_rank);
  MPI_Comm_size(intra_comm,&intra_size);


  // Creating an inter group communication picking the 0th ranked element in each intra group
  int inter_color = 0;
  int inter_rank, inter_size;

  MPI_Comm inter_comm;
  if(intra_rank == 0)
    MPI_Comm_split(MPI_COMM_WORLD, inter_color, myrank, &inter_comm);
  else
    MPI_Comm_split(MPI_COMM_WORLD, MPI_UNDEFINED, myrank,&inter_comm);

  MPI_Comm_rank(inter_comm,&inter_rank);
  MPI_Comm_size(inter_comm,&inter_size);

  
 /* 
  * printf("My Rank: %d My Node %s My Group ID: %d\n", myrank, name, get_group_id(name,length));
 */

  // Initialize buffer to random values
  srand(time(NULL));
  double high = 2021.0;
  for (int i=1; i<=count; i++)
     buf[i] = (high*(double)rand())/(double)RAND_MAX;

  // has to be called by all processes
  double sTime = MPI_Wtime();
  MPI_Reduce(buf, recvBuf, count, MPI_DOUBLE, MPI_MAX, 0, intra_comm);
  MPI_Reduce(recvBuf, recvBuf2, count, MPI_DOUBLE, MPI_MAX, 0, inter_comm);
  double eTime = MPI_Wtime();

  // simple check
  printf ("bcast default %d %lf \n", myrank, eTime - sTime);

  MPI_Finalize();
  free(buf);

}
void mpi_gather_optimized(int D){
double avg_time=0.0;
printf("Inside gather Optimized\n");

}
void mpi_alltoallv_optimized(int D){
double avg_time=0.0;
printf("Inside A2Av optimized\n");
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

