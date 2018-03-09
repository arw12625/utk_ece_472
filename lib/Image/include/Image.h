#ifndef _IMAGE_H_
#define _IMAGE_H_

#include <stdint.h>
#include <stdbool.h>

#include "Matrix_i16.h"

#define IMAGE_SCALE 256

typedef struct Image {
	Matrix_i16* pixels;
} Image;
typedef int16_t im_t;

Image* allocateImage(size_t rows, size_t cols);
bool freeImage(Image* im);

Image* wrapMatrixIntoImage(Matrix_i16* m);
Image* createImageFromMatrixEntries(Matrix_i16* m);
//Image* wrapEntriesIntoImage(size_t rows, size_t cols, im_t* dat);
Image* createImageFromEntries(size_t rows, size_t cols, im_t* dat);
bool copyImagePixels(Image* dest, Image* source);
Image* duplicateImage(Image* source);

bool setImagePixel(im_t val, size_t row, size_t col, Image* m);
im_t getImagePixel(size_t row, size_t col, Image* m);
//indexImage returns the pointer to the entry of m at the specified row and column
im_t* indexImage(size_t row, size_t col, Image* m);
bool setSubImage(size_t rows, size_t cols, size_t destRow, size_t destCol, Image* dest, size_t sourceRow, size_t sourceCol, Image* source);
Image* createSubImage(size_t rows, size_t cols, size_t row, size_t col, Image* source);

bool setAllImagePixels(Image* m, im_t val);

bool printSubImage(size_t rows, size_t cols, size_t row, size_t col, Image* m);
bool printImage(Image* m);

bool imageHasSameSize(Image* m1, Image* m2);


#endif