// Main source file for Assignment1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "mpi.h"

// Part-1: Uses multiple sends and receives
void multiple_send_receive(int N, int num_of_steps){
    
    int  myrank, size;
    int  proc_row, proc_col; // Process coordinates in square decomposition
    int  topo_size; // Gives side length of square decomposition
    
    double start_time, time, max_time;
    
    double **old = (double **)malloc((N+2)* sizeof(double *)); // Data matrices--outer dim are recv buffers
    for (int i=0; i<N+2;i++)
       old[i] = (double*) malloc ((N+2)* sizeof(double));

    MPI_Init(NULL, NULL);
    MPI_Comm_rank( MPI_COMM_WORLD, &myrank );
    MPI_Comm_size( MPI_COMM_WORLD, &size );
    
    topo_size = sqrt((double)size);
    proc_row = myrank/topo_size;
    proc_col = myrank%topo_size;

    // Neighboring process ranks
    int left_rank = myrank-1, right_rank = myrank+1;
    int top_rank = myrank-topo_size, bottom_rank = myrank+topo_size;
 
    if (proc_row == 0)
        top_rank = MPI_PROC_NULL;
    else if (proc_row == topo_size-1)
        bottom_rank = MPI_PROC_NULL;
    
    if (proc_col == 0)
        left_rank = MPI_PROC_NULL;
    else if (proc_col == topo_size-1)
        right_rank = MPI_PROC_NULL;

    //printf("Rank:%d, %d -- (%d,%d)\n", myrank, topo_size, proc_row, proc_col);
    //printf("Rank:%d Left:%d Right:%d Top:%d Bottom:%d nrequest:%d\n", myrank, 
    //            left_rank, right_rank, top_rank, bottom_rank, nrequest);    
    for (int i=1; i<=N; i++)
        for (int j=1; j<=N; j++)
            old[i][j] = myrank+i+j-2; // TODO: Random initialization
    
    start_time = MPI_Wtime();

    for (int step=0; step<num_of_steps; step++){
        
        double **new = (double **)malloc((N+2)* sizeof(double *)); 
        for (int i=0; i<N+2;i++)
            new[i] = (double*) malloc ((N+2)* sizeof(double));
        
        MPI_Status status[8*N];
        MPI_Request request[8*N];

        // Receive elements
        // Receive from left
        for (int r=1; r<=N; r++)
            MPI_Irecv(&old[r][0], 1, MPI_DOUBLE, left_rank, r, MPI_COMM_WORLD, &request[r-1]);
        // Receive from right
        for (int r=1; r<=N; r++)
            MPI_Irecv(&old[r][N+1], 1, MPI_DOUBLE, right_rank, r, MPI_COMM_WORLD, &request[N+r-1]);
        // Receive from top
        for (int c=1; c<=N; c++)
            MPI_Irecv(&old[0][c], 1, MPI_DOUBLE, top_rank, c, MPI_COMM_WORLD, &request[2*N+c-1]);
        // Receive from bottom
        for (int c=1; c<=N; c++)
            MPI_Irecv(&old[N+1][c], 1, MPI_DOUBLE, bottom_rank, c, MPI_COMM_WORLD, &request[3*N+c-1]);
        
        // Send elements
        // Send to left
        for (int r=1; r<=N; r++)
            MPI_Isend(&old[r][1], 1, MPI_DOUBLE, left_rank, r, MPI_COMM_WORLD, &request[4*N+r-1]);
        // Send to right
        for (int r=1; r<=N; r++)
            MPI_Isend(&old[r][N], 1, MPI_DOUBLE, right_rank, r, MPI_COMM_WORLD, &request[5*N+r-1]);
        // Send to top
        for (int c=1; c<=N; c++)
            MPI_Isend(&old[1][c], 1, MPI_DOUBLE, top_rank, c, MPI_COMM_WORLD, &request[6*N+c-1]);
        // Send to bottom
        for (int c=1; c<=N; c++)
            MPI_Isend(&old[N][c], 1, MPI_DOUBLE, bottom_rank, c, MPI_COMM_WORLD, &request[7*N+c-1]);

        // Stencil computation for non-halo region
        for (int r=2; r<=N-1; r++)
           for (int c=2; c<=N-1; c++)
                new[r][c] = (old[r-1][c] + old[r+1][c] + old[r][c-1] + old[r][c+1])/4;
        
        MPI_Waitall(8*N, request, status); 

        //TODO:Stencil computation for halo regio (separate computation for corner processes)
        /*if (proc_row==0 && proc_col==0){
          int r=1; //First row
          for (int c=1; c<=N; c++){
            if (c==1)
                new[r][c] = (old[r+1][c] + old[r][c+1])/2;
            else
                new[r][c] = (old[r+1][c] + old[r][c+1] + old[r][c-1])/3;
          }

          r=N; //Last row
          for (int c=1; c<=N; c++){
            if (c==1)
                new[r][c] = (old[r+1][c] + old[r][c+1] + old[r-1][c])/3;
            else
                new[r][c] = (old[r-1][c] + old[r+1][c] + old[r][c-1] + old[r][c+1])/4;
          }

        int ncount = 4, r=1, c=1;


        }*/
        //memcpy (old, new, (N+2)*(N+2)*sizeof(double));i
        free(new);
    }

    time = MPI_Wtime() - start_time;
    MPI_Reduce (&time, &max_time, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

    if (myrank == 0) printf ("Max time = %lf\n", max_time);

    MPI_Finalize();

    /*printf("Process:%d\n", myrank);
    for (int i=0; i<N+2; i++){
        for (int j=0; j<N+2; j++)
            printf("%lf ", old[i][j]);
        printf("\n");    
    }*/
    free(old);
}

// Part-2: Uses pack/unpack and send/receives
void pack_send_unpack_receive(int N, int num_of_steps){

    int  myrank, size;
    int  proc_row, proc_col; // Process coordinates in square decomposition
    int  topo_size; // Gives side length of square decomposition

    double start_time, time, max_time;

    double **old = (double **)malloc((N+2)* sizeof(double *)); // Data matrices--outer dim are recv buffers
    for (int i=0; i<N+2;i++)
       old[i] = (double*) malloc ((N+2)* sizeof(double));

    MPI_Init(NULL, NULL);
    MPI_Comm_rank( MPI_COMM_WORLD, &myrank );
    MPI_Comm_size( MPI_COMM_WORLD, &size );

    topo_size = sqrt((double)size);
    proc_row = myrank/topo_size;
    proc_col = myrank%topo_size;

    // Neighboring process ranks
    int left_rank = myrank-1, right_rank = myrank+1;
    int top_rank = myrank-topo_size, bottom_rank = myrank+topo_size;

    if (proc_row == 0)
        top_rank = MPI_PROC_NULL;
    else if (proc_row == topo_size-1)
        bottom_rank = MPI_PROC_NULL;

    if (proc_col == 0)
        left_rank = MPI_PROC_NULL;
    else if (proc_col == topo_size-1)
        right_rank = MPI_PROC_NULL;

    /*printf("Rank:%d, %d -- (%d,%d)\n", myrank, topo_size, proc_row, proc_col);
    printf("Rank:%d Left:%d Right:%d Top:%d Bottom:%d\n", myrank, 
               left_rank, right_rank, top_rank, bottom_rank);*/    
    for (int i=1; i<=N; i++)
        for (int j=1; j<=N; j++)
            old[i][j] = myrank+i+j-2; // TODO: Random initialization

    start_time = MPI_Wtime();

    for (int step=0; step<num_of_steps; step++){

        double **new = (double **)malloc((N+2)* sizeof(double *));
        for (int i=0; i<N+2;i++)
            new[i] = (double*) malloc ((N+2)* sizeof(double));

        MPI_Status status[8];
        MPI_Request request[8];

        // Receive elements
        double lrecv[N], rrecv[N], trecv[N], brecv[N];
        int nreq=0;
        // Receive from left
        MPI_Irecv(lrecv, N, MPI_PACKED, left_rank, MPI_ANY_TAG, MPI_COMM_WORLD, &request[nreq++]);
        // Receive from right
        MPI_Irecv(rrecv, N, MPI_PACKED, right_rank, MPI_ANY_TAG, MPI_COMM_WORLD, &request[nreq++]);
        // Receive from top
        MPI_Irecv(trecv, N, MPI_PACKED, top_rank, MPI_ANY_TAG, MPI_COMM_WORLD, &request[nreq++]);
        // Receive from bottom
        MPI_Irecv(brecv, N, MPI_PACKED, bottom_rank, MPI_ANY_TAG, MPI_COMM_WORLD, &request[nreq++]);

        // Send elements
        double lbuf[N], rbuf[N], tbuf[N], bbuf[N];
        int lpos=0, rpos=0, tpos=0, bpos=0;

        // Send to left
        for (int r=1; r<=N; r++)
            MPI_Pack(&old[r][1], 1, MPI_DOUBLE, lbuf, sizeof(double)*N, &lpos, MPI_COMM_WORLD);
        MPI_Isend(lbuf, lpos, MPI_PACKED, left_rank, 0, MPI_COMM_WORLD, &request[nreq++]);

        // Send to right
        for (int r=1; r<=N; r++)
            MPI_Pack(&old[r][N], 1, MPI_DOUBLE, rbuf, sizeof(double)*N, &rpos, MPI_COMM_WORLD);
        MPI_Isend(rbuf, rpos, MPI_PACKED, right_rank, 0, MPI_COMM_WORLD, &request[nreq++]);

        // Send to top
        for (int c=1; c<=N; c++)
            MPI_Pack(&old[1][c], 1, MPI_DOUBLE, tbuf, sizeof(double)*N, &tpos, MPI_COMM_WORLD);
        MPI_Isend(tbuf, tpos, MPI_PACKED, top_rank, 0, MPI_COMM_WORLD, &request[nreq++]);
    
        // Send to bottom
        for (int c=1; c<=N; c++)
            MPI_Pack(&old[N][c], 1, MPI_DOUBLE, bbuf, sizeof(double)*N, &bpos, MPI_COMM_WORLD);
        MPI_Isend(bbuf, bpos, MPI_PACKED, bottom_rank, 0, MPI_COMM_WORLD, &request[nreq++]);

        // Stencil computation for non-halo region
        //for (int r=2; r<=N-1; r++)
        //   for (int c=2; c<=N-1; c++)
        //        new[r][c] = (old[r-1][c] + old[r+1][c] + old[r][c-1] + old[r][c+1])/4;
        
        MPI_Waitall(nreq, request, status);
        //for (int i=0; i<nreq; i++)
        //   printf("%d\n", status[i].MPI_ERROR); 
        //lpos=0, rpos=0, tpos=0, bpos=0; //TODO: It is okay to use these variables now?
        
        // Unpack elements
        // left
        //for (int r=1; r<=N; r++)
        //    MPI_Unpack(lrecv, sizeof(double)*N, &lpos, &old[r][0], 1, MPI_DOUBLE, MPI_COMM_WORLD);
        // right
        /*for (int r=1; r<=N; r++)
            MPI_Unpack(rrecv, sizeof(double)*N, &rpos, &old[r][N+1], 1, MPI_DOUBLE, MPI_COMM_WORLD);
        // top
        for (int c=1; c<=N; c++)
            MPI_Unpack(trecv, sizeof(double)*N, &tpos, &old[0][c], 1, MPI_DOUBLE, MPI_COMM_WORLD);
        // bottom
        for (int c=1; c<=N; c++)
            MPI_Unpack(brecv, sizeof(double)*N, &bpos, &old[N+1][c], 1, MPI_DOUBLE, MPI_COMM_WORLD);*/

        //TODO:Stencil computation for halo regio (separate computation for corner processes)
        


      //  free(new);
    }

    time = MPI_Wtime() - start_time;
//    MPI_Reduce (&time, &max_time, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

//    if (myrank == 0) printf ("Max time = %lf\n", max_time);

    MPI_Finalize();

    /*printf("Process:%d\n", myrank);
    for (int i=0; i<N+2; i++){
        for (int j=0; j<N+2; j++)
            printf("%lf ", old[i][j]);
        printf("\n");    
    }*/
   free(old);
}

// Part-3; Uses derived datatypes
void derived_datatype_send_receive(int N, int num_of_steps){

}


int main( int argc, char *argv[])
{

    int N = atoi(argv[1]); // square root of data points per process (size of matrix row/col)
    //int P = atoi (argv[2]);  TODO: do we need this? can be inferred from size
    int num_of_steps = atoi (argv[2]);
    int option = atoi (argv[3]); // part to run
    
    if (option == 1)
        multiple_send_receive(N, num_of_steps);
    else if (option == 2)
        pack_send_unpack_receive(N,num_of_steps);
    else if (option == 3)
        derived_datatype_send_receive(N,num_of_steps);
    else{
        multiple_send_receive(N, num_of_steps);
        pack_send_unpack_receive(N,num_of_steps);
        derived_datatype_send_receive(N,num_of_steps);
    }

    return 0;
}
