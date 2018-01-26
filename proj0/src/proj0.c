#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "Matrix.h"

int main() {
	
	//Set the image width and height
	size_t width = 640, height = 480;
	//Set the print width and height
	size_t printWidth = 16, printHeight = 8;
	
	//Part 1
	Matrix* A = allocateMatrix(height, width);
	setAllMatrixEntries(A, 25);
	
	//Part 2
	writeMatrix("images/A.img", A);
	
	//Part 3
	readMatrix("images/A.img", A);
	
	//Part 4
	printf("A:\n");
	printSubmatrix(printHeight, printWidth, 0, 0, A);
	printf("\n");
	
	//Part 5
	Matrix* B = allocateMatrix(height, width);
	setAllMatrixEntries(B, 100);
	
	//Part 6
	writeMatrix("images/B.img", B);
	
	//Part 7
	readMatrix("images/B.img", B);
	
	//Part 8
	printf("B:\n");
	printSubmatrix(printHeight, printWidth, 0, 0, B);
	printf("\n");
	
	//Part 9
	Matrix* C = createMatrixFromSum(A,B);
	/*Matrix* C = allocateMatrix(height, width);
	multiplyMatrices(C, A, B);*/
	
	//Part 10
	writeMatrix("images/C.img", C);
	
	//Part 11
	readMatrix("images/C.img", C);
	
	//Part 12
	printf("C:\n");
	printSubmatrix(printHeight, printWidth, 0, 0, C);
	
	//free memory of allocated matrices
	freeMatrix(A);
	freeMatrix(B);
	freeMatrix(C);
	
}
