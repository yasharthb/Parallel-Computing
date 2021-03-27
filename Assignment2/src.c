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

void mpi_bcast_default(int D, double *time_curr){

//  printf("Inside Bcast Default\n");

  int myrank, size;
  double *buf;
  int count = (D*KB)/sizeof(double);
  buf = (double *)malloc(D*KB);
  FILE *fp;
  fp = fopen("data.tmp", "w");

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

  double time = eTime - sTime;
  double max_time;

  MPI_Reduce (&time, &max_time, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

  // simple check
  printf ("bcast default %d %lf \n", myrank, eTime - sTime);

  if (myrank == 0){
	  fprintf(fp,"%lf\n",max_time);
	  fprintf(stdout,"%lf\n",max_time);
 	  *time_curr = max_time;
  }

  fclose(fp);
  MPI_Finalize();
  free(buf);
}

void mpi_reduce_default(int D, double *time_curr){

  int myrank, size, count;
  double *buf = (double *)malloc(D*KB);
  double *recvBuf = (double *)malloc(D*KB);
  count = (D*KB)/sizeof(double);
  FILE *fp;
  fp = fopen("data.tmp", "w");

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

  double time = eTime - sTime;
  double max_time;

  MPI_Reduce (&time, &max_time, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

  // simple check
  printf ("reduce default %d %lf \n", myrank, eTime - sTime);

  if (myrank == 0){
          fprintf(fp,"%lf\n",max_time);
          fprintf(stdout,"%lf\n",max_time);
          *time_curr = max_time;
  }


  // finalize
  MPI_Finalize();
  free(buf);
  free(recvBuf);
}

void mpi_gather_default(int D, double *time_curr){

  int myrank, size, count;
  double *buf = (double *)malloc(D*KB);;
  count = (D*KB)/sizeof(double);
  FILE *fp;
  fp = fopen("data.tmp", "w");

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

  double time = eTime - sTime;
  double max_time;

  MPI_Reduce (&time, &max_time, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

  // simple check
  printf ("gather default %d %lf \n", myrank, eTime - sTime);

  if (myrank == 0){
          fprintf(fp,"%lf\n",max_time);
          fprintf(stdout,"%lf\n",max_time);
          *time_curr = max_time;
  }

  // finalize
  MPI_Finalize();
  free(buf);
  free(recvBuf);
}

void mpi_alltoallv_default(int D, double *time_curr){
	  
  int myrank, size, send_count,total_count;
  FILE *fp;
  fp = fopen("data.tmp", "w");


  // Setup
  MPI_Init(NULL, NULL);
  MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  double *buf = (double *)malloc(D*KB*size);
  double *recvBuf = (double *)malloc(D*KB*size);

  send_count = (D*KB)/sizeof(double);
  total_count = send_count*size;

  /* Load up the buffers */
  for (int i=0; i<total_count; i++) {
      buf[i] = i + 100*myrank;
      recvBuf[i] = -i;
  }

/*
  // Initialize buffer to random values
  srand(time(NULL));
  double high = 2021.0;
  for (int i=1; i<=count; i++)
     buf[i] = (high*(double)rand())/(double)RAND_MAX;
*/


  int *countBuf = (int *)malloc( size * sizeof(int));
  int *recvCountBuf = (int *)malloc( size * sizeof(int));
  int *recvDisplBuf = (int *)malloc( size * sizeof(int));
  int *displBuf = (int *)malloc( size * sizeof(int));

  for (int i=0; i<size; i++) {
      countBuf[i] = send_count;
      recvCountBuf[i] = send_count;
      recvDisplBuf[i] = i*send_count;
      displBuf[i] = i*send_count;
  }

  double sTime = MPI_Wtime();
  MPI_Alltoallv( buf, countBuf, displBuf, MPI_DOUBLE,
                       recvBuf, recvCountBuf, recvDisplBuf, MPI_DOUBLE, MPI_COMM_WORLD);
  double eTime = MPI_Wtime();


/* Check recvBuf
    int *p;
    for (int i=0; i<size; i++) {
        p = recvBuf + recvDisplBuf[i];
        for (int j=0; j<send_count; j++) {
            if (p[j] != i * send_count) {
                printf("[%d] got %d expected %d for %dth\n",
                                    myrank, p[j],(i*(i+1))/2 + j, j);
            }else{
                printf("[%d] got %d expected %d for %dth\n",
                                    myrank, p[j],(i*(i+1))/2 + j, j);
	    }
        }
    }
*/

  double time = eTime - sTime;
  double max_time;

  MPI_Reduce (&time, &max_time, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

  printf ("AlltoAllv default %d %lf \n", myrank, eTime - sTime);

  if (myrank == 0){
          fprintf(fp,"%lf\n",max_time);
          fprintf(stdout,"%lf\n",max_time);
          *time_curr = max_time;
  }

  // Finalize
  MPI_Finalize();
  free(buf);
  free(recvBuf);
  free(countBuf);
  free(recvCountBuf);
  free(displBuf);
  free(recvDisplBuf);

/* Inspired from http://mpi.deino.net/mpi_functions/MPI_Alltoallv.html */

}

void mpi_bcast_optimized(int D, double *time_curr){

  int myrank, size, length;
  double *buf;
  int count = (D*KB)/sizeof(double);
  buf = (double *)malloc(D*KB);
  char name[MPI_MAX_PROCESSOR_NAME];
 
  FILE *fp;
  fp = fopen("data.tmp", "w");


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
  int inter_rank=-1, inter_size=-1;

  MPI_Comm inter_comm;
  if(intra_rank == 0){
  	MPI_Comm_split(MPI_COMM_WORLD, inter_color, myrank, &inter_comm);
        MPI_Comm_rank(inter_comm,&inter_rank);
	MPI_Comm_size(inter_comm,&inter_size);
    }
   else
	MPI_Comm_split(MPI_COMM_WORLD, MPI_UNDEFINED, myrank,&inter_comm);

  //MPI_Comm_rank(inter_comm,&inter_rank);
  //MPI_Comm_size(inter_comm,&inter_size);

  
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
  if(intra_rank==0)
  	MPI_Bcast(buf, count, MPI_DOUBLE, 0, inter_comm);
  MPI_Bcast(buf, count, MPI_DOUBLE, 0, intra_comm);
  double eTime = MPI_Wtime();

  double time = eTime - sTime;
  double max_time;

  MPI_Reduce (&time, &max_time, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

  // simple check
  printf ("bcast optmized %d %lf \n", myrank, eTime - sTime);

  if (myrank == 0){
          fprintf(fp,"%lf\n",max_time);
          fprintf(stdout,"%lf\n",max_time);
          *time_curr = max_time;
  }

  if(intra_rank==0)
        MPI_Comm_free(&inter_comm);
  MPI_Comm_free(&intra_comm);

  MPI_Finalize();
  free(buf);
}
void mpi_reduce_optimized(int D, double *time_curr){

  int myrank, size, length;
  double *buf, *recvBuf, *recvBuf2;
  int count = (D*KB)/sizeof(double);
  buf = (double *)malloc(D*KB);
  recvBuf = (double *) malloc(D*KB);
  recvBuf2 = (double *) malloc(D*KB);
  char name[MPI_MAX_PROCESSOR_NAME];
  FILE *fp;
  fp = fopen("data.tmp", "w");


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
  int inter_rank =-1, inter_size =-1;

  MPI_Comm inter_comm;
  if(intra_rank == 0){
    MPI_Comm_split(MPI_COMM_WORLD, inter_color, myrank, &inter_comm);
    MPI_Comm_rank(inter_comm,&inter_rank);
    MPI_Comm_size(inter_comm,&inter_size);
  }
  else
    MPI_Comm_split(MPI_COMM_WORLD, MPI_UNDEFINED, myrank,&inter_comm);

  
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
  if(intra_rank==0)
  	MPI_Reduce(recvBuf, recvBuf2, count, MPI_DOUBLE, MPI_MAX, 0, inter_comm);
  double eTime = MPI_Wtime();

  double time = eTime - sTime;
  double max_time;

  MPI_Reduce (&time, &max_time, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

  // simple check
  printf ("reduce optimized %d %lf \n", myrank, eTime - sTime);

  if (myrank == 0){
          fprintf(fp,"%lf\n",max_time);
          fprintf(stdout,"%lf\n",max_time);
          *time_curr = max_time;
  }

  if(intra_rank==0)
  	MPI_Comm_free(&inter_comm);
  MPI_Comm_free(&intra_comm);

  //Finalize
  MPI_Finalize();
  free(buf);
  free(recvBuf);
  free(recvBuf2);

}

void mpi_gather_optimized(int D, double *time_curr){

  int myrank, size, length;
  double *buf, *recvBuf, *recvBuf2;
  int count = (D*KB)/sizeof(double);
  buf = (double *)malloc(D*KB);
  char name[MPI_MAX_PROCESSOR_NAME];
  FILE *fp;
  fp = fopen("data.tmp", "w");


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

  recvBuf = (double *) malloc(D*KB*intra_size);

  // Creating an inter group communication picking the 0th ranked element in each intra group
  int inter_color = 0;
  int inter_rank, inter_size;

  MPI_Comm inter_comm;
  if(intra_rank == 0){
    MPI_Comm_split(MPI_COMM_WORLD, inter_color, myrank, &inter_comm);
    MPI_Comm_rank(inter_comm,&inter_rank);
    MPI_Comm_size(inter_comm,&inter_size);
  }
  else
    MPI_Comm_split(MPI_COMM_WORLD, MPI_UNDEFINED, myrank,&inter_comm);

  recvBuf2 = (double *) malloc(D*KB*size);

  double sTime = MPI_Wtime();
  MPI_Gather(buf, count, MPI_DOUBLE, recvBuf, count, MPI_DOUBLE, 0, intra_comm);
  if(intra_rank==0)
      MPI_Gather(recvBuf, count*intra_size, MPI_DOUBLE, recvBuf2, count*intra_size, MPI_DOUBLE, 0, inter_comm);
  double eTime = MPI_Wtime();

  double time = eTime - sTime;
  double max_time;

  MPI_Reduce (&time, &max_time, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

   // simple check
  printf ("gather optimized %d %lf \n", myrank, eTime - sTime);

  if (myrank == 0){
          fprintf(fp,"%lf\n",max_time);
          fprintf(stdout,"%lf\n",max_time);
          *time_curr = max_time;
  }

  if(intra_rank==0)
      MPI_Comm_free(&inter_comm);
  MPI_Comm_free(&intra_comm);
  // finalize
  MPI_Finalize();
  free(buf);
  free(recvBuf);

}

void mpi_alltoallv_optimized(int D, double *time_curr){

  int myrank, size, length, send_count,total_count;
  char name[MPI_MAX_PROCESSOR_NAME];
  FILE *fp;
  fp = fopen("data.tmp", "w");

  // Setup
  MPI_Init(NULL, NULL);
  MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

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
  if(intra_rank == 0){
    MPI_Comm_split(MPI_COMM_WORLD, inter_color, myrank, &inter_comm);
    MPI_Comm_rank(inter_comm,&inter_rank);
    MPI_Comm_size(inter_comm,&inter_size);
  }
  else
    MPI_Comm_split(MPI_COMM_WORLD, MPI_UNDEFINED, myrank,&inter_comm);
  
  double *buf = (double *)malloc(D*KB*size);
  double *recvBuf = (double *)malloc(D*KB*size);
  send_count = (D*KB)/sizeof(double);
  total_count=send_count*size;

  /* Load up the buffers */
  for (int i=0; i<total_count; i++) {
      buf[i] = i + 100*myrank;
      recvBuf[i] = -i;
  }

/*
  // Initialize buffer to random values
  srand(time(NULL));
  double high = 2021.0;
  for (int i=1; i<=count; i++)
     buf[i] = (high*(double)rand())/(double)RAND_MAX;
*/


  int *countBuf = (int *)malloc(size * sizeof(int));
  int *recvCountBuf = (int *)malloc(size * sizeof(int));
  int *recvDisplBuf = (int *)malloc(size * sizeof(int));
  int *displBuf = (int *)malloc(size * sizeof(int));

  for (int i=0; i<size; i++) {
      countBuf[i] = send_count;
      recvCountBuf[i] = send_count;
      recvDisplBuf[i] = i*send_count;
      displBuf[i] = i*send_count;
  }

  double sTime = MPI_Wtime();
  MPI_Alltoallv( buf, countBuf, displBuf, MPI_DOUBLE,
                       recvBuf, recvCountBuf, recvDisplBuf, MPI_DOUBLE, MPI_COMM_WORLD);
  double eTime = MPI_Wtime();

/* // Check recvBuf
    int *p;
    for (int i=0; i<size; i++) {
        p = recvBuf + recvDisplBuf[i];
        for (int j=0; j<myrank; j++) {
            if (p[j] != i * 100 + (myrank*(myrank+1))/2 + j) {
                printf("[%d] got %d expected %d for %dth\n",
                                    myrank, p[j],(i*(i+1))/2 + j, j);
            }else{
                printf("[%d] got %d expected %d for %dth\n",
                                    myrank, p[j],(i*(i+1))/2 + j, j);
            }
        }
    }
*/

  double time = eTime - sTime;
  double max_time;

  MPI_Reduce (&time, &max_time, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

  printf ("AlltoAllv optimized %d %lf \n", myrank, eTime - sTime);

  if (myrank == 0){
          fprintf(fp,"%lf\n",max_time);
          fprintf(stdout,"%lf\n",max_time);
          *time_curr = max_time;
  }

  if(intra_rank==0)
  	MPI_Comm_free(&inter_comm);
  MPI_Comm_free(&intra_comm);


  // Finalize
  MPI_Finalize();
  free(buf);
  free(recvBuf);
  free(countBuf);
  free(recvCountBuf);
  free(displBuf);
  free(recvDisplBuf);

}

int main( int argc, char *argv[])
{

    int D = atoi(argv[1]); // Data Size in KBs
    int option = atoi (argv[2]); // part to run
    int optimized = atoi(argv[3]); //Optimized == 1, Default = 0
    double time_curr = 0;

    if(!optimized){
	if (option == 1)
       		 mpi_bcast_default(D,&time_curr);
   	else if (option == 2)
       		 mpi_reduce_default(D,&time_curr);
	else if (option == 3)
                 mpi_gather_default(D,&time_curr);
	else if (option == 4)
                 mpi_alltoallv_default(D,&time_curr);
	else
		printf("Unsupported default option\n");
   }
   else{
       if (option == 1)
                 mpi_bcast_optimized(D,&time_curr);
       else if (option == 2)
                 mpi_reduce_optimized(D,&time_curr);
       else if (option == 3)
                 mpi_gather_optimized(D,&time_curr);
       else if (option == 4)
                 mpi_alltoallv_optimized(D,&time_curr);
       else
                printf("Unsupported optimized option\n");
  
    }

       return 0;
}

