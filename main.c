#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mpi.h"
//#include <math.h>

void genMatrix(int m, int n, double *A);
void genVector(int n, double *b);
void split_send(double *A, double *b, double *rowofA, int p, int m, int n, int *numjobs, int source, int* dest, int tag);
void calc_res( double *rowofA, double *b, int m, int n,int p,int rowsPerProcess, double * rowResult);
void getResult(int m, int n, double *A, double *b, double *y);

int main(int argc, char* argv[])
{
  int         my_rank;       /* rank of process      */
  int         p;             /* number of processes  */
  int         source=0;        /* rank of sender       */
  int         destination;
  int         tag = 0;       /* tag for messages     */
  MPI_Status  status;        /* status for receive   */

  int rowsPerProcess;
  int processesInUse;

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
  MPI_Comm_size(MPI_COMM_WORLD, &p);

  int m = atoi(argv[1]);
  int n = atoi(argv[2]);

  //allocate b
  double *b = (double *)malloc(m*sizeof(double));


  //assign number of rows process is responsible for
    if (my_rank > n%p - 1)
      rowsPerProcess = n/p;
    else 
      rowsPerProcess = n/p+1;

  double *rowResult;
  if (my_rank == 0)
     rowResult = (double *) malloc(n*sizeof(double));
  else
     rowResult = (double *) malloc(rowsPerProcess*sizeof(double));


  //Clear memory contents of final vector
  if(my_rank ==0){
    for ( int i = 0; i < n; i ++){
      *rowResult = 0;
      rowResult++;
    }
    rowResult = rowResult - n; 
  }
  else{
    for ( int i = 0; i < rowsPerProcess; i ++){
      *rowResult = 0;
      rowResult++;
    }
    rowResult = rowResult - rowsPerProcess; 
  }

  //assign number of processes that will be used
  if (p>n){
    processesInUse = n;
  }
  else
    processesInUse = p;


  //allocate mem for rows
  double *rowofA;
  if (my_rank == 0){
     rowofA = (double *) malloc(n*m*sizeof(double));
  }
  else{
     rowofA = (double *) malloc(rowsPerProcess*m*sizeof(double)); 
  }    


  //Enter process 0 main
  if (my_rank == 0){
    int *dest = (int *) malloc((processesInUse)*sizeof(int));  
    double *totalResult;   // = (double *)malloc(n*sizeof(double));
    int *numjobs = (int *)malloc(processesInUse*sizeof(int));

    //building destination list
    for (int i = 0; i < processesInUse; i++){
      *dest = i;
      //printf("dest:%d\n", *dest);
      dest = dest + 1;
    }
    dest = dest - (processesInUse);

    //initialize numjobs[]
    for (int i = 0; i < processesInUse; i++){
      if ( *dest > n%processesInUse - 1){
	*numjobs = n/processesInUse;
      }
      else{
	*numjobs = n/processesInUse+1;
      }
      dest++;
      numjobs++;
    }
    dest = dest - processesInUse;
    numjobs = numjobs - (processesInUse);
  
    //gen matrix
    genMatrix(m,n, rowofA);
    //gen vector
    genVector(m, b);


    //split and send A and b
    split_send(rowofA, b, rowofA, processesInUse, m, n, numjobs, my_rank, dest, tag);

    //calculate: result[] = matrix row * vector
    calc_res(rowofA, b, m, n, processesInUse, rowsPerProcess, rowResult);
    totalResult = rowResult;
    rowResult = rowResult + rowsPerProcess;
    numjobs++;

    //receive results from other processes -- load into final array
    for (int i = 1; i < processesInUse; i ++){
      MPI_Recv(rowResult, *numjobs, MPI_DOUBLE, i, tag, MPI_COMM_WORLD, &status);
      //printf("0 Received from %f from %d\n", *rowResult, i);
      rowResult = rowResult + *numjobs;
      numjobs++;
    } 
    rowResult = totalResult;

    //print final array
    printf("Total Result: [ ");
    for (int i = 0; i < n; i++){
      printf("%f ", *totalResult);
      totalResult++;
    }
    printf("]\n");
    totalResult = rowResult;

    //Call getResult
    getResult(m,n, rowofA,b,totalResult);

  }
  //All processess other than 0 and < n rows
  else if (my_rank < n){

    //Receive rowsofA
    MPI_Recv(rowofA, m*rowsPerProcess, MPI_DOUBLE, source, tag, MPI_COMM_WORLD, &status);
    
    //get b
    MPI_Bcast(b, m, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    //calculate: result[] = matrix row * vector
    calc_res(rowofA, b, m, n,processesInUse,rowsPerProcess, rowResult);

    //send results to 0
    destination = 0;
    MPI_Send(rowResult, rowsPerProcess, MPI_DOUBLE, destination, tag, MPI_COMM_WORLD);
      
}


  MPI_Finalize();

  return 0;
} 

