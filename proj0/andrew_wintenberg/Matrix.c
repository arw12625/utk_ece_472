#include <stdio.h>
#include <string.h>

#include "Matrix.h"

Matrix* allocateMatrix(size_t rows, size_t cols) {
	Matrix* m = malloc(sizeof(Matrix));
	m->rows = rows;
	m->cols = cols;
	m->mat = calloc(rows * cols, sizeof(uint8_t));
}

Matrix* createMatrixFromEntries(size_t rows, size_t cols, uint8_t* dat) {
	if(!dat) {
		return NULL;
	}
	Matrix* m = malloc(sizeof(Matrix));
	m->rows = rows;
	m->cols = cols;
	m->mat = dat;
}

bool copyMatrixEntries(Matrix* dest, Matrix* source) {
	if(!source) {
		printf("Cannot copy matrix from NULL source");
		return false;
	}
	if(!dest) {
		printf("Cannot copy matrix to NULL dest");
		return false;
	}
	if(hasSameDimension(dest, source)) {
		printf("Matrix size mismatch in copy");
		return false;
	}
	memcpy(dest->mat, source->mat, sizeof(uint8_t) * source->rows * source->cols);
	return true;
}

Matrix* duplicateMatrix(Matrix* source) {
	Matrix* dup = allocateMatrix(source->rows, source->cols);
	copyMatrixEntries(dup, source);
	return dup;
}

bool freeMatrix(Matrix* m) {
	if(!m) {
		printf("Cannot free NULL matrix");
		return false;
	}
	free(m->mat);
	m->mat = NULL;
	free(m);
	return true;
}

bool setSubmatrix(size_t rows, size_t cols, size_t destRow, size_t destCol, Matrix* dest, size_t sourceRow, size_t sourceCol, Matrix* source) {
	if(!source) {
		printf("Cannot set submatrix from NULL source");
		return false;
	}
	if(!dest) {
		printf("Cannot set submatrix to NULL dest");
		return false;
	}
	if(rows + sourceRow > source->rows || cols + sourceCol > source->cols
	&& rows + destRow > dest->rows && cols + destCol > dest->cols) {
		printf("Size mismatch when setting submatrix");
		return false;
	}
	size_t i;
	for(i = 0; i < rows; i++) {
		memcpy(indexMatrix(i + destRow, destCol, dest),
			indexMatrix(i + sourceRow, sourceCol, source),
			sizeof(uint8_t) * cols); 
	}
	return true;
}

Matrix* createSubmatrix(size_t rows, size_t cols, size_t row, size_t col, Matrix* source) {
	if(!source) {
		printf("Cannot create submatrix from NULL source");
		return false;
	}
	Matrix* sub = allocateMatrix(rows, cols);
	if(setSubmatrix(rows, cols, 0, 0, sub, row, col, source)) {
		return sub;
	} else {
		return NULL;
	}
}

bool setAllMatrixEntries(Matrix* m, uint8_t val) {
	if(!m) {
		printf("Cannot set entries to NULL matrix");
		return false;
	}
	memset(m->mat, val, m->rows * m->cols * sizeof(uint8_t));
	return true;
}

bool setMatrixEntry(uint8_t val, size_t row, size_t col, Matrix* m) {
	/*
	if(!m) {
		printf("Cannot set entry of NULL matrix");
		return false;
	}
	if(row >= m->rows || col >= m->cols) {
		return false;
	}
	*/
	uint8_t* p = indexMatrix(row, col, m);
	if(!p) {
		return false;
	}
	*(p) = val;
	
}

uint8_t getMatrixEntry(size_t row, size_t col, Matrix* m) {
	/*
	if(!m) {
		printf("Cannot get entry from NULL matrix");
		return 0;
	}
	if(row >= m->rows || col >= m->cols) {
		return 0;
	}
	*/
	return *indexMatrix(row, col, m);
}

uint8_t* indexMatrix(size_t row, size_t col, Matrix* m) {
	if(!m) {
		printf("Cannot index NULL matrix");
		return NULL;
	}
	if(row >= m->rows || col >= m->cols) {
		return NULL;
	}
	return &(m->mat[row * m->cols + col]);
}

