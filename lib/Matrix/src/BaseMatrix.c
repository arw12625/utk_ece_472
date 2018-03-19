#include <stdio.h>
#include <string.h>


#define __TYPIFY(method, name) method ## name
#define _TYPIFY(method, name) __TYPIFY(method, name)
#define TYPIFY(method) _TYPIFY(method, mat_n)
#define Matrix TYPIFY(Matrix)

Matrix* TYPIFY(allocateEmptyMatrix)(size_t rows, size_t cols) {
	Matrix* m = malloc(sizeof(Matrix));
	m->rows = rows;
	m->cols = cols;
	return m;
}

Matrix* TYPIFY(allocateMatrix)(size_t rows, size_t cols) {
	Matrix* m = TYPIFY(allocateEmptyMatrix)(rows, cols);
	m->mat = calloc(rows * cols, sizeof(mat_t));
	return m;
}

/*
Matrix* TYPIFY(wrapEntriesIntoMatrix)(size_t rows, size_t cols, mat_t* dat) {
	if(!dat) {
		printf("Cannot wrap NULL entries into matrix\n");
		return NULL;
	}
	Matrix* m = TYPIFY(allocateEmptyMatrix)(rows, cols);
	m->mat = dat;
	return m;
}*/

Matrix* TYPIFY(createMatrixFromEntries)(size_t rows, size_t cols, mat_t* dat) {
	if(!dat) {
		printf("Cannot create matrix from NULL entries\n");
		return NULL;
	}
	Matrix* m = TYPIFY(allocateMatrix)(rows, cols);
	memcpy(m->mat, dat, sizeof(mat_t) * rows * cols);
	return m;
}

bool TYPIFY(copyMatrixEntries)(Matrix* dest, Matrix* source) {
	if(!source) {
		printf("Cannot copy matrix from NULL source\n");
		return false;
	}
	if(!dest) {
		printf("Cannot copy matrix to NULL dest\n");
		return false;
	}
	if(!TYPIFY(hasSameDimension)(dest, source)) {
		printf("Matrix size mismatch in copy\n");
		return false;
	}
	memcpy(dest->mat, source->mat, sizeof(mat_t) * source->rows * source->cols);
	return true;
}

Matrix* TYPIFY(duplicateMatrix)(Matrix* source) {
	if(!source) {
		printf("Cannot duplicate NULL matrix\n");
		return NULL;
	}
	Matrix* dup = TYPIFY(allocateMatrix)(source->rows, source->cols);
	TYPIFY(copyMatrixEntries)(dup, source);
	return dup;
}

bool TYPIFY(freeMatrix)(Matrix* m) {
	if(!m) {
		printf("Cannot free NULL matrix\n");
		return false;
	}
	free(m->mat);
	m->mat = NULL;
	free(m);
	return true;
}

bool TYPIFY(setSubmatrix)(size_t rows, size_t cols, size_t destRow, size_t destCol, Matrix* dest, size_t sourceRow, size_t sourceCol, Matrix* source) {
	if(!source) {
		printf("Cannot set submatrix from NULL source\n");
		return false;
	}
	if(!dest) {
		printf("Cannot set submatrix to NULL dest\n");
		return false;
	}
	if(rows + sourceRow > source->rows || cols + sourceCol > source->cols
	&& rows + destRow > dest->rows && cols + destCol > dest->cols) {
		printf("Size mismatch when setting submatrix\n");
		return false;
	}
	size_t i;
	for(i = 0; i < rows; i++) {
		memcpy(TYPIFY(indexMatrix)(i + destRow, destCol, dest),
			TYPIFY(indexMatrix)(i + sourceRow, sourceCol, source),
			sizeof(mat_t) * cols); 
	}
	return true;
}

Matrix* TYPIFY(createSubmatrix)(size_t rows, size_t cols, size_t row, size_t col, Matrix* source) {
	if(!source) {
		printf("Cannot create submatrix from NULL source\n");
		return false;
	}
	Matrix* sub = TYPIFY(allocateMatrix)(rows, cols);
	if(TYPIFY(setSubmatrix)(rows, cols, 0, 0, sub, row, col, source)) {
		return sub;
	} else {
		return NULL;
	}
}


