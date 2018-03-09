#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "Matrix_d.h"

bool write3x3Filter(const char* kerPath, double entries[9]) {
	Matrix_d* kerMat = createMatrixFromEntries_d(3, 3, entries);
	bool success = writeMatrix_d(kerPath, kerMat);
	freeMatrix_d(kerMat);
	return success;
}

int main()  {

	double ker1[] = {1.0/9, 1.0/9, 1.0/9, 1.0/9, 1.0/9, 1.0/9, 1.0/9, 1.0/9, 1.0/9};
	write3x3Filter("kernels/ker1.dat", ker1);

	double ker2[] = {1.0/16.0, 2.0/16.0, 1.0/16.0, 2.0/16.0, 4.0/16.0, 2.0/16.0, 1.0/16.0, 2.0/16.0, 1.0/16.0};
	write3x3Filter("kernels/ker2.dat", ker2);

	double ker3[] = {0, 1, 0, 1, -4, 1, 0, 1, 0};
	write3x3Filter("kernels/ker3.dat", ker3);

	double ker4[] = {1, 1, 1, 1, -8, 1, 1, 1, 1};
	write3x3Filter("kernels/ker4.dat", ker4);

	double ker5[] = {0, -1, 0, -1, 4, -1, 0, -1, 0};
	write3x3Filter("kernels/ker5.dat", ker5);

	double ker6[] = {-1, -2, -1, 0, 0, 0, 1, 2, 1};
	write3x3Filter("kernels/ker6.dat", ker6);

	double ker7[] = {-1, 0, 1, -2, 0, 2, -1, 0, 1};
	write3x3Filter("kernels/ker7.dat", ker7);
	
	return 0;
}
