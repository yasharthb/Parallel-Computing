// Main source file for Assignment3

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include "mpi.h"
#define FILE_SIZE 256

int main( int argc, char *argv[])
{

  if(argc != 2){
    printf("File Name not provided!");
    return -1;
  }
  
  char filename[FILE_SIZE];
  strcpy(filename, argv[1]); 
  
  


  return 0;
}

