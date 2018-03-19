#ifndef _IMAGE_PROCESSING_H_
#define _IMAGE_PROCESSING_H_

#include <stdint.h>
#include <stdbool.h>

#include "Image.h"
#include "Matrix_d.h"

bool sumImageChannels(size_t destChannel, Image* dest, size_t summand1Channel,
	Image* summand1, size_t summand2Channel, Image* summand2);
bool sumImages(Image* dest, Image* summand1, Image* summand2);
Image* createImageFromSum(Image* summand1, Image* summand2);

bool scaleImageChannel(float scaler, size_t channel, Image* im);

//operates on the values of the image channel by the affine interval map from [a1, b1] to [a2, b2]
bool intervalMapImageChannel(im_t a1, im_t b1, im_t a2, im_t b2, size_t channel, Image* im);

//interval maps the pixels of the image from the range of the matrix to the desired range
//eg maps the mininum of m to minVal and the maximum of m to maxVal
bool scaleRangeImageChannel(im_t minVal, im_t maxVal, size_t channel, Image* im);
//scale all channels of the image independently into the given range
bool scaleRangeImageIndependent(im_t minVal, im_t maxVal, Image* im);
//apply a uniform scaling operation across all channels into the given range
bool scaleRangeImageUniform(im_t minVal, im_t maxVal, Image* im);

bool applyChannelLookupTransform(im_t lookup[IMAGE_SCALE], size_t channel, Image* im);
bool applyLookupTransform(im_t lookup[IMAGE_SCALE], Image* im);

bool invertImage(Image* im);
bool exponentiateImage(Image* im);
bool logarithmImage(Image* im);
bool powerLawImage(Image* im, double gamma);

bool computeImageChannelHistogram(double histogram[IMAGE_SCALE], size_t channel, Image* im);
bool computeAndWriteImageChannelHistogram(const char* path, size_t channel, Image* im);
Image* generateHistogramPlot(double histogram[IMAGE_SCALE]);
bool histogramEqualizeImageChannel(size_t channel, Image* source);

bool applyImageChannelKernel(Matrix_d* kernel, size_t channel, Image* im);
bool applyImageChannelKernelFromFile(const char* kernelPath, size_t channel, Image* im);
bool applyImageChannelMedianFilter(size_t filterRows, size_t filterCols, size_t channel, Image* im);
bool applyImageKernelIndependent(Matrix_d* kernel, size_t channel, Image* im);
bool applyImageKernelFromFileIndependent(const char* kernelPath, size_t channel, Image* im);

Image* computeGradientImage(Image* im);
Image* computeGradientImageApprox(Image* im);

#endif