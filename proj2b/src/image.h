#ifndef _IMAGE_H_
#define _IMAGE_H_

#include <stdint.h>
#include <stdbool.h>

#define IMAGE_SCALE 256

typedef enum ImageChannelModel {
	CHANNEL_MODEL_NONE, CHANNEL_MODEL_GRAY, CHANNEL_MODEL_RGB, CHANNEL_MODEL_RGBA
} ImageChannelModel;

#define CHANNEL_GRAY 0
#define CHANNEL_RED 0
#define CHANNEL_GREEN 1
#define CHANNEL_BLUE 2
#define CHANNEL_ALPHA 3

typedef int16_t im_t;

typedef struct Image {
	size_t rows;
	size_t cols;
	size_t numChannels;
	ImageChannelModel channelModel;
	im_t* data;
} Image;

Image* allocateImage(size_t rows, size_t cols, size_t numChannels);
Image* allocateImageWithModel(size_t rows, size_t cols, ImageChannelModel model);
int freeImage(Image* im);

Image* wrapEntriesIntoImage(size_t rows, size_t cols, size_t numChannels, im_t* data);
Image* createImageFromEntries(size_t rows, size_t cols, size_t numChannels, im_t* data);

Image* createImageFromBytes(size_t rows, size_t cols, size_t numChannels, uint8_t* data);
int getBytesFromImage(uint8_t* data, Image* im);

int copyImageData(Image* dest, Image* source);
Image* duplicateImage(Image* source);

int setSubImage(size_t rows, size_t cols,
	size_t destRow, size_t destCol, Image* dest,
	size_t sourceRow, size_t sourceCol, Image* source);
Image* createSubImage(size_t rows, size_t cols, size_t row, size_t col, Image* source);

int setAllImageChannelValues(im_t val, size_t channel, Image* im);
int setAllImageValues(im_t val, Image* im);
int setAllImagePixels(im_t* val, Image* im);

int printSubImage(size_t rows, size_t cols, size_t row, size_t col, Image* im);
int printImage(Image* im);

bool imageHasSameSize(Image* im1, Image* im2);

static inline im_t* indexImage(size_t row, size_t col, Image* im) {
	return &(im->data[im->numChannels * (im->cols * row  + col)]);
}

static inline im_t getImageValue(size_t row, size_t col, size_t channel, Image* im) {
	return im->data[im->numChannels * (im->cols * row  + col) + channel];
}
static inline void setImageValue(im_t value, size_t row, size_t col, size_t channel, Image* im) {
	im->data[im->numChannels * (im->cols * row  + col) + channel] = value;
}

#endif