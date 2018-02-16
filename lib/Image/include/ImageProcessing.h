#ifndef _IMAGE_PROCESSING_H_
#define _IMAGE_PROCESSING_H_

#include <stdint.h>
#include <stdbool.h>

#include "Image.h"

bool sumImages(Image* dest, Image* summand1, Image* summand2);
//Image* createImageFromSum(Image* summand1, Image* summand2);

bool scaleImage(Image* m, float scaler);
bool multiplyImages(Image* dest, Image* m1, Image* m2);

//operates on the pixels of the image by the affine interval map from [a1, b1] to [a2, b2]
bool intervalMapImage(Image* m, im_t a1, im_t b1, im_t a2, im_t b2);

//interval maps the pixels of the image from the range of the matrix to the desired range
//eg maps the mininum of m to minVal and the maximum of m to maxVal
bool scaleRangeImage(Image* m, im_t minVal, im_t maxVal);

bool applyLookupTransform(Image* im, im_t lookup[IMAGE_SCALE]);

bool invertImagePixels(Image* im);

bool exponentiateImagePixels(Image* im);
bool logarithmImagePixels(Image* im);
bool powerLawImagePixels(Image* im, double gamma);

bool computeImageHistogram(Image* im, double histogram[IMAGE_SCALE]);
bool computeAndWriteImageHistogram(Image* im, const char* path);
Image* generateHistogramPlot(double histogram[IMAGE_SCALE]);
bool histogramEqualizeImage(Image* source);

#endif