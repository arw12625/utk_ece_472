#include "image.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Image* allocateEmptyImage() {
	Image* im = malloc(sizeof(Image));
	im->rows = 0;
	im->cols = 0;
	im->numChannels = 0;
	im->channelModel = CHANNEL_MODEL_NONE;
	im->data = NULL;
	return im;
}

Image* allocateImage(size_t rows, size_t cols, size_t numChannels) {
	if(rows == 0 || cols == 0) {
		printf("Cannot allocate image with zero rows or columns\n");
		return NULL;
	}
	if(numChannels == 0) {
		printf("Cannot allocate image with zero channels \n");
		return NULL;
	}
	
	Image* im = allocateEmptyImage();
	im->rows = rows;
	im->cols = cols;
	im->numChannels = numChannels;
	im->data = malloc(sizeof(im_t) * rows * cols * numChannels);
	return im;
}

Image* allocateImageWithModel(size_t rows, size_t cols, ImageChannelModel model) {
	size_t numChannels = 0;
	switch(model) {
		case CHANNEL_MODEL_GRAY :
			numChannels = 1; break;
		case CHANNEL_MODEL_RGB :
			numChannels = 3; break;
		case CHANNEL_MODEL_RGBA :
			numChannels = 4; break;
	}
	Image* im = allocateImage(rows, cols, numChannels);
	im->channelModel = model;
	return im;
}

int freeImage(Image* im) {
	if(!im) {
		printf("Cannot free NULL image\n");
		return false;
	}
	
	free(im->data);
	im->data = NULL;
	free(im);
	return 0;
}

Image* wrapEntriesIntoImage(size_t rows, size_t cols, size_t numChannels, im_t* data) {
	if(rows == 0 || cols == 0) {
		printf("Cannot wrap entries into image with zero rows or columns\n");
		return NULL;
	}
	if(numChannels == 0) {
		printf("Cannot wrap entries into image with zero channels \n");
		return NULL;
	}
	if(!data) {
		printf("Cannot wrap entries into image with NULL entries\n");
		return NULL;
	}
	Image* im = allocateEmptyImage();
	im->rows = rows;
	im->cols = cols;
	im->numChannels = numChannels;
	im->data = data;
	return im;
}

Image* createImageFromEntries(size_t rows, size_t cols, size_t numChannels, im_t* data) {
	if(rows == 0 || cols == 0) {
		printf("Cannot create image from entries with zero rows or columns\n");
		return NULL;
	}
	if(numChannels == 0) {
		printf("Cannot create image from entries  with zero channels \n");
		return NULL;
	}
	if(!data) {
		printf("Cannot create image from entries with NULL entries\n");
		return NULL;
	}
	
	Image* im = allocateImage(rows, cols, numChannels);
	
	memcpy(im->data, data, rows * cols * numChannels * sizeof(im_t));
	
	return im;
}

Image* createImageFromBytes(size_t rows, size_t cols, size_t numChannels, uint8_t* data) {
	if(rows == 0 || cols == 0) {
		printf("Cannot create image from entries with zero rows or columns\n");
		return NULL;
	}
	if(numChannels == 0) {
		printf("Cannot create image from entries  with zero channels \n");
		return NULL;
	}
	if(!data) {
		printf("Cannot create image from entries with NULL entries\n");
		return NULL;
	}
	
	Image* im = allocateImage(rows, cols, numChannels);
	
	size_t i;
	for(i = 0; i < rows * cols * numChannels; i++) {
		im->data[i] = (im_t)data[i];
	}
	
	return im;
}

int getBytesFromImage(uint8_t* data, Image* im) {
	
	if(!im) {
		printf("Cannot get bytes from NULL image\n");
		return -1;
	}
	
	size_t i;
	for(i = 0; i < im->rows * im->cols * im->numChannels; i++) {
		data[i] = (uint8_t)im->data[i];
	}
	
	return 0;
	
}

