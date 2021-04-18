// Main source file for Assignment3

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <float.h>
#include <limits.h>
#include "mpi.h"
#define FILE_SIZE 256
#define MIN(a,b) (((a)<(b))?(a):(b))

int main( int argc, char *argv[])
{

    if(argc != 2){
        printf("File Name not provided!");
        return -1;
    }
    
    char filename[FILE_SIZE];
    strcpy(filename, argv[1]); 
    FILE *fp = fopen(filename, "r");
    
    FILE *fp_out;
    fp_out = fopen("output.txt", "w");
 
    FILE *fp_dat;
    fp_dat = fopen("data.tmp", "w");

    MPI_Init(&argc, &argv);
    int rank, size;
    MPI_Comm_rank( MPI_COMM_WORLD, &rank);
    MPI_Comm_size( MPI_COMM_WORLD, &size);

    int ncol = 0; // No of years
    int nrow = 0; // No of stations

    double* data;
    // Process 0 reads the file
    if (rank == 0){
        char line[500]; //TODO: Size of one row?
        int curr = 0;
        
        // Get the number of columns from the first row
        fgets(line, sizeof(line), fp);
        char* ptr;
        ptr = strtok(line, ",");
        while (ptr){
            ncol++;
            ptr = strtok(NULL, ",");
        }
        ncol -= 2;
    
        data = (double*)malloc(sizeof(double)*ncol);
    
        while (fgets (line, sizeof(line), fp)){
            nrow++;

            // Realloc when reading new rows
            if (nrow>1)
                data = (double*)realloc(data, sizeof(double)*ncol*nrow);

            ptr = strtok(line, ",");
            for (int i=0; i<ncol+2; i++){
                if (i<2){
                    ptr = strtok(NULL, ",");
                    continue;
                }   
                sscanf(ptr, "%lf", &data[curr+i-2]);
                ptr = strtok(NULL, ",");
            }

            //Update offset for data array
            curr += ncol;
        }
        /*for (int i=0; i<nrow; i++){
            if (i!=nrow-1)
                continue;
            for (int j=0; j<ncol; j++)
                printf("%lf ", data[i*ncol+j]);
            printf("\n");
        }*/
    }

    double stime = MPI_Wtime();
    
    // Broadcast nrow and ncol information
    MPI_Bcast(&nrow, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&ncol, 1, MPI_INT, 0, MPI_COMM_WORLD);
    
    int* sendcnt = (int*)malloc(size*sizeof(int));
    int* displarr = (int*)malloc(size*sizeof(int));
    int disp = 0;
    
    //Deduce no of rows to send to each process
    if (!rank){
        for (int i=0; i<size; i++){
            if (i<nrow%size){
                sendcnt[i] = ncol*((nrow/size) + 1);
                displarr[i] = disp;
                disp += sendcnt[i];
            }

            else{
                sendcnt[i] = ncol*(nrow/size);
                displarr[i] = disp;
                disp += sendcnt[i];
            }
        }
    }
    /*if (!rank)
        for (int i=0; i<size; i++)
            printf("Rank:%d %d %d\n",i, sendcnt[i], displarr[i]);*/

    int recvcnt;
    if (rank<nrow%size)
        recvcnt = ncol*((nrow/size)+1);
    else
        recvcnt = ncol*(nrow/size);

    double* input = (double*)malloc(sizeof(double)*recvcnt);

    // Scatterv to distribute data to each process
    MPI_Scatterv(data, sendcnt, displarr, MPI_DOUBLE, input, recvcnt, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    //printf("Process:%d -- %d\n", rank, recvcnt);
    
    double* procmin = (double*)malloc(sizeof(double)*ncol); // Store the yearwise min on each process
    int rowcnt = recvcnt/ncol;
    
    //Get yearwise minimum
    for (int yr=0; yr<ncol; yr++){
        double mintemp = DBL_MAX; 
        for (int r=0; r<rowcnt; r++)
            mintemp = MIN(mintemp, input[r*ncol+yr]);
        procmin[yr] = mintemp;
    }

    //MPI_Reduce to get yearwise min on process 0
    double* yearmin = (double*)malloc(sizeof(double)*ncol); // Stores the yearwise min after reduce call
    MPI_Reduce(procmin, yearmin, ncol, MPI_DOUBLE, MPI_MIN, 0, MPI_COMM_WORLD);

    if (!rank){
        for (int i=0; i<ncol; i++)
            fprintf(fp_out, "%lf,", yearmin[i]);
        fprintf(fp_out, "\n");
    }

    //Global minimum
    if (!rank){
        double globalmin = DBL_MAX;
        for (int yr=0; yr<ncol; yr++)
            globalmin = MIN(globalmin, yearmin[yr]);
        fprintf(fp_out, "%lf\n", globalmin);
    }

    double time = MPI_Wtime()-stime;
    double maxtime;
    MPI_Reduce (&time, &maxtime, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
    if (!rank){
	fprintf(fp_dat," %lf\n", maxtime);    
        fprintf(fp_out, "%lf\n", maxtime);
    }

    MPI_Finalize();
    return 0;
}