Matrix* TYPIFY(createSubmatrixPadZero)(size_t rows, size_t cols, ptrdiff_t row, ptrdiff_t col, Matrix* source) {
	if(!source) {
		printf("Cannot create submatrix from NULL source\n");
		return false;
	}
	Matrix* sub = TYPIFY(allocateMatrix)(rows, cols);
	
	if(row > (ptrdiff_t)source->rows || col > (ptrdiff_t)source->cols || row + rows <= 0 || col + cols <= 0) {
		return sub;
	}
	
	size_t sourceRowStart = maxVal(0, row), sourceRowEnd = minVal(source->rows, row + rows);
	size_t destRowStart = maxVal(0, -row), destRowEnd = minVal(rows, source->rows - row);
	size_t sourceColStart = maxVal(0, col), sourceColEnd = minVal(source->cols, col + cols);
	size_t destColStart = maxVal(0, -col), destColEnd = minVal(cols, source->cols - col);
	

	TYPIFY(setSubmatrix)(destRowEnd - destRowStart, destColEnd - destColStart,
		destRowStart, destColStart, sub,
		sourceRowStart, sourceColStart, source);
		
	return sub;
}

bool TYPIFY(setAllMatrixEntries)(Matrix* m, mat_t val) {
	if(!m) {
		printf("Cannot set entries to NULL matrix\n");
		return false;
	}
	//does not work with non-uint8_t values
	//memset(m->mat, val, m->rows * m->cols * sizeof(mat_t));
	
	size_t i, j;
	for(i = 0; i < m->rows; i++) {
		for(j = 0; j < m->cols; j++) {
			TYPIFY(setMatrixEntry)(val, i, j, m);
		}
	}
	
	return true;
}

bool TYPIFY(scaleMatrix)(Matrix* m, float scaler) {
	if(!m) {
		printf("Cannot scale NULL matrix\n");
		return false;
	}
	size_t i, j;
	for(i = 0; i < m->rows; i++) {
		for(j = 0; j < m->cols; j++) {
			TYPIFY(setMatrixEntry)((mat_t)(scaler * TYPIFY(getMatrixEntry)(i,j,m)), i, j, m);
		}
	}
	return true;
}

bool TYPIFY(multiplyMatrices)(Matrix* dest, Matrix* m1, Matrix* m2) {
	return TYPIFY(naiveMultiplyMatrices)(dest, m1, m2);
}

bool TYPIFY(naiveMultiplyMatrices)(Matrix* dest, Matrix* m1, Matrix* m2) {
	if(!dest) {
		printf("Cannot multiply matrices into NULL matrix\n");
		return false;
	}
	if(!m1 || !m2) {
		printf("Cannot multiply with a NULL matrix\n");
		return false;
	}
	if(m1 == dest || m2 == dest) {
		printf("Destination matrix must be different from multiplicands\n");
		return false;
	}
	if(m1->cols != m2->rows || dest->cols != m2->cols || dest->rows != m1->rows) {
		printf("Size mismatch in matrix multiplication\n");
		return false;
	}
	
	size_t i, j, k;
	for(i = 0; i < dest->rows; i++) {
		for(j = 0; j < dest->cols; j++) {
			mat_t val = 0;
			for(k = 0; k < m1->cols; k++) {
				val += TYPIFY(getMatrixEntry)(i, k, m1) * TYPIFY(getMatrixEntry)(k, j, m2);
			}
			TYPIFY(setMatrixEntry)(val, i, j, dest);
		}
	}
	
	return true;
}

bool TYPIFY(setMatrixEntry)(mat_t val, size_t row, size_t col, Matrix* m) {
	/*
	if(!m) {
		printf("Cannot set entry of NULL matrix\n");
		return false;
	}
	if(row >= m->rows || col >= m->cols) {
		return false;
	}
	*/
	mat_t* p = TYPIFY(indexMatrix)(row, col, m);
	if(!p) {
		return false;
	}
	*(p) = val;
	
}

mat_t TYPIFY(getMatrixEntry)(size_t row, size_t col, Matrix* m) {
	/*
	if(!m) {
		printf("Cannot get entry from NULL matrix");
		return 0;
	}
	if(row >= m->rows || col >= m->cols) {
		return 0;
	}
	*/
	return *TYPIFY(indexMatrix)(row, col, m);
}

