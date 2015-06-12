#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

void getResult(int m, int n, double *A, double *b, double *y)
{
  double *testrowResult = (double *) malloc(n*sizeof(double));
  bool resultsSame;

  for (int j = 0; j < n; j++){
    for (int i = 0; i < m; i++){
      *testrowResult = *testrowResult + *A * (*b);
      A++;
      b++;
    }
    b = b-m;
    testrowResult++;
  }
  testrowResult = testrowResult - n;
  A = A - m*n;

	printf("Total Result: [ ");
    for (int i = 0; i < n; i++){
      printf("%f ", *testrowResult);
      testrowResult++;
    }
    printf("]\n");
    testrowResult = testrowResult - n;

	int i;
	for ( i = 0; i < n; i++){
		if ( *testrowResult != *y){
			resultsSame = false;
			break;		
		}
		else{
			if( i == (n-1))
				resultsSame = true;
		}
		testrowResult++;
		y++;
	}
	testrowResult = testrowResult - i;
	y = y - i;

	if ( resultsSame == true)
		puts("True");
	else
		printf("False: %d", i);
}
