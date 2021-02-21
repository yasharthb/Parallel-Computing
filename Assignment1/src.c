// Main source file for Assignment1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include "mpi.h"

// Part-1: Uses multiple sends and receives
void multiple_send_receive(int N, int num_of_steps){

    int  myrank, size;
    int  proc_row, proc_col; // Process coordinates in square decomposition
    int  topo_size; // Gives side length of square decomposition
    double start_time, etime, max_time;

    double **old = malloc((N+2)*sizeof(double *)); // Data matrices--outer dim are recv buffers
    old[0] = malloc((N+2)*(N+2)*sizeof(double));
    for (int i=1; i<(N+2);i++)
       old[i] = old[i-1]+(N+2);

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
    if (proc_row == topo_size-1)
        bottom_rank = MPI_PROC_NULL;

    if (proc_col == 0)
        left_rank = MPI_PROC_NULL;
    if (proc_col == topo_size-1)
        right_rank = MPI_PROC_NULL;

    /*printf("Rank:%d, %d -- (%d,%d)\n", myrank, topo_size, proc_row, proc_col);
    printf("Rank:%d Left:%d Right:%d Top:%d Bottom:%d \n", myrank,
                left_rank, right_rank, top_rank, bottom_rank);*/
    srand(time(NULL));
    double high = 2021.0, low = -2021.0;
    for (int i=1; i<=N; i++)
        for (int j=1; j<=N; j++)
            old[i][j] = ((high-low)*(double)rand())/(double)RAND_MAX + low;

    start_time = MPI_Wtime();

    for (int step=0; step<num_of_steps; step++){

        double **new = malloc((N+2)*sizeof(double *)); // Data matrices--outer dim are recv buffers
        new[0] = malloc((N+2)*(N+2)*sizeof(double));
        for (int i=1; i<(N+2);i++)
          new[i] = new[i-1]+(N+2);

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
                new[r][c] = (old[r-1][c] + old[r+1][c] + old[r][c-1] + old[r][c+1])/4.0;

        MPI_Waitall(8*N, request, status);

        //Stencil computation for halo regio (separate computation for corner processes)
        if(proc_row == 0 || proc_row == topo_size-1 || proc_col == 0 || proc_col == topo_size-1){

            // Top Left Process
            if(proc_row ==0 && proc_col == 0){
                int r=1; // First row
                for (int c=1; c<=N; c++){
                    if (c==1)
                        new[r][c] = (old[r+1][c] + old[r][c+1])/2.0;
                    else
                        new[r][c] = (old[r+1][c] + old[r][c+1] + old[r][c-1])/3.0;
                }
                r=N; //Last row
                for (int c=1; c<=N; c++){
                    if (c==1)
                        new[r][c] = (old[r+1][c] + old[r][c+1] + old[r-1][c])/3.0;
                    else
                        new[r][c] = (old[r-1][c] + old[r+1][c] + old[r][c-1] + old[r][c+1])/4.0;
                }
                int c=1; // First column
                for (int r=2; r<N; r++)
                    new[r][c] = (old[r-1][c] + old[r][c+1] + old[r+1][c])/3.0;

                c=N; //Last column
                for (int r=2; r<N; r++)
                        new[r][c] = (old[r-1][c] + old[r+1][c] + old[r][c-1] + old[r][c+1])/4.0;
            }

            //Bottom Left Process
            if(proc_row ==topo_size-1 && proc_col == 0){
                int r=N; // Last row
                for (int c=1; c<=N; c++){
                    if (c==1)
                        new[r][c] = (old[r-1][c] + old[r][c+1])/2.0;
                    else
                        new[r][c] = (old[r-1][c] + old[r][c+1] + old[r][c-1])/3.0;
                }
                r=1; //First row
                for (int c=1; c<=N; c++){
                    if (c==1)
                        new[r][c] = (old[r+1][c] + old[r][c+1] + old[r-1][c])/3.0;
                    else
                        new[r][c] = (old[r-1][c] + old[r+1][c] + old[r][c-1] + old[r][c+1])/4.0;
                }
                int c=1; // First column
                for (int r=2; r<N; r++)
                    new[r][c] = (old[r-1][c] + old[r][c+1] + old[r+1][c])/3.0;

                c=N; //Last column
                for (int r=2; r<N; r++)
                        new[r][c] = (old[r-1][c] + old[r+1][c] + old[r][c-1] + old[r][c+1])/4.0;
            }

            // Bottom Right Process
            if(proc_row ==topo_size-1 && proc_col == topo_size-1){
                int r=N; // Last row
                for (int c=1; c<=N; c++){
                    if (c==N)
                        new[r][c] = (old[r-1][c] + old[r][c-1])/2.0;
                    else{
                        new[r][c] = (old[r-1][c] + old[r][c-1] + old[r][c+1])/3.0;
                    }
                }
                r=1; //First row
                for (int c=1; c<=N; c++){
                    if (c==N)
                        new[r][c] = (old[r+1][c] + old[r][c-1] + old[r-1][c])/3.0;
                    else
                        new[r][c] = (old[r-1][c] + old[r+1][c] + old[r][c-1] + old[r][c+1])/4.0;
                }
                int c=N; // Last column
                for (int r=2; r<N; r++)
                    new[r][c] = (old[r-1][c] + old[r][c-1] + old[r+1][c])/3.0;

                c=1; //First column
                for (int r=2; r<N; r++)
                        new[r][c] = (old[r-1][c] + old[r+1][c] + old[r][c-1] + old[r][c+1])/4.0;
            }

            //Top Right Process
            if(proc_row == 0 && proc_col == topo_size-1){
                int r=1; // First row
                for (int c=1; c<=N; c++){
                    if (c==N)
                        new[r][c] = (old[r+1][c] + old[r][c-1])/2.0;
                    else
                        new[r][c] = (old[r+1][c] + old[r][c+1] + old[r][c-1])/3.0;
                }
                r=N; //Last row
                for (int c=1; c<=N; c++){
                    if (c==N)
                        new[r][c] = (old[r+1][c] + old[r][c-1] + old[r-1][c])/3.0;
                    else
                        new[r][c] = (old[r-1][c] + old[r+1][c] + old[r][c-1] + old[r][c+1])/4.0;
                }
                int c=N; // Last Column
                for (int r=2; r<N; r++)
                    new[r][c] = (old[r-1][c] + old[r][c-1] + old[r+1][c])/3.0;

                c=1; //First column
                for (int r=2; r<N; r++)
                    new[r][c] = (old[r-1][c] + old[r+1][c] + old[r][c-1] + old[r][c+1])/4.0;
            }

            // Top Mid Process
            if(proc_row ==0 && proc_col != 0 && proc_col !=topo_size-1){
                int r=1; // First row
                for (int c=1; c<=N; c++)
                     new[r][c] = (old[r+1][c] + old[r][c+1] + old[r][c-1])/3.0;

                r=N; //Last row
                for (int c=1; c<=N; c++)
                    new[r][c] = (old[r-1][c] + old[r+1][c] + old[r][c-1] + old[r][c+1])/4.0;

                int c=1; // First column
                for (int r=2; r<N; r++)
                    new[r][c] = (old[r-1][c] + old[r+1][c] + old[r][c-1] + old[r][c+1])/4.0;

                c=N; //Last column
                for (int r=2; r<N; r++)
                    new[r][c] = (old[r-1][c] + old[r+1][c] + old[r][c-1] + old[r][c+1])/4.0;
            }

            //Bottom Mid Process
            if(proc_row ==topo_size-1 && proc_col != 0 && proc_col !=topo_size-1){

                int r=N; // Last row
                for (int c=1; c<=N; c++)
                    new[r][c] = (old[r-1][c] + old[r][c+1] + old[r][c-1])/3.0;

                r=1; //First row
                for (int c=1; c<=N; c++)
                    new[r][c] = (old[r-1][c] + old[r+1][c] + old[r][c-1] + old[r][c+1])/4.0;

                int c=1; // First column
                for (int r=2; r<N; r++)
                    new[r][c] = (old[r-1][c] + old[r+1][c] + old[r][c-1] + old[r][c+1])/4.0;

                c=N; //Last column
                for (int r=2; r<N; r++)
                    new[r][c] = (old[r-1][c] + old[r+1][c] + old[r][c-1] + old[r][c+1])/4.0;
            }

            //Left Mid Process
            if(proc_row !=topo_size-1 && proc_row!=0 && proc_col == 0){
                int r=N; // Last row
                for (int c=1; c<=N; c++){
                    if (c==1)
                        new[r][c] = (old[r-1][c] + old[r][c+1] + old[r+1][c])/3.0; // ?r+1,c in last term
                    else
                        new[r][c] = (old[r-1][c] + old[r+1][c] + old[r][c-1] + old[r][c+1])/4.0;
                }
                r=1; //First row
                for (int c=1; c<=N; c++){
                    if (c==1)
                        new[r][c] = (old[r+1][c] + old[r][c+1] + old[r-1][c])/3.0;
                    else
                        new[r][c] = (old[r-1][c] + old[r+1][c] + old[r][c-1] + old[r][c+1])/4.0;
                }
                int c=1; // First column
                for (int r=2; r<N; r++)
                    new[r][c] = (old[r-1][c] + old[r][c+1] + old[r+1][c])/3.0;

                c=N; //Last column
                for (int r=2; r<N; r++)
                    new[r][c] = (old[r-1][c] + old[r+1][c] + old[r][c-1] + old[r][c+1])/4.0;
            }

            //Right Mid Process
            if(proc_row != topo_size-1 && proc_row!=0 && proc_col == topo_size-1){
                int r=1; // First row
                for (int c=1; c<=N; c++){
                    if (c==N)
                        new[r][c] = (old[r+1][c] + old[r-1][c] + old[r][c-1])/3.0; //
                    else
                        new[r][c] = (old[r-1][c] + old[r+1][c] + old[r][c-1] + old[r][c+1])/4.0;
                }
                r=N; //Last row
                for (int c=1; c<=N; c++){
                    if (c==N)
                        new[r][c] = (old[r+1][c] + old[r][c-1] + old[r-1][c])/3.0;
                    else
                        new[r][c] = (old[r-1][c] + old[r+1][c] + old[r][c-1] + old[r][c+1])/4.0;
                }
                int c=N; // Last Column
                for (int r=2; r<N; r++)
                    new[r][c] = (old[r-1][c] + old[r][c-1] + old[r+1][c])/3.0;

                c=1; //First column
                for (int r=2; r<N; r++)
                    new[r][c] = (old[r-1][c] + old[r+1][c] + old[r][c-1] + old[r][c+1])/4.0;
            }
        }
        else{
            //Inner processes
            int r=1;
            for (int c=1; c<=N; c++)
                new[r][c] = (old[r-1][c] + old[r+1][c] + old[r][c-1] + old[r][c+1])/4.0;
            r=N;
            for (int c=1; c<=N; c++)
                new[r][c] = (old[r-1][c] + old[r+1][c] + old[r][c-1] + old[r][c+1])/4.0;

            int c=N; // Last Column
            for (int r=2; r<N; r++)
                new[r][c] = (old[r-1][c] + old[r+1][c] + old[r][c-1] + old[r+1][c])/4.0;

            c=1; //First column
            for (int r=2; r<N; r++)
                new[r][c] = (old[r-1][c] + old[r+1][c] + old[r][c-1] + old[r][c+1])/4.0;
        }

        memcpy (old, new, (N+2)*(N+2)*sizeof(double));
        free(new);
    }

    etime = MPI_Wtime() - start_time;
    MPI_Reduce (&etime, &max_time, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

    if (myrank == 0) printf ("P:%d N:%d Max_time:%lf\n", size, N, max_time);

    MPI_Finalize();

    /*printf("Process:%d\n", myrank);
    for (int i=0; i<N+2; i++){
        for (int j=0; j<N+2; j++)
            printf("%lf ", old[i][j]);
        printf("\n");
    }*/
    free(old[0]);
    free(old);
}

// Part-2: Uses pack/unpack and send/receives
void pack_send_unpack_receive(int N, int num_of_steps){

    int  myrank, size, pack_size;
    int  proc_row, proc_col; // Process coordinates in square decomposition
    int  topo_size; // Gives side length of square decomposition

    double start_time, etime, max_time;

    double **old = malloc((N+2)*sizeof(double *)); // Data matrices--outer dim are recv buffers
    old[0] = malloc((N+2)*(N+2)*sizeof(double));
    for (int i=1; i<(N+2);i++)
       old[i] = old[i-1]+(N+2);

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
    if (proc_row == topo_size-1)
        bottom_rank = MPI_PROC_NULL;

    if (proc_col == 0)
        left_rank = MPI_PROC_NULL;
    if (proc_col == topo_size-1)
        right_rank = MPI_PROC_NULL;

    /*printf("Rank:%d, %d -- (%d,%d)\n", myrank, topo_size, proc_row, proc_col);
    printf("Rank:%d Left:%d Right:%d Top:%d Bottom:%d\n", myrank,
               left_rank, right_rank, top_rank, bottom_rank);*/
    srand(time(NULL));
    double high = 2021.0, low = -2021.0;
    for (int i=1; i<=N; i++)
        for (int j=1; j<=N; j++)
            old[i][j] = ((high-low)*(double)rand())/(double)RAND_MAX + low;

    start_time = MPI_Wtime();

    for (int step=0; step<num_of_steps; step++){

        double **new = malloc((N+2)*sizeof(double *)); // Data matrices--outer dim are recv buffers
        new[0] = malloc((N+2)*(N+2)*sizeof(double));
        for (int i=1; i<(N+2);i++)
          new[i] = new[i-1]+(N+2);

        MPI_Status status[8];
        MPI_Request request[8];

        // Receive elements
        double lrecv[N], rrecv[N], trecv[N], brecv[N];
        int nreq=0;
        // Receive from left
        MPI_Irecv(lrecv, sizeof(double)*N, MPI_PACKED, left_rank, MPI_ANY_TAG, MPI_COMM_WORLD, &request[nreq++]);
        // Receive from right
        MPI_Irecv(rrecv, sizeof(double)*N, MPI_PACKED, right_rank, MPI_ANY_TAG, MPI_COMM_WORLD, &request[nreq++]);
        // Receive from top
        MPI_Irecv(trecv, sizeof(double)*N, MPI_PACKED, top_rank, MPI_ANY_TAG, MPI_COMM_WORLD, &request[nreq++]);
        // Receive from bottom
        MPI_Irecv(brecv, sizeof(double)*N, MPI_PACKED, bottom_rank, MPI_ANY_TAG, MPI_COMM_WORLD, &request[nreq++]);

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
        for (int r=2; r<=N-1; r++)
           for (int c=2; c<=N-1; c++)
                new[r][c] = (old[r-1][c] + old[r+1][c] + old[r][c-1] + old[r][c+1])/4.0;

        MPI_Waitall(nreq, request, status);

        lpos=0, rpos=0, tpos=0, bpos=0;

        // Unpack elements
        // left
        for (int r=1; r<=N; r++)
            MPI_Unpack(lrecv, sizeof(double)*N, &lpos, &old[r][0], 1, MPI_DOUBLE, MPI_COMM_WORLD);
        // right
        for (int r=1; r<=N; r++)
            MPI_Unpack(rrecv, sizeof(double)*N, &rpos, &old[r][N+1], 1, MPI_DOUBLE, MPI_COMM_WORLD);
        // top
        for (int c=1; c<=N; c++)
            MPI_Unpack(trecv, sizeof(double)*N, &tpos, &old[0][c], 1, MPI_DOUBLE, MPI_COMM_WORLD);
        // bottom
        for (int c=1; c<=N; c++)
            MPI_Unpack(brecv, sizeof(double)*N, &bpos, &old[N+1][c], 1, MPI_DOUBLE, MPI_COMM_WORLD);

        //Stencil computation for halo regio (separate computation for corner processes)
        if(proc_row == 0 || proc_row == topo_size-1 || proc_col == 0 || proc_col == topo_size-1){

            // Top Left Process
            if(proc_row ==0 && proc_col == 0){
                int r=1; // First row
                for (int c=1; c<=N; c++){
                    if (c==1)
                        new[r][c] = (old[r+1][c] + old[r][c+1])/2.0;
                    else
                        new[r][c] = (old[r+1][c] + old[r][c+1] + old[r][c-1])/3.0;
                }
                r=N; //Last row
                for (int c=1; c<=N; c++){
                    if (c==1)
                        new[r][c] = (old[r+1][c] + old[r][c+1] + old[r-1][c])/3.0;
                    else
                        new[r][c] = (old[r-1][c] + old[r+1][c] + old[r][c-1] + old[r][c+1])/4.0;
                }
                int c=1; // First column
                for (int r=2; r<N; r++)
                    new[r][c] = (old[r-1][c] + old[r][c+1] + old[r+1][c])/3.0;

                c=N; //Last column
                for (int r=2; r<N; r++)
                        new[r][c] = (old[r-1][c] + old[r+1][c] + old[r][c-1] + old[r][c+1])/4.0;
            }

            //Bottom Left Process
            if(proc_row ==topo_size-1 && proc_col == 0){
                int r=N; // Last row
                for (int c=1; c<=N; c++){
                    if (c==1)
                        new[r][c] = (old[r-1][c] + old[r][c+1])/2.0;
                    else
                        new[r][c] = (old[r-1][c] + old[r][c+1] + old[r][c-1])/3.0;
                }
                r=1; //First row
                for (int c=1; c<=N; c++){
                    if (c==1)
                        new[r][c] = (old[r+1][c] + old[r][c+1] + old[r-1][c])/3.0;
                    else
                        new[r][c] = (old[r-1][c] + old[r+1][c] + old[r][c-1] + old[r][c+1])/4.0;
                }
                int c=1; // First column
                for (int r=2; r<N; r++)
                    new[r][c] = (old[r-1][c] + old[r][c+1] + old[r+1][c])/3.0;

                c=N; //Last column
                for (int r=2; r<N; r++)
                        new[r][c] = (old[r-1][c] + old[r+1][c] + old[r][c-1] + old[r][c+1])/4.0;
            }

            // Bottom Right Process
            if(proc_row ==topo_size-1 && proc_col == topo_size-1){
                int r=N; // Last row
                for (int c=1; c<=N; c++){
                    if (c==N)
                        new[r][c] = (old[r-1][c] + old[r][c-1])/2.0;
                    else{
                        new[r][c] = (old[r-1][c] + old[r][c-1] + old[r][c+1])/3.0;
                    }
                }
                r=1; //First row
                for (int c=1; c<=N; c++){
                    if (c==N)
                        new[r][c] = (old[r+1][c] + old[r][c-1] + old[r-1][c])/3.0;
                    else
                        new[r][c] = (old[r-1][c] + old[r+1][c] + old[r][c-1] + old[r][c+1])/4.0;
                }
                int c=N; // Last column
                for (int r=2; r<N; r++)
                    new[r][c] = (old[r-1][c] + old[r][c-1] + old[r+1][c])/3.0;

                c=1; //First column
                for (int r=2; r<N; r++)
                        new[r][c] = (old[r-1][c] + old[r+1][c] + old[r][c-1] + old[r][c+1])/4.0;
            }

            //Top Right Process
            if(proc_row == 0 && proc_col == topo_size-1){
                int r=1; // First row
                for (int c=1; c<=N; c++){
                    if (c==N)
                        new[r][c] = (old[r+1][c] + old[r][c-1])/2.0;
                    else
                        new[r][c] = (old[r+1][c] + old[r][c+1] + old[r][c-1])/3.0;
                }
                r=N; //Last row
                for (int c=1; c<=N; c++){
                    if (c==N)
                        new[r][c] = (old[r+1][c] + old[r][c-1] + old[r-1][c])/3.0;
                    else
                        new[r][c] = (old[r-1][c] + old[r+1][c] + old[r][c-1] + old[r][c+1])/4.0;
                }
                int c=N; // Last Column
                for (int r=2; r<N; r++)
                    new[r][c] = (old[r-1][c] + old[r][c-1] + old[r+1][c])/3.0;

                c=1; //First column
                for (int r=2; r<N; r++)
                    new[r][c] = (old[r-1][c] + old[r+1][c] + old[r][c-1] + old[r][c+1])/4.0;
            }

            // Top Mid Process
            if(proc_row ==0 && proc_col != 0 && proc_col !=topo_size-1){
                int r=1; // First row
                for (int c=1; c<=N; c++)
                     new[r][c] = (old[r+1][c] + old[r][c+1] + old[r][c-1])/3.0;

                r=N; //Last row
                for (int c=1; c<=N; c++)
                    new[r][c] = (old[r-1][c] + old[r+1][c] + old[r][c-1] + old[r][c+1])/4.0;

                int c=1; // First column
                for (int r=2; r<N; r++)
                    new[r][c] = (old[r-1][c] + old[r+1][c] + old[r][c-1] + old[r][c+1])/4.0;

                c=N; //Last column
                for (int r=2; r<N; r++)
                    new[r][c] = (old[r-1][c] + old[r+1][c] + old[r][c-1] + old[r][c+1])/4.0;
            }

            //Bottom Mid Process
            if(proc_row ==topo_size-1 && proc_col != 0 && proc_col !=topo_size-1){

                int r=N; // Last row
                for (int c=1; c<=N; c++)
                    new[r][c] = (old[r-1][c] + old[r][c+1] + old[r][c-1])/3.0;

                r=1; //First row
                for (int c=1; c<=N; c++)
                    new[r][c] = (old[r-1][c] + old[r+1][c] + old[r][c-1] + old[r][c+1])/4.0;

                int c=1; // First column
                for (int r=2; r<N; r++)
                    new[r][c] = (old[r-1][c] + old[r+1][c] + old[r][c-1] + old[r][c+1])/4.0;

                c=N; //Last column
                for (int r=2; r<N; r++)
                    new[r][c] = (old[r-1][c] + old[r+1][c] + old[r][c-1] + old[r][c+1])/4.0;
            }

            //Left Mid Process
            if(proc_row !=topo_size-1 && proc_row!=0 && proc_col == 0){
                int r=N; // Last row
                for (int c=1; c<=N; c++){
                    if (c==1)
                        new[r][c] = (old[r-1][c] + old[r][c+1] + old[r+1][c])/3.0; // ?r+1,c in last term
                    else
                        new[r][c] = (old[r-1][c] + old[r+1][c] + old[r][c-1] + old[r][c+1])/4.0;
                }
                r=1; //First row
                for (int c=1; c<=N; c++){
                    if (c==1)
                        new[r][c] = (old[r+1][c] + old[r][c+1] + old[r-1][c])/3.0;
                    else
                        new[r][c] = (old[r-1][c] + old[r+1][c] + old[r][c-1] + old[r][c+1])/4.0;
                }
                int c=1; // First column
                for (int r=2; r<N; r++)
                    new[r][c] = (old[r-1][c] + old[r][c+1] + old[r+1][c])/3.0;

                c=N; //Last column
                for (int r=2; r<N; r++)
                    new[r][c] = (old[r-1][c] + old[r+1][c] + old[r][c-1] + old[r][c+1])/4.0;
            }

            //Right Mid Process
            if(proc_row != topo_size-1 && proc_row!=0 && proc_col == topo_size-1){
                int r=1; // First row
                for (int c=1; c<=N; c++){
                    if (c==N)
                        new[r][c] = (old[r+1][c] + old[r-1][c] + old[r][c-1])/3.0; //
                    else
                        new[r][c] = (old[r-1][c] + old[r+1][c] + old[r][c-1] + old[r][c+1])/4.0;
                }
                r=N; //Last row
                for (int c=1; c<=N; c++){
                    if (c==N)
                        new[r][c] = (old[r+1][c] + old[r][c-1] + old[r-1][c])/3.0;
                    else
                        new[r][c] = (old[r-1][c] + old[r+1][c] + old[r][c-1] + old[r][c+1])/4.0;
                }
                int c=N; // Last Column
                for (int r=2; r<N; r++)
                    new[r][c] = (old[r-1][c] + old[r][c-1] + old[r+1][c])/3.0;

                c=1; //First column
                for (int r=2; r<N; r++)
                    new[r][c] = (old[r-1][c] + old[r+1][c] + old[r][c-1] + old[r][c+1])/4.0;
            }
        }
        else{
            //Inner processes
            int r=1;
            for (int c=1; c<=N; c++)
                new[r][c] = (old[r-1][c] + old[r+1][c] + old[r][c-1] + old[r][c+1])/4.0;
            r=N;
            for (int c=1; c<=N; c++)
                new[r][c] = (old[r-1][c] + old[r+1][c] + old[r][c-1] + old[r][c+1])/4.0;

            int c=N; // Last Column
            for (int r=2; r<N; r++)
                new[r][c] = (old[r-1][c] + old[r+1][c] + old[r][c-1] + old[r+1][c])/4.0;

            c=1; //First column
            for (int r=2; r<N; r++)
                new[r][c] = (old[r-1][c] + old[r+1][c] + old[r][c-1] + old[r][c+1])/4.0;
        }

        memcpy (old, new, (N+2)*(N+2)*sizeof(double));
        free(new);
    }

    etime = MPI_Wtime() - start_time;
    MPI_Reduce (&etime, &max_time, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

    if (myrank == 0) printf ("P:%d N:%d Max_time:%lf\n", size, N, max_time);

    MPI_Finalize();

    /*printf("Process:%d\n", myrank);
    for (int i=0; i<N+2; i++){
        for (int j=0; j<N+2; j++)
            printf("%lf ", old[i][j]);
        printf("\n");
    }*/
    free(old[0]);
    free(old);
}

// Part-3; Uses derived datatypes
void derived_datatype_send_receive(int N, int num_of_steps){
    int  myrank, size;
    int  proc_row, proc_col; // Process coordinates in square decomposition
    int  topo_size; // Gives side length of square decomposition

    double start_time, etime, max_time;

    double **old = malloc((N+2)*sizeof(double *)); // Data matrices--outer dim are recv buffers
    old[0] = malloc((N+2)*(N+2)*sizeof(double));
    for (int i=1; i<(N+2);i++)
       old[i] = old[i-1]+(N+2);

    MPI_Init(NULL, NULL);
    MPI_Comm_rank( MPI_COMM_WORLD, &myrank );
    MPI_Comm_size( MPI_COMM_WORLD, &size );

    topo_size = sqrt((double)size);
    proc_row = myrank/topo_size;
    proc_col = myrank%topo_size;

    MPI_Datatype rowVector,columnVector;
    MPI_Type_vector(N, 1, 1, MPI_DOUBLE, &rowVector);
    MPI_Type_commit(&rowVector);

    MPI_Type_vector(N, 1, N+2, MPI_DOUBLE, &columnVector);
    MPI_Type_commit(&columnVector);

    // Neighboring process ranks
    int left_rank = myrank-1, right_rank = myrank+1;
    int top_rank = myrank-topo_size, bottom_rank = myrank+topo_size;

    if (proc_row == 0)
        top_rank = MPI_PROC_NULL;
    if (proc_row == topo_size-1)
        bottom_rank = MPI_PROC_NULL;

    if (proc_col == 0)
        left_rank = MPI_PROC_NULL;
    if (proc_col == topo_size-1)
        right_rank = MPI_PROC_NULL;

    srand(time(NULL));
    double high = 2021.0, low = -2021.0;
    for (int i=1; i<=N; i++)
        for (int j=1; j<=N; j++)
            old[i][j] = ((high-low)*(double)rand())/(double)RAND_MAX + low;


    start_time = MPI_Wtime();

    for (int step=0; step<num_of_steps; step++){

        double **new = malloc((N+2)*sizeof(double *)); // Data matrices--outer dim are recv buffers
        new[0] = malloc((N+2)*(N+2)*sizeof(double));
        for (int i=1; i<(N+2);i++)
          new[i] = new[i-1]+(N+2);

        MPI_Status status[8];
        MPI_Request request[8];

        int nreq=0;

        // Receive elements
        // Receive from left
        MPI_Irecv(&old[1][0], 1, columnVector, left_rank, MPI_ANY_TAG, MPI_COMM_WORLD, &request[nreq++]);
        // Receive from right
        MPI_Irecv(&old[1][N+1], 1, columnVector, right_rank, MPI_ANY_TAG, MPI_COMM_WORLD, &request[nreq++]);
        // Receive from top
        MPI_Irecv(&old[0][1], 1, rowVector, top_rank, MPI_ANY_TAG, MPI_COMM_WORLD, &request[nreq++]);
        // Receive from bottom
        MPI_Irecv(&old[N+1][1], 1, rowVector, bottom_rank, MPI_ANY_TAG, MPI_COMM_WORLD, &request[nreq++]);

        // Send elements
        // Send to left
        MPI_Isend(&old[1][1], 1, columnVector, left_rank, step, MPI_COMM_WORLD, &request[nreq++]);
        // Send to right
        MPI_Isend(&old[1][N], 1, columnVector, right_rank, step, MPI_COMM_WORLD, &request[nreq++]);
        // Send to top
        MPI_Isend(&old[1][1], 1, rowVector, top_rank, step, MPI_COMM_WORLD, &request[nreq++]);
        // Send to bottom
        MPI_Isend(&old[N][1], 1, rowVector, bottom_rank, step, MPI_COMM_WORLD, &request[nreq++]);

        // Stencil computation for non-halo region
        for (int r=2; r<=N-1; r++)
           for (int c=2; c<=N-1; c++)
                new[r][c] = (old[r-1][c] + old[r+1][c] + old[r][c-1] + old[r][c+1])/4.0;

        MPI_Waitall(nreq, request, status);

	      //Stencil computation for halo regio (separate computation for corner processes)
        if(proc_row == 0 || proc_row == topo_size-1 || proc_col == 0 || proc_col == topo_size-1){

            // Top Left Process
            if(proc_row ==0 && proc_col == 0){
                int r=1; // First row
                for (int c=1; c<=N; c++){
                    if (c==1)
                        new[r][c] = (old[r+1][c] + old[r][c+1])/2.0;
                    else
                        new[r][c] = (old[r+1][c] + old[r][c+1] + old[r][c-1])/3.0;
                }
                r=N; //Last row
                for (int c=1; c<=N; c++){
                    if (c==1)
                        new[r][c] = (old[r+1][c] + old[r][c+1] + old[r-1][c])/3.0;
                    else
                        new[r][c] = (old[r-1][c] + old[r+1][c] + old[r][c-1] + old[r][c+1])/4.0;
                }
                int c=1; // First column
                for (int r=2; r<N; r++)
                    new[r][c] = (old[r-1][c] + old[r][c+1] + old[r+1][c])/3.0;

                c=N; //Last column
                for (int r=2; r<N; r++)
                        new[r][c] = (old[r-1][c] + old[r+1][c] + old[r][c-1] + old[r][c+1])/4.0;
            }

            //Bottom Left Process
            if(proc_row ==topo_size-1 && proc_col == 0){
                int r=N; // Last row
                for (int c=1; c<=N; c++){
                    if (c==1)
                        new[r][c] = (old[r-1][c] + old[r][c+1])/2.0;
                    else
                        new[r][c] = (old[r-1][c] + old[r][c+1] + old[r][c-1])/3.0;
                }
                r=1; //First row
                for (int c=1; c<=N; c++){
                    if (c==1)
                        new[r][c] = (old[r+1][c] + old[r][c+1] + old[r-1][c])/3.0;
                    else
                        new[r][c] = (old[r-1][c] + old[r+1][c] + old[r][c-1] + old[r][c+1])/4.0;
                }
                int c=1; // First column
                for (int r=2; r<N; r++)
                    new[r][c] = (old[r-1][c] + old[r][c+1] + old[r+1][c])/3.0;

                c=N; //Last column
                for (int r=2; r<N; r++)
                        new[r][c] = (old[r-1][c] + old[r+1][c] + old[r][c-1] + old[r][c+1])/4.0;
            }

            // Bottom Right Process
            if(proc_row ==topo_size-1 && proc_col == topo_size-1){
                int r=N; // Last row
                for (int c=1; c<=N; c++){
                    if (c==N)
                        new[r][c] = (old[r-1][c] + old[r][c-1])/2.0;
                    else{
                        new[r][c] = (old[r-1][c] + old[r][c-1] + old[r][c+1])/3.0;
                    }
                }
                r=1; //First row
                for (int c=1; c<=N; c++){
                    if (c==N)
                        new[r][c] = (old[r+1][c] + old[r][c-1] + old[r-1][c])/3.0;
                    else
                        new[r][c] = (old[r-1][c] + old[r+1][c] + old[r][c-1] + old[r][c+1])/4.0;
                }
                int c=N; // Last column
                for (int r=2; r<N; r++)
                    new[r][c] = (old[r-1][c] + old[r][c-1] + old[r+1][c])/3.0;

                c=1; //First column
                for (int r=2; r<N; r++)
                        new[r][c] = (old[r-1][c] + old[r+1][c] + old[r][c-1] + old[r][c+1])/4.0;
            }

            //Top Right Process
            if(proc_row == 0 && proc_col == topo_size-1){
                int r=1; // First row
                for (int c=1; c<=N; c++){
                    if (c==N)
                        new[r][c] = (old[r+1][c] + old[r][c-1])/2.0;
                    else
                        new[r][c] = (old[r+1][c] + old[r][c+1] + old[r][c-1])/3.0;
                }
                r=N; //Last row
                for (int c=1; c<=N; c++){
                    if (c==N)
                        new[r][c] = (old[r+1][c] + old[r][c-1] + old[r-1][c])/3.0;
                    else
                        new[r][c] = (old[r-1][c] + old[r+1][c] + old[r][c-1] + old[r][c+1])/4.0;
                }
                int c=N; // Last Column
                for (int r=2; r<N; r++)
                    new[r][c] = (old[r-1][c] + old[r][c-1] + old[r+1][c])/3.0;

                c=1; //First column
                for (int r=2; r<N; r++)
                    new[r][c] = (old[r-1][c] + old[r+1][c] + old[r][c-1] + old[r][c+1])/4.0;
            }

            // Top Mid Process
            if(proc_row ==0 && proc_col != 0 && proc_col !=topo_size-1){
                int r=1; // First row
                for (int c=1; c<=N; c++)
                     new[r][c] = (old[r+1][c] + old[r][c+1] + old[r][c-1])/3.0;

                r=N; //Last row
                for (int c=1; c<=N; c++)
                    new[r][c] = (old[r-1][c] + old[r+1][c] + old[r][c-1] + old[r][c+1])/4.0;

                int c=1; // First column
                for (int r=2; r<N; r++)
                    new[r][c] = (old[r-1][c] + old[r+1][c] + old[r][c-1] + old[r][c+1])/4.0;

                c=N; //Last column
                for (int r=2; r<N; r++)
                    new[r][c] = (old[r-1][c] + old[r+1][c] + old[r][c-1] + old[r][c+1])/4.0;
            }

            //Bottom Mid Process
            if(proc_row ==topo_size-1 && proc_col != 0 && proc_col !=topo_size-1){

                int r=N; // Last row
                for (int c=1; c<=N; c++)
                    new[r][c] = (old[r-1][c] + old[r][c+1] + old[r][c-1])/3.0;

                r=1; //First row
                for (int c=1; c<=N; c++)
                    new[r][c] = (old[r-1][c] + old[r+1][c] + old[r][c-1] + old[r][c+1])/4.0;

                int c=1; // First column
                for (int r=2; r<N; r++)
                    new[r][c] = (old[r-1][c] + old[r+1][c] + old[r][c-1] + old[r][c+1])/4.0;

                c=N; //Last column
                for (int r=2; r<N; r++)
                    new[r][c] = (old[r-1][c] + old[r+1][c] + old[r][c-1] + old[r][c+1])/4.0;
            }

            //Left Mid Process
            if(proc_row !=topo_size-1 && proc_row!=0 && proc_col == 0){
                int r=N; // Last row
                for (int c=1; c<=N; c++){
                    if (c==1)
                        new[r][c] = (old[r-1][c] + old[r][c+1] + old[r+1][c])/3.0; // ?r+1,c in last term
                    else
                        new[r][c] = (old[r-1][c] + old[r+1][c] + old[r][c-1] + old[r][c+1])/4.0;
                }
                r=1; //First row
                for (int c=1; c<=N; c++){
                    if (c==1)
                        new[r][c] = (old[r+1][c] + old[r][c+1] + old[r-1][c])/3.0;
                    else
                        new[r][c] = (old[r-1][c] + old[r+1][c] + old[r][c-1] + old[r][c+1])/4.0;
                }
                int c=1; // First column
                for (int r=2; r<N; r++)
                    new[r][c] = (old[r-1][c] + old[r][c+1] + old[r+1][c])/3.0;

                c=N; //Last column
                for (int r=2; r<N; r++)
                    new[r][c] = (old[r-1][c] + old[r+1][c] + old[r][c-1] + old[r][c+1])/4.0;
            }

            //Right Mid Process
            if(proc_row != topo_size-1 && proc_row!=0 && proc_col == topo_size-1){
                int r=1; // First row
                for (int c=1; c<=N; c++){
                    if (c==N)
                        new[r][c] = (old[r+1][c] + old[r-1][c] + old[r][c-1])/3.0; //
                    else
                        new[r][c] = (old[r-1][c] + old[r+1][c] + old[r][c-1] + old[r][c+1])/4.0;
                }
                r=N; //Last row
                for (int c=1; c<=N; c++){
                    if (c==N)
                        new[r][c] = (old[r+1][c] + old[r][c-1] + old[r-1][c])/3.0;
                    else
                        new[r][c] = (old[r-1][c] + old[r+1][c] + old[r][c-1] + old[r][c+1])/4.0;
                }
                int c=N; // Last Column
                for (int r=2; r<N; r++)
                    new[r][c] = (old[r-1][c] + old[r][c-1] + old[r+1][c])/3.0;

                c=1; //First column
                for (int r=2; r<N; r++)
                    new[r][c] = (old[r-1][c] + old[r+1][c] + old[r][c-1] + old[r][c+1])/4.0;
            }
        }
        else{
            //Inner processes
            int r=1;
            for (int c=1; c<=N; c++)
                new[r][c] = (old[r-1][c] + old[r+1][c] + old[r][c-1] + old[r][c+1])/4.0;
            r=N;
            for (int c=1; c<=N; c++)
                new[r][c] = (old[r-1][c] + old[r+1][c] + old[r][c-1] + old[r][c+1])/4.0;

            int c=N; // Last Column
            for (int r=2; r<N; r++)
                new[r][c] = (old[r-1][c] + old[r+1][c] + old[r][c-1] + old[r+1][c])/4.0;

            c=1; //First column
            for (int r=2; r<N; r++)
                new[r][c] = (old[r-1][c] + old[r+1][c] + old[r][c-1] + old[r][c+1])/4.0;
        }

        memcpy (old, new, (N+2)*(N+2)*sizeof(double));
        free(new);
    }

    etime = MPI_Wtime() - start_time;
    MPI_Reduce (&etime, &max_time, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

    if (myrank == 0) printf ("P:%d N:%d Max_time:%lf\n", size, N, max_time);

    MPI_Finalize();

    /*printf("Process:%d\n", myrank);
    for (int i=0; i<N+2; i++){
        for (int j=0; j<N+2; j++)
            printf("%lf ", old[i][j]);
        printf("\n");
    }*/
    free(old[0]);
    free(old);
}


int main( int argc, char *argv[])
{

    int N = atoi(argv[1]); // square root of data points per process (size of matrix row/col)
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
