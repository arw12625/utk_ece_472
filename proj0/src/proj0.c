#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "Matrix_u8.h"

int main() {
	
	//Set the image width and height
	size_t width = 640, height = 480;
	//Set the print width and height
	size_t printWidth = 16, printHeight = 8;
	
	//Part 1
	Matrix_u8* A = allocateMatrix_u8(height, width);
	setAllMatrixEntries_u8(A, 25);
	
	//Part 2
	writeMatrix_u8("images/A.img", A);
	
	//Part 3
	Matrix* A1 = readMatrix_u8("images/A.img");
	
	//Part 4
	printf("A:\n");
	printSubmatrix_u8(printHeight, printWidth, 0, 0, A);
	printf("\n");
	
	//Part 5
	Matrix_u8* B = allocateMatrix_u8(height, width);
	setAllMatrixEntries_u8(B, 100);
	
	//Part 6
	writeMatrix_u8("images/B.img", B);
	
	//Part 7
	Matrix* B1 = readMatrix_u8("images/B.img", B);
	
	//Part 8
	printf("B:\n");
	printSubmatrix_u8(printHeight, printWidth, 0, 0, B);
	printf("\n");
	
	//Part 9
	Matrix_u8* C = createMatrixFromSum_u8(A,B);
	/*Matrix* C = allocateMatrix(height, width);
	multiplyMatrices(C, A, B);*/
	
	//Part 10
	writeMatrix_u8("images/C.img", C);
	
	//Part 11
	Matrix* C1 = readMatrix_u8("images/C.img", C);
	
	//Part 12
	printf("C:\n");
	printSubmatrix_u8(printHeight, printWidth, 0, 0, C);
	
	//free memory of allocated matrices
	freeMatrix_u8(A);
	freeMatrix_u8(B);
	freeMatrix_u8(C);
	freeMatrix_u8(A1);
	freeMatrix_u8(B1);
	freeMatrix_u8(C1);
	
}
