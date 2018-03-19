#include "Image.h"
#include <stdio.h>

Image* allocateEmptyImage() {
	Image* im = malloc(sizeof(Image));
	im->rows = 0;
	im->cols = 0;
	im->numChannels = 0;
	im->channelModel = CHANNEL_MODEL_NONE;
	im->channels = NULL;
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
	im->channels = malloc(sizeof(Matrix_i16*) * numChannels);
	size_t i;
	for(i = 0; i < numChannels; i++) {
		im->channels[i] = allocateMatrix_i16(rows, cols);
	}
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

bool freeImage(Image* im) {
	if(!im) {
		printf("Cannot free NULL image\n");
		return false;
	}
	bool freed = true;
	size_t i;
	for(i = 0; i < im->numChannels; i++) {
		freed = freed && freeMatrix_i16(im->channels[i]);
		im->channels[i] = NULL;
	}
	free(im->channels);
	im->channels = NULL;
	free(im);
	return freed;
}

Image* wrapMatricesIntoImage(size_t numChannels, Matrix_i16** m) {
	if(!m) {
		printf("Cannot wrap NULL matrices into Image\n");
		return NULL;
	}
	if(numChannels == 0) {
		printf("Cannot wrap zero channel matrices into an image \n");
		return NULL;
	}
	if(!(m[0])) {
		printf("Cannot wrap a NULL matrix into an image channel\n");
		return NULL;		
	}
	size_t i;
	size_t rows = m[0]->rows, cols = m[0]->cols;
	for(i = 1; i < numChannels; i++) {
		if(!(m[i])) {
			printf("Cannot wrap a NULL matrix into an image channel\n");
			return NULL;
		}
		if(m[i]->rows != rows || m[i]->cols != cols) {
			printf("Cannot wrap matrices with different dimensions into image\n");
			return NULL;
		}
	}
	Image* im = allocateEmptyImage();
	im->rows = rows;
	im->cols = cols;
	im->numChannels = numChannels;
	im->channels = m;
	return im;
}

Image* createImageFromMatrices(size_t numChannels, Matrix_i16** m) {
	if(!m) {
		printf("Cannot create Image from NULL matrices\n");
		return NULL;
	}
	if(numChannels == 0) {
		printf("Cannot create an image from zero channel matrices\n");
		return NULL;
	}
	if(!(m[0])) {
		printf("Cannot create an image channel with a NULL matrix\n");
		return NULL;
	}
	size_t i;
	size_t rows = m[0]->rows, cols = m[0]->cols;
	for(i = 1; i < numChannels; i++) {
		if(!(m[i])) {
		printf("Cannot create an image channel with a NULL matrix\n");
			return NULL;
		}
		if(m[i]->rows != rows || m[i]->cols != cols) {
			printf("Cannot create an image with channel matrices with different dimensions\n");
			return NULL;
		}
	}
	Image* im = allocateImage(rows, cols, numChannels);
	for(i = 0; i < numChannels; i++) {
		copyMatrixEntries_i16(im->channels[i], m[i]);
	}
	return im;
}

/*
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
*/

Image* createImageFromEntries(size_t rows, size_t cols, size_t numChannels, im_t** dat) {
	if(!dat) {
		printf("Cannot create Image from Null entry array\n");
		return NULL;
	}
	if(numChannels == 0) {
		printf("Cannot create an image from entries with zero channels\n");
		return NULL;
	}
	size_t i;
	for(i = 0; i < numChannels; i++) {
		if(!(dat[i])) {
			printf("Cannot create an image channel with NULL entries\n");
			return NULL;
		}
	}
	Matrix_i16* channels[numChannels];
	for(i = 0; i < numChannels; i++) {
		channels[i] = createMatrixFromEntries_i16(rows, cols, dat[i]);
	}
	
	return createImageFromMatrices(numChannels, channels);
}

bool copyImageData(Image* dest, Image* source) {
	if(!dest || !source) {
		printf("Cannot copy NULL images\n");
		return false;
	}
	if(!imageHasSameSize(dest, source) || dest->numChannels != source->numChannels) {
		printf("Cannot copy source image to destination with different size or number of channels\n");
		return false;
	}
	size_t i;
	bool success = true;
	for(i = 0; i < dest->numChannels; i++) {
		success = success && copyMatrixEntries_i16(dest->channels[i], source->channels[i]);
	}
	dest->channelModel = source->channelModel;
	return success;
}

Image* duplicateImage(Image* source) {
	if(!source) {
		printf("Cannot duplicate NULL image");
		return NULL;
	}
	Image* dup = allocateImage(source->rows, source->cols, source->numChannels);
	copyImageData(dup, source);
	return dup;
}

bool setImageChannelValue(im_t val, size_t row, size_t col, size_t channel, Image* im) {
	if(!im) {
		printf("Cannot set channel value of NULL image\n");
		return false;
	}
	return setMatrixEntry_i16(val, row, col, im->channels[channel]);
}

im_t getImageChannelValue(size_t row, size_t col, size_t channel, Image* im) {
	if(!im) {
		printf("Cannot get channel value from NULL image\n");
		return 0;
	}
	return getMatrixEntry_i16(row, col, im->channels[channel]);
}

//indexImageChannel returns the pointer to the channel value at the specified row and column
im_t* indexImageChannel(size_t row, size_t col, size_t channel, Image* im) {
	if(!im) {
		printf("Cannot index NULL image\n");
		return NULL;
	}
	return indexMatrix_i16(row, col, im->channels[channel]);
}

bool setSubImageChannel(size_t rows, size_t cols,
	size_t destRow, size_t destCol, size_t destChannel, Image* dest,
	size_t sourceRow, size_t sourceCol, size_t sourceChannel, Image* source) {
	if(!dest) {
		printf("Cannot set sub-image channel of NULL image\n");
		return false;
	}
	if(!source) {
		printf("Cannot set sub-image channel from NULL image\n");
		return false;
	}
	if(destChannel >= dest->numChannels || sourceChannel >= source->numChannels) {
		printf("Channel index out of range in source or destination when setting sub-image channel\n");
		return false;
	}
	return setSubmatrix_i16(rows, cols, destRow, destCol, dest->channels[destChannel],
		sourceRow, sourceCol, source->channels[sourceChannel]);
}

bool setSubImage(size_t rows, size_t cols, size_t destRow, size_t destCol, Image* dest,
	size_t sourceRow, size_t sourceCol, Image* source) {
	if(!dest) {
		printf("Cannot set sub-image of NULL image\n");
		return false;
	}
	if(!source) {
		printf("Cannot set sub-image from NULL image\n");
		return false;
	}
	if(dest->numChannels != source->numChannels) {
		printf("Cannot set sub-image of destination from source with a different number of channels\n");
		return false;
	}
	size_t i;
	bool success = true;
	for(i = 0; i < dest->numChannels; i++) {
		success = success && setSubmatrix_i16(rows, cols, destRow, destCol, dest->channels[i],
			sourceRow, sourceCol, source->channels[i]);
	}
	return success;
}

Image* createSubImage(size_t rows, size_t cols, size_t row, size_t col, Image* source) {
	if(!source) {
		printf("Cannot create sub-image from NULL image\n");
		return NULL;
	}
	Image* subImage = allocateImage(rows, cols, source->numChannels);
	subImage->channelModel = source->channelModel;
	setSubImage(rows, cols, 0, 0, subImage, row, col, source);
	return subImage;
}

bool setAllImageChannelValues(im_t val, size_t channel, Image* im) {
	if(!im) {
		printf("Cannot set all channel values of NULL image\n");
		return false;
	}
	return setAllMatrixEntries_i16(im->channels[channel], val);
}

bool setAllImageValues(im_t val, Image* im) {
	if(!im) {
		printf("Cannot set all values of NULL image\n");
		return false;
	}
	size_t i;
	bool success = true;
	for(i = 0; i < im->numChannels; i++) {
		success = success && setAllImageChannelValues(val, i, im);
	}
	return success;
}

bool printSubImageChannel(size_t rows, size_t cols, size_t row, size_t col, size_t channel, Image* im) {
	if(!im) {
		printf("Cannot print sub-image channel of NULL image\n");
		return false;
	}
	if(channel >= im->numChannels) {
		printf("Channel index out of range in printing sub-image channel\n");
		return false;
	}
	return printSubmatrix_i16(rows, cols, row, col, im->channels[channel]);
}

bool printImageChannel(size_t channel, Image* im) {
	if(!im) {
		printf("Cannot print image channel of NULL image\n");
		return false;
	}
	if(channel >= im->numChannels) {
		printf("Channel index out of range in printing image channel\n");
		return false;
	}
	return printMatrix_i16(im->channels[channel]);
}

bool printSubImage(size_t rows, size_t cols, size_t row, size_t col, Image* im) {
	if(!im) {
		printf("Cannot print sub-image of NULL image\n");
		return false;
	}
	size_t i;
	bool success = true;
	for(i = 0; i < im->numChannels; i++) {
		printf("Channel %u\n", i);
		success = success && printSubImageChannel(rows, cols, row, col, i, im);
	}
	return success;
}

bool printImage(Image* im)  {
	if(!im) {
		printf("Cannot print NULL image\n");
		return false;
	}
	size_t i;
	bool success = true;
	for(i = 0; i < im->numChannels; i++) {
		printf("Channel %u\n", i);
		success = success && printImageChannel(i, im);
	}
	return success;
}

bool imageHasSameSize(Image* im1, Image* im2) {
	if(!im1 || !im2) {
		printf("Cannot compare size of NULL images\n");
		return false;
	}
	return im1->rows == im2->rows && im1->cols == im2->cols;
}

bool imageHasSameChannelModel(Image* im1, Image* im2) {
	return im1->channelModel == im2->channelModel;
}

