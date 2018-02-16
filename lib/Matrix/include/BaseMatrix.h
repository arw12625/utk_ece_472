

#include <stdlib.h>
//#include <stdint.h>
#include <stdbool.h>

#define __TYPIFY(method, name) method ## name
#define _TYPIFY(method, name) __TYPIFY(method, name)
#define TYPIFY(method) _TYPIFY(method, mat_n)

/*
 * All matrices must be allocated with allocateMatrix before use.
 * All matrices must be freed with freeMatrix after use.
 *
 * All methods parameters of type Matrix* should point to an allocated Matrix.
 * That is both the struct Matrix and its array of entries should be allocated
 *	and the rows and cols set to non-zero integers.
 *
 * Methods returning Matrix* will return NULL if the operation was determined
 *	to be a failure.
 * Methods returning a bool will return true if the operation was determined
 *	to be a success and false otherwise.
 */
 

#define Matrix TYPIFY(Matrix)
typedef struct Matrix {
	size_t rows, cols;
	mat_t *mat;
} Matrix;

Matrix* TYPIFY(allocateMatrix)(size_t rows, size_t cols);
bool TYPIFY(freeMatrix)(Matrix* m);

Matrix* TYPIFY(wrapEntriesIntoMatrix)(size_t rows, size_t cols, mat_t* dat);
Matrix* TYPIFY(createMatrixFromEntries)(size_t rows, size_t cols, mat_t* dat);
bool TYPIFY(copyMatrixEntries)(Matrix* dest, Matrix* source);
Matrix* TYPIFY(duplicateMatrix)(Matrix* source);

bool TYPIFY(setMatrixEntry)(mat_t val, size_t row, size_t col, Matrix* m);
mat_t TYPIFY(getMatrixEntry)(size_t row, size_t col, Matrix* m);
//indexMatrix returns the pointer to the entry of m at the specified row and column
mat_t* TYPIFY(indexMatrix)(size_t row, size_t col, Matrix* m);
bool TYPIFY(setSubmatrix)(size_t rows, size_t cols, size_t destRow, size_t destCol, Matrix* dest, size_t sourceRow, size_t sourceCol, Matrix* source);
Matrix* TYPIFY(createSubmatrix)(size_t rows, size_t cols, size_t row, size_t col, Matrix* source);

bool TYPIFY(setAllMatrixEntries)(Matrix* m, mat_t val);

bool TYPIFY(writeMatrix)(const char* path, Matrix* m);
Matrix* TYPIFY(readMatrix)(const char* path);

bool TYPIFY(printSubmatrix)(size_t rows, size_t cols, size_t row, size_t col, Matrix* m);
bool TYPIFY(printMatrix)(Matrix* m);

bool TYPIFY(sumMatrices)(Matrix* dest, Matrix* summand1, Matrix* summand2);
Matrix* TYPIFY(createMatrixFromSum)(Matrix* summand1, Matrix* summand2);

bool TYPIFY(scaleMatrix)(Matrix* m, float scaler);
bool TYPIFY(multiplyMatrices)(Matrix* dest, Matrix* m1, Matrix* m2);
bool TYPIFY(naiveMultiplyMatrices)(Matrix* dest, Matrix* m1, Matrix* m2);

bool TYPIFY(hasSameDimension)(Matrix* m1, Matrix* m2);

bool TYPIFY(findMaxEntryMatrix)(Matrix* m, mat_t* maxVal, size_t* row, size_t* col);
mat_t TYPIFY(getMaxEntryMatrix)(Matrix* m);
bool TYPIFY(findMinEntryMatrix)(Matrix* m, mat_t* minVal, size_t* row, size_t* col);
mat_t TYPIFY(getMinEntryMatrix)(Matrix* m);

//operates on the entries of the matrix by the affine interval map from [a1, b1] to [a2, b2]
bool TYPIFY(intervalMapMatrix)(Matrix* m, mat_t a1, mat_t b1, mat_t a2, mat_t b2);

//interval maps the entries of the matrix from the range of the matrix to the desired range
//eg maps the mininum of m to minVal and the maximum of m to maxVal
bool TYPIFY(scaleRangeMatrix)(Matrix* m, mat_t minVal, mat_t maxVal);





#undef Matrix
#undef __TYPIFY
#undef _TYPIFY
#undef TYPIFY
