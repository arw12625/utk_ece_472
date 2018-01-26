#ifndef _MATRIX_H_
#define _MATRIX_H_

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

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
 
typedef uint32_t mat_t;
 
typedef struct Matrix {
	size_t rows, cols;
	mat_t *mat;
} Matrix;


Matrix* allocateMatrix(size_t rows, size_t cols);
bool freeMatrix(Matrix* m);

Matrix* createMatrixFromEntries(size_t rows, size_t cols, mat_t* dat);
bool copyMatrixEntries(Matrix* dest, Matrix* source);
Matrix* duplicateMatrix(Matrix* source);

bool setMatrixEntry(mat_t val, size_t row, size_t col, Matrix* m);
mat_t getMatrixEntry(size_t row, size_t col, Matrix* m);
//indexMatrix returns the pointer to the entry of m at the specified row and column
mat_t* indexMatrix(size_t row, size_t col, Matrix* m);
bool setSubmatrix(size_t rows, size_t cols, size_t destRow, size_t destCol, Matrix* dest, size_t sourceRow, size_t sourceCol, Matrix* source);
Matrix* createSubmatrix(size_t rows, size_t cols, size_t row, size_t col, Matrix* source);

bool setAllMatrixEntries(Matrix* m, mat_t val);

bool writeMatrix(const char* path, Matrix* m);
bool readMatrix(const char* path, Matrix* m);
Matrix* createMatrixFromFile(const char* path);

bool printSubmatrix(size_t rows, size_t cols, size_t row, size_t col, Matrix* m);
bool printMatrix(Matrix* m);

bool sumMatrices(Matrix* dest, Matrix* summand1, Matrix* summand2);
Matrix* createMatrixFromSum(Matrix* summand1, Matrix* summand2);

bool scaleMatrix(Matrix* m, float scaler);
bool multiplyMatrices(Matrix* dest, Matrix* m1, Matrix* m2);
bool naiveMultiplyMatrices(Matrix* dest, Matrix* m1, Matrix* m2);


bool hasSameDimension(Matrix* m1, Matrix* m2);
#endif