inline mat_t* TYPIFY(indexMatrix)(size_t row, size_t col, Matrix* m) {
	if(!m) {
		printf("Cannot index NULL matrix\n");
		return NULL;
	}
	if(row >= m->rows || col >= m->cols) {
		return NULL;
	}
	return &(m->mat[row * m->cols + col]);
}

bool TYPIFY(writeMatrix)(const char* path, Matrix* m) {
	if(!m) {
		printf("Cannot write NULL matrix\n");
		return false;
	}
	FILE* fp = fopen(path, "w");
	if(!fp) {
		printf("Could not open file to write matrix to: %s\n", path);
		return false;
	}
	
	uint32_t rows = m->rows, cols = m->cols;
	fwrite(&rows, sizeof(uint32_t), 1, fp);
	fwrite(&cols, sizeof(uint32_t), 1, fp);
	size_t numWritten = fwrite(m->mat, sizeof(mat_t), rows * cols, fp);
	fclose(fp);
	if(rows * cols != numWritten) {
		printf("Error when writing image to: %s\n", path);
		return false;
	}
	return true;
}

Matrix* TYPIFY(readMatrix)(const char* path) {
	FILE* fp = fopen(path, "r");
	if(!fp) {
		printf("Could not open file to read matrix from: %s\n", path);
		return NULL;
	}
	
	uint32_t rows, cols;
	fread(&rows, sizeof(uint32_t), 1, fp);
	fread(&cols, sizeof(uint32_t), 1, fp);
	Matrix* m = TYPIFY(allocateMatrix)(rows, cols);
	size_t numRead = fread(m->mat, sizeof(mat_t), rows * cols, fp);
	fclose(fp);
	if(rows * cols != numRead) {
		printf("Error when reading matrix from: %s\n", path);
		return NULL;
	}
	return m;
}

/*bool TYPIFY(readMatrix)(const char* path, Matrix* m) {
	if(!m) {
		printf("Cannot read into NULL matrix\n");
		return false;
	}
	FILE* fp = fopen(path, "r");
	if(!fp) {
		printf("Could not open file to read matrix from: %s\n", path);
		return false;
	}
	
	uint32_t rows, cols;
	fread(&rows, sizeof(uint32_t), 1, fp);
	fread(&cols, sizeof(uint32_t), 1, fp);
	if(m->rows == rows && m->cols == cols) {
		size_t numRead = fread(m->mat, sizeof(mat_t), rows * cols, fp);
		fclose(fp);
		if(rows * cols != numRead) {
			printf("Error when reading matrix from: %s\n", path);
			return false;
		}
	} else {
		printf("Size mismatch when reading matrix\n");
		fclose(fp);
		return false;
	}
	return true;
}*/

bool TYPIFY(printSubmatrix)(size_t rows, size_t cols, size_t row, size_t col, Matrix* m) {
	if(!m) {
		printf("Cannot print NULL matrix\n");
		return false;
	}
	if(row + rows > m->rows || col + cols > m->cols) {
		printf("Size mismatch when printing matrix\n");
		return false;
	}
	
	size_t i, j;
	for(i = 0; i < rows; i++) {
		for(j = 0; j < cols - 1; j++) {
			printf("%f, ", (double)TYPIFY(getMatrixEntry)(i + row, j + col, m));
		}
		printf("%f\n", (double)TYPIFY(getMatrixEntry)(i + row, j + col, m));
	}
	return true;
}

bool TYPIFY(printMatrix)(Matrix* m) {
	return TYPIFY(printSubmatrix)(m->rows, m->cols, 0, 0, m);
}

bool TYPIFY(sumMatrices)(Matrix* dest, Matrix* summand1, Matrix* summand2) {
	if(!dest) {
		printf("Cannot sum matrices into NULL matrix\n");
		return false;
	}
	if(!summand1 || !summand2) {
		printf("Cannot sum with a NULL matrix\n");
		return false;
	}
	if(!TYPIFY(hasSameDimension)(summand1, summand2) || !TYPIFY(hasSameDimension)(summand1, dest)) {
		printf("Size mismatch in summing matrices\n");
		return false;
	}
	size_t i, j;
	for(i = 0; i < dest->rows; i++) {
		for(j = 0; j < dest->cols; j++) {
			TYPIFY(setMatrixEntry)(TYPIFY(getMatrixEntry)(i,j,summand1) + TYPIFY(getMatrixEntry)(i,j,summand2), 
				i, j, dest);
		}
	}
	return true;
	
}