int copyImageData(Image* dest, Image* source) {
	if(!dest || !source) {
		printf("Cannot copy NULL images\n");
		return -1;
	}
	if(!imageHasSameSize(dest, source) || dest->numChannels != source->numChannels) {
		printf("Cannot copy source image to destination with different size or number of channels\n");
		return -1;
	}
	memcpy(dest->data, source->data,
		source->rows * source->cols * source->numChannels * sizeof(im_t));
	dest->channelModel = source->channelModel;
	return 0;
}

Image* duplicateImage(Image* source) {
	if(!source) {
		printf("Cannot duplicate NULL image\n");
		return NULL;
	}
	Image* dup = allocateImage(source->rows, source->cols, source->numChannels);
	copyImageData(dup, source);
	return dup;
}

int setSubImage(size_t rows, size_t cols,
	size_t destRow, size_t destCol, Image* dest,
	size_t sourceRow, size_t sourceCol, Image* source) {
	
	if(!source) {
		printf("Cannot set sub-image of NULL image\n");
		return -1;
	}
	if(rows + sourceRow > source->rows || cols + sourceCol > source->cols
		|| rows + destRow > dest->rows || cols + destCol > dest->cols) {
		
		printf("Size mismatch when setting sub-image\n");
		return -1;
	}
	
	size_t i;
	for(i = 0; i < rows; i++) {
		memcpy(indexImage(i + destRow, destCol, dest),
			indexImage(i + sourceRow, sourceCol, source),
			sizeof(im_t) * cols * source->numChannels); 
	}
	return 0;
}
	
Image* createSubImage(size_t rows, size_t cols, size_t row, size_t col, Image* source) {
	if(!source) {
		printf("Cannot create sub-image of NULL image\n");
		return NULL;
	}
	if(rows + row > source->rows || cols + col > source->cols) {
		printf("Size mismatch in creating sub-image\n");
		return NULL;
	}
	Image* subImage = allocateImage(rows, cols, source->numChannels);
	subImage->channelModel = source->channelModel;
	
	if(setSubImage(rows, cols, 0, 0, subImage, row, col, source) < 0) {
		return NULL;
	} else {
		return subImage;
	}
}

int setAllImageChannelValues(im_t val, size_t channel, Image* im) {
	if(!im) {
		printf("Cannot set all image values of NULL image");
		return -1;
	}
	if(channel >= im->numChannels) {
		printf("Channel index out of range in setting all values of the image channel\n");
		return -1;
	}
	size_t i;
	for(i = 0; i < im->rows * im->cols * im->numChannels; i+= im->numChannels) {
		im->data[i + channel] = val;
	}
	return 0;
}

int setAllImageValues(im_t val, Image* im) {
	if(!im) {
		printf("Cannot set all image values of NULL image");
		return -1;
	}
	size_t i;
	for(i = 0; i < im->rows * im->cols * im->numChannels; i++) {
		im->data[i] = val;
	}
	return 0;
}
int setAllImagePixels(im_t* val, Image* im) {
	if(!im) {
		printf("Cannot set all image pixels of NULL image");
		return -1;
	}
	size_t i,j;
	for(i = 0; i < im->rows * im->cols * im->numChannels; i+= im->numChannels) {
		for(j = 0; j < im->numChannels; j++) {
			im->data[i + j] = val[j];
		}
	}
	return 0;
}

int printSubImage(size_t rows, size_t cols, size_t row, size_t col, Image* im) {
	if(!im) {
		printf("Cannot create sub-image of NULL image\n");
		return -1;
	}
	if(rows + row > im->rows || cols + col > im->cols) {
		printf("Size mismatch in creating sub-image\n");
		return -1;
	}
	
	size_t i,j, channel;
	for(channel = 0; channel < im->numChannels; channel++) {
		printf("Channel %d\n", channel);
		for(i = 0; i < rows; i++) {
			for(j = 0; j < cols - 1; j++) {
				printf("%d, ", getImageValue(i + row, j + col, channel, im));
			}
			printf("%d\n", getImageValue(i + row, j + col, channel, im));
		}
		printf("\n");
	}
	return 0;
}

int printImage(Image* im) {
	return printSubImage(im->rows, im->cols, 0, 0, im);
}

bool imageHasSameSize(Image* im1, Image* im2) {
	return (im1->rows == im2->rows && im1->cols == im2->cols);
}