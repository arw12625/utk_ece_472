#include "Image.h"
#include <stdio.h>

Image* allocateEmptyImage() {
	Image* im = malloc(sizeof(Image));
	return im;
}

Image* allocateImage(size_t rows, size_t cols) {
	Image* im = allocateEmptyImage();
	im->pixels = allocateMatrix_u16(rows, cols);
	return im;
}

bool freeImage(Image* im) {
	if(!im) {
		printf("Cannot free NULL image\n");
		return false;
	}
	bool freed = freeMatrix_u16(im->pixels);
	im->pixels = NULL;
	free(im);
	return freed;
}

Image* wrapMatrixIntoImage(Matrix_u16* m) {
	if(!m) {
		printf("Cannot wrap NULL matrix into Image");
		return NULL;
	}
	Image* im = allocateEmptyImage();
	im->pixels = m;
	return im;
}

Image* createImageFromMatrixEntries(Matrix_u16* m) {
	if(!m) {
		printf("Cannot create Image from Null matrix");
		return NULL;
	}
	Image* im = allocateImage(m->rows, m->cols);
	copyMatrixEntries_u16(im->pixels, m);
	return im;
}

Image* wrapEntriesIntoImage(size_t rows, size_t cols, im_t* dat) {
	if(!dat) {
		printf("Cannot wrap NULL entry array into Image");
		return NULL;
	}
	Image* im = allocateEmptyImage();
	Matrix_u16* pix = wrapEntriesIntoMatrix_u16(rows, cols, dat);
	im->pixels = pix;
	return im;
}

Image* createImageFromEntries(size_t rows, size_t cols, im_t* dat) {
	if(!dat) {
		printf("Cannot create Image from Null entry array\n");
		return NULL;
	}
	Image* im = allocateEmptyImage();
	Matrix_u16* pix = createMatrixFromEntries_u16(rows, cols, dat);
	im->pixels = pix;
	return im;
}

bool copyImagePixels(Image* dest, Image* source) {
	if(!dest || !source) {
		printf("Cannot copy NULL images\n");
		return false;
	}
	return copyMatrixEntries_u16(dest->pixels, source->pixels);
}

Image* duplicateImage(Image* source) {
	if(!source) {
		printf("Cannot duplicate NULL image");
		return NULL;
	}
	Image* dup = allocateImage(source->pixels->rows, source->pixels->cols);
	copyImagePixels(dup, source);
	return dup;
}

bool setImagePixel(im_t val, size_t row, size_t col, Image* im) {
	if(!im) {
		printf("Cannot set pixel of NULL image\n");
		return false;
	}
	return setMatrixEntry_u16(val, row, col, im->pixels);
}

im_t getImagePixel(size_t row, size_t col, Image* im) {
	if(!im) {
		printf("Cannot get pixel from NULL image\n");
		return 0;
	}
	return getMatrixEntry_u16(row, col, im->pixels);
}

//indexImage returns the pointer to the pixel at the specified row and column
im_t* indexImage(size_t row, size_t col, Image* im) {
	if(!im) {
		printf("Cannot index NULL image\n");
		return NULL;
	}
	return indexMatrix_u16(row, col, im->pixels);
}

bool setSubImage(size_t rows, size_t cols, size_t destRow, size_t destCol, Image* dest, size_t sourceRow, size_t sourceCol, Image* source) {
	if(!dest) {
		printf("Cannot set sub-image of NULL image\n");
		return false;
	}
	if(!source) {
		printf("Cannot set sub-image from NULL image\n");
		return false;
	}
	return setSubmatrix_u16(rows, cols, destRow, destCol, dest->pixels, sourceRow, sourceCol, source->pixels);
}

Image* createSubImage(size_t rows, size_t cols, size_t row, size_t col, Image* source) {
	if(!source) {
		printf("Cannot create sub-image from NULL image\n");
		return NULL;
	}
	Image* subImage = allocateImage(rows, cols);
	//should probably change this not to allocate new matrix
	Matrix_u16* subMat = createSubmatrix_u16(rows, cols, row, col, source->pixels);
	copyMatrixEntries_u16(subImage->pixels, subMat);
	free(subMat);
	return subImage;
}

bool setAllImagePixels(Image* im, im_t val) {
	if(!im) {
		printf("Cannot set all pixels of NULL image\n");
		return false;
	}
	return setAllMatrixEntries_u16(im->pixels, val);
}

bool printSubImage(size_t rows, size_t cols, size_t row, size_t col, Image* im) {
	if(!im) {
		printf("Cannot print sub-image of NULL image\n");
		return false;
	}
	return printSubmatrix_u16(rows, cols, row, col, im->pixels);
}

bool printImage(Image* im) {
	if(!im) {
		printf("Cannot print NULL image\n");
		return false;
	}
	return printMatrix_u16(im->pixels);
}


bool multiplyImages(Image* dest, Image* im1, Image* im2) {
	if(!dest || !im1 || !im2) {
		printf("Cannot multiply with NULL images\n");
		return false;
	}
	return multiplyMatrices_u16(dest->pixels, im1->pixels, im2->pixels);
}

bool imageHasSameSize(Image* im1, Image* im2) {
	if(!im1 || !im2) {
		printf("Cannot compare size of NULL images\n");
		return false;
	}
	return hasSameDimension_u16(im1->pixels, im2->pixels);
}