Matrix* TYPIFY(createMatrixFromSum)(Matrix* summand1, Matrix* summand2) {
	if(!TYPIFY(hasSameDimension)(summand1, summand2)) {
		printf("Size mismatch in creating matrix from sum\n");
	}
	Matrix* sum = TYPIFY(allocateMatrix)(summand1->rows, summand1->cols);
	if(TYPIFY(sumMatrices)(sum, summand1, summand2)) {
		return sum;
	} else {
		return NULL;
	}
}

bool TYPIFY(hasSameDimension)(Matrix* m1, Matrix* m2) {
	if(!m1 || !m2) {
		printf("Cannot compare dimensions of a NULL matrix\n");
		return false;
	}
	return (m1->rows == m2->rows) && (m1->cols == m2->cols);
}



bool TYPIFY(findMaxEntryMatrix)(Matrix* m, mat_t* maxVal, size_t* row, size_t* col) {
	if(!m) {
		printf("Cannot find max of NULL matrix\n");
		return false;
	}
	size_t i,j;
	*row = 0;
	*col = 0;
	*maxVal = TYPIFY(getMatrixEntry)(*row, *col, m);
	for(i = 0; i < m->rows; i++) {
		for(j = 0; j < m->cols; j++) {
			mat_t curVal = TYPIFY(getMatrixEntry)(i,j,m);
			if(curVal > *maxVal) {
				*maxVal = curVal;
				*row = i;
				*col = j;
			}
		}
	}
	
	return true;
}

mat_t TYPIFY(getMaxEntryMatrix)(Matrix* m) {
	mat_t maxVal;
	size_t row, col;
	TYPIFY(findMaxEntryMatrix)(m, &maxVal, &row, &col);
	return maxVal;
}

bool TYPIFY(findMinEntryMatrix)(Matrix* m, mat_t* minVal, size_t* row, size_t* col) {
	if(!m) {
		printf("Cannot find max of NULL matrix\n");
		return false;
	}
	size_t i,j;
	*row = 0;
	*col = 0;
	*minVal = TYPIFY(getMatrixEntry)(*row, *col, m);
	for(i = 0; i < m->rows; i++) {
		for(j = 0; j < m->cols; j++) {
			mat_t curVal = TYPIFY(getMatrixEntry)(i,j,m);
			if(curVal < *minVal) {
				*minVal = curVal;
				*row = i;
				*col = j;
			}
		}
	}
	
	return true;
}

mat_t TYPIFY(getMinEntryMatrix)(Matrix* m) {
	mat_t minVal;
	size_t row, col;
	TYPIFY(findMinEntryMatrix)(m, &minVal, &row, &col);
	return minVal;
}



bool TYPIFY(intervalMapMatrix)(Matrix* m, mat_t a1, mat_t b1, mat_t a2, mat_t b2) {
	
	if(!m) {
		printf("Cannot interval map NULL matrix");
		return false;
	}
	if(a1 == b1) {
		printf("Cannot interval map Matrix with empty range (eg a1 == b1)");
		return false;
	}
	
	double s = ((double)b2 - a2) / ((double)b1 - a1);
	double o = -s * a1 + a2;
	
	size_t i,j;
	for(i = 0; i < m->rows; i++) {
		for(j = 0; j < m->cols; j++) {
			TYPIFY(setMatrixEntry)( (mat_t)(TYPIFY(getMatrixEntry)(i,j,m) * s + o), i, j, m);
		}
	}
	return true;
}

bool TYPIFY(scaleRangeMatrix)(Matrix* m, mat_t minVal, mat_t maxVal) {
	mat_t maxEnt = TYPIFY(getMaxEntryMatrix)(m);
	mat_t minEnt = TYPIFY(getMinEntryMatrix)(m);
	
	//if all entries of m are equal, then set all the entries to the midpoint of the desired interval
	if(maxEnt == minEnt) {
		TYPIFY(setAllMatrixEntries)(m, (maxVal + minVal) / 2);
		return true;
	}
	
	return TYPIFY(intervalMapMatrix)(m, minEnt, maxEnt, minVal, maxVal);
}





#undef Matrix
#undef __TYPIFY
#undef _TYPIFY
#undef TYPIFY
