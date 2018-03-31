#ifndef _IMAGE_PROCESSING_H_
#define _IMAGE_PROCESSING_H_

#include <stdint.h>

#include "image.h"

im_t getImageMaxChannelValue(size_t channel, Image* image);
im_t getImageMinChannelValue(size_t channel, Image* image);

int setImageChannelBorder(im_t val, size_t borderRows, size_t borderCols, size_t channel, Image* im);
int setImageBorder(im_t val, size_t borderRows, size_t borderCols, Image* im);

int sumImages(Image* dest, Image* summand1, Image* summand2);
Image* createImageFromSum(Image* summand1, Image* summand2);

int scaleImage(float scaler, Image* im);
int scaleImageChannel(float scaler, size_t channel, Image* im);

//operates on the values of the image channel by the affine interval map from [a1, b1] to [a2, b2]
int intervalMapImageChannel(im_t a1, im_t b1, im_t a2, im_t b2, size_t channel, Image* im);

//interval maps the pixels of the image from the range of the matrix to the desired range
//eg maps the mininum of m to minVal and the maximum of m to maxVal
int scaleRangeImageChannel(im_t minVal, im_t maxVal, size_t channel, Image* im);

//scale all channels of the image independently into the given range
int scaleRangeImageIndependent(im_t minVal, im_t maxVal, Image* im);
//apply a uniform scaling operation across all channels into the given range
int scaleRangeImageUniform(im_t minVal, im_t maxVal, Image* im);

int applyChannelLookupTransform(im_t lookup[IMAGE_SCALE], size_t channel, Image* im);
int applyLookupTransform(im_t lookup[IMAGE_SCALE], Image* im);

int invertImage(Image* im);
int exponentiateImage(Image* im);
int logarithmImage(Image* im);
int powerLawImage(Image* im, float gamma);

int computeImageChannelHistogram(float histogram[IMAGE_SCALE], size_t channel, Image* im);
int computeAndWriteImageChannelHistogram(const char* path, size_t channel, Image* im);

int histogramEqualizeImageChannel(size_t channel, Image* source);

int applyImageChannelKernel(size_t kernelRows, size_t kernelCols,
		float kernel[kernelRows][kernelCols], size_t channel, Image* im);

int applyImageKernel(size_t kernelRows, size_t kernelCols,
		float kernel[kernelRows][kernelCols], Image* im);

//int applyImageChannelMedianFilter(size_t filterRows, size_t filterCols, size_t channel, Image* im);

Image* computeGradientImage(Image* im);
Image* computeGradientImageApprox(Image* im);

int imageAbsoluteValue(Image* im);

int truncateImageRange(im_t minVal, im_t maxVal, Image* im);

int convertScalerRGB_HSI(float* destHSI, float* sourceRGB);
int convertScalerHSI_RGB(float* destRGB, float* sourceHSI);
int convertPixelRGB_HSI(im_t* destHSI, im_t* sourceRGB);
int convertPixelHSI_RGB(im_t* destRGB, im_t* sourceHSI);

Image* convertImageRGB_HSI(Image* im);
Image* convertImageHSI_RGB(Image* im);

#endif