bool writeMatrix(const char* path, Matrix* m) {
	if(!m) {
		printf("Cannot write NULL matrix");
		return false;
	}
	FILE* fp = fopen(path, "w");
	if(!fp) {
		printf("Could not open file to write matrix to: %s", path);
		return false;
	}
	
	uint32_t rows = m->rows, cols = m->cols;
	fwrite(&rows, sizeof(uint32_t), 1, fp);
	fwrite(&cols, sizeof(uint32_t), 1, fp);
	size_t numWritten = fwrite(m->mat, sizeof(uint8_t), rows * cols * sizeof(uint8_t), fp);
	fclose(fp);
	if(rows * cols * sizeof(uint8_t) != numWritten) {
		printf("Error when writing image to: %s", path);
		return false;
	}
	return true;
}

Matrix* createMatrixFromFile(const char* path) {
	FILE* fp = fopen(path, "r");
	if(!fp) {
		printf("Could not open file to read matrix from: %s", path);
		return NULL;
	}
	
	uint32_t rows, cols;
	fread(&rows, sizeof(uint32_t), 1, fp);
	fread(&cols, sizeof(uint32_t), 1, fp);
	Matrix* m = allocateMatrix(rows, cols);
	size_t numRead = fread(m->mat, sizeof(uint8_t), rows * cols * sizeof(uint8_t), fp);
	fclose(fp);
	if(rows * cols * sizeof(uint8_t) != numRead) {
		printf("Error when reading matrix from: %s", path);
		return NULL;
	}
	return m;
}

bool readMatrix(const char* path, Matrix* m) {
	if(!m) {
		printf("Cannot read into NULL matrix");
		return false;
	}
	FILE* fp = fopen(path, "r");
	if(!fp) {
		printf("Could not open file to read matrix from: %s", path);
		return false;
	}
	
	uint32_t rows, cols;
	fread(&rows, sizeof(uint32_t), 1, fp);
	fread(&cols, sizeof(uint32_t), 1, fp);
	if(m->rows == rows && m->cols == cols) {
		size_t numRead = fread(m->mat, sizeof(uint8_t), rows * cols * sizeof(uint8_t), fp);
		fclose(fp);
		if(rows * cols * sizeof(uint8_t) != numRead) {
			printf("Error when reading matrix from: %s", path);
			return false;
		}
	} else {
		printf("Size mismatch when reading matrix");
		fclose(fp);
		return false;
	}
	return true;
}

bool printSubmatrix(size_t rows, size_t cols, size_t row, size_t col, Matrix* m) {
	if(!m) {
		printf("Cannot print NULL matrix");
		return false;
	}
	if(row + rows > m->rows || col + cols > m->cols) {
		printf("Size mismatch when printing matrix");
		return false;
	}
	
	size_t i, j;
	for(i = 0; i < rows; i++) {
		for(j = 0; j < cols - 1; j++) {
			printf("%i, ", getMatrixEntry(i + row, j + col, m));
		}
		printf("%i\n", getMatrixEntry(i + row, j + col, m));
	}
	return true;
}

bool printMatrix(Matrix* m) {
	return printSubmatrix(m->rows, m->cols, 0, 0, m);
}

bool sumMatrices(Matrix* dest, Matrix* summand1, Matrix* summand2) {
	if(!dest) {
		printf("Cannot sum matrices into NULL matrix");
		return false;
	}
	if(!summand1 || !summand2) {
		printf("Cannot sum with a NULL matrix");
		return false;
	}
	if(!hasSameDimension(summand1, summand2) || !hasSameDimension(summand1, dest)) {
		printf("Size mismatch in summing matrices");
		return false;
	}
	size_t i, j;
	for(i = 0; i < dest->rows; i++) {
		for(j = 0; j < dest->cols; j++) {
			setMatrixEntry(getMatrixEntry(i,j,summand1) + getMatrixEntry(i,j,summand2), 
				i, j, dest);
		}
	}
	return true;
	
}

Matrix* createMatrixFromSum(Matrix* summand1, Matrix* summand2) {
	if(!hasSameDimension(summand1, summand2)) {
		printf("Size mismatch in creating matrix from sum");
	}
	Matrix* sum = allocateMatrix(summand1->rows, summand1->cols);
	if(sumMatrices(sum, summand1, summand2)) {
		return sum;
	} else {
		return NULL;
	}
}

bool hasSameDimension(Matrix* m1, Matrix* m2) {
	if(!m1 || !m2) {
		printf("Cannot compare dimensions of a NULL matrix");
		return false;
	}
	return (m1->rows == m2->rows) && (m1->cols == m2->cols);
}