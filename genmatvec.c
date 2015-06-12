#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"

void genMatrix(int m, int n, double *A){
  double drand48();
  for (int i = 0; i < m*n; i ++){
    *A = drand48();
    A = A+1;
  }
  A = A - (m*n);

}


void genVector(int m, double *b){
  double drand48();
  for (int i = 0; i < m; i ++){
    *b = drand48();
    b = b+1;
  }
  b = b - (m-1);
}

void split_send( double *A, double *b, double *rowofA, int p, int m, int n, int *numjobs, int source, int *dest, int tag){

  int *localdest;
  localdest = dest;

  //sending 0 -> p
  for (int i = 0; i < p; i++){
    if (*localdest == 0){
      localdest++;
      rowofA = rowofA + m*(*numjobs);
      numjobs++;
    }
    else{
      MPI_Send(rowofA,m* (*numjobs), MPI_DOUBLE, *localdest,tag,MPI_COMM_WORLD);
      rowofA = rowofA + m* (*numjobs);
      numjobs++;
      localdest++;
    }
  
  }
  numjobs = numjobs - p;
  localdest = dest;
  rowofA = rowofA - p*m*n;

  free(localdest);

  MPI_Bcast(b, m, MPI_DOUBLE, 0, MPI_COMM_WORLD);
}

void calc_res(double *rowofA, double *b, int m, int n, int p, int rowsPerProcess, double *rowResult ){

  for (int j = 0; j < rowsPerProcess; j++){
    for (int i = 0; i < m; i++){
      *rowResult = *rowResult + *rowofA * (*b);
      rowofA++;
      b++;
    }
    b = b-m;
    rowResult++;
  }
  rowResult = rowResult - rowsPerProcess;

 }
