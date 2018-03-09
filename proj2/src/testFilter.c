#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "Matrix_i16.h"


int main()  {
	
	int16_t dat[] = {1, 2, 3, 4, 5, 6, 7, 8, 9};
	Matrix_i16* mat = createMatrixFromEntries_i16(3, 3, dat);
	printf("Original Matrix:\n");
	printMatrix_i16(mat);
	Matrix_i16* topLeftNbhd = createSubmatrixPadZero_i16(3, 3, -1, -1, mat);
	printf("Top Left 3x3 Neighborhood with Padding:\n");
	printMatrix_i16(topLeftNbhd);
	freeMatrix_i16(topLeftNbhd);
	
	Matrix_i16* bottomRightNbhd = createSubmatrixPadZero_i16(3, 3, 1, 1, mat);
	printf("Bottom Right 3x3 Neighborhood with Padding:\n");
	printMatrix_i16(bottomRightNbhd);
	freeMatrix_i16(bottomRightNbhd);
	
	freeMatrix_d(mat);
	
	
	return 0;
}
