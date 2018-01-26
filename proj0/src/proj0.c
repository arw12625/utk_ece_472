#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "Matrix.h"

int main() {
	
	size_t width = 640, height = 480;
	size_t printWidth = 16, printHeight = 8;
	
	Matrix* A = allocateMatrix(height, width);
	setAllMatrixEntries(A, 25);
	writeMatrix("images/A.img", A);
	readMatrix("images/A.img", A);
	printf("A:\n");
	printSubmatrix(printHeight, printWidth, 0, 0, A);
	printf("\n");
	
	Matrix* B = allocateMatrix(height, width);
	setAllMatrixEntries(B, 100);
	writeMatrix("images/B.img", B);
	readMatrix("images/B.img", B);
	printf("B:\n");
	printSubmatrix(printHeight, printWidth, 0, 0, B);
	printf("\n");
	
	printf("C:\n");
	Matrix* C = createMatrixFromSum(A,B);
	writeMatrix("images/C.img", C);
	readMatrix("images/C.img", C);
	printSubmatrix(printHeight, printWidth, 0, 0, C);
	
	freeMatrix(A);
	freeMatrix(B);
	freeMatrix(C);
	
}
