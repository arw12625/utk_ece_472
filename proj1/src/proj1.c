#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "Matrix.h"

int main() {
	
	size_t width = 5, height = 5;
	
	Matrix* A = allocateMatrix(height, width);
	setAllMatrixEntries(A, 1);
	printf("A:\n");
	printMatrix(A);
	printf("\n");
	
	Matrix* B = allocateMatrix(height, width);
	multiplyMatrices(B, A, A);
	printf("B:\n");
	printMatrix(B);
	printf("\n");
	
	freeMatrix(A);
	freeMatrix(B);
	
}
