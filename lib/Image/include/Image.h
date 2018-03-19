#ifndef _IMAGE_H_
#define _IMAGE_H_

#include <stdint.h>
#include <stdbool.h>

#include "Matrix_i16.h"

#define IMAGE_SCALE 256

typedef enum ImageChannelModel {
	CHANNEL_MODEL_NONE, CHANNEL_MODEL_GRAY, CHANNEL_MODEL_RGB, CHANNEL_MODEL_RGBA
} ImageChannelModel;

#define CHANNEL_GRAY 0
#define CHANNEL_RED 0
#define CHANNEL_GREEN 1
#define CHANNEL_BLUE 2
#define CHANNEL_ALPHA 3

typedef struct Image {
	size_t rows, cols;
	size_t numChannels;
	ImageChannelModel channelModel;
	Matrix_i16** channels;
} Image;
typedef int16_t im_t;

Image* allocateImage(size_t rows, size_t cols, size_t numChannels);
Image* allocateImageWithModel(size_t rows, size_t cols, ImageChannelModel model);
bool freeImage(Image* im);

Image* wrapMatricesIntoImage(size_t numChannels, Matrix_i16** m);
Image* createImageFromMatrices(size_t numChannels, Matrix_i16** m);
//Image* wrapEntriesIntoImage(size_t rows, size_t cols, im_t* dat);
Image* createImageFromEntries(size_t rows, size_t cols, size_t numChannels, im_t** dat);
bool copyImageData(Image* dest, Image* source);
Image* duplicateImage(Image* source);

bool setImageChannelValue(im_t val, size_t row, size_t col, size_t channel, Image* m);
im_t getImageChannelValue(size_t row, size_t col, size_t channel, Image* m);
//indexImage returns the pointer to the entry of m at the specified row and column and channel
im_t* indexImageChannel(size_t row, size_t col, size_t channel, Image* m);
bool setSubImageChannel(size_t rows, size_t cols,
	size_t destRow, size_t destCol, size_t destChannel, Image* dest,
	size_t sourceRow, size_t sourceCol, size_t sourceChannel, Image* source);
bool setSubImage(size_t rows, size_t cols,
	size_t destRow, size_t destCol, Image* dest,
	size_t sourceRow, size_t sourceCol, Image* source);
Image* createSubImage(size_t rows, size_t cols, size_t row, size_t col, Image* source);

bool setAllImageValues(im_t val, Image* im);
bool setAllImageChannelValues(im_t val, size_t channel, Image* im);

bool printSubImageChannel(size_t rows, size_t cols, size_t row, size_t col, size_t channel, Image* im);
bool printImageChannel(size_t channel, Image* im);
bool printSubImage(size_t rows, size_t cols, size_t row, size_t col, Image* im);
bool printImage(Image* im);

bool imageHasSameSize(Image* im1, Image* im2);
bool imageHasSameChannelModel(Image* im1, Image* im2);


#endif