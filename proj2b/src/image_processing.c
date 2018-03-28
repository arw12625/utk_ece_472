

#include "image_processing.h"

#include <stdio.h>
#include <math.h>

im_t getImageMaxChannelValue(size_t channel, Image* im) {
	if(!im) {
		printf("Cannot get max value of NULL image\n");
		return -1;
	}
	if(channel >= im->numChannels) {
		printf("Channel index out of range in finding max value image\n");
		return -1;
	}
	
	im_t maxVal = im->data[channel];
	size_t i;
	for(i = 0; i < im->rows * im->cols * im->numChannels; i += im->numChannels) {
		if(im->data[i] > maxVal) {
			maxVal = im->data[i];
		}
	}
	
	return maxVal;
}

im_t getImageMinChannelValue(size_t channel, Image* im) {
	if(!im) {
		printf("Cannot get min value of NULL image\n");
		return -1;
	}
	if(channel >= im->numChannels) {
		printf("Channel index out of range in finding min value image\n");
		return -1;
	}
	
	im_t minVal = im->data[channel];
	size_t i;
	for(i = 0; i < im->rows * im->cols * im->numChannels; i += im->numChannels) {
		if(im->data[i] < minVal) {
			minVal = im->data[i];
		}
	}
	
	return minVal;
}

im_t getImageMinChannelValue(size_t channel, Image* image);

int sumImages(Image* dest, Image* summand1, Image* summand2) {
	if(!summand1 || !summand2) {
		printf("Cannot sum a NULL image\n");
		return -1;
	}
	if(!dest) {
		printf("Cannot sum into a NULL destination image\n");
		return -1;
	}
	if(!imageHasSameSize(dest, summand1) || !imageHasSameSize(summand1, summand2)) {
		printf("Cannot sum images with different dimensions\n");
		return -1;
	}
	if(dest->numChannels != summand1->numChannels || summand1->numChannels != summand2->numChannels) {
		printf("Cannot sum images with a different number of channels\n");
		return -1;
	}
	
	size_t i;
	size_t numEntries = dest->rows * dest->cols * dest->numChannels;
	for(i = 0; i < numEntries; i++) {
		dest->data[i] = summand1->data[i] + summand2->data[i];
	}
	return 0;
}

Image* createImageFromSum(Image* summand1, Image* summand2) {
	if(!summand1 || !summand2) {
		printf("Cannot sum a NULL image\n");
		return NULL;
	}
	if(!imageHasSameSize(summand1, summand2)) {
		printf("Cannot sum images with different dimensions\n");
		return NULL;
	}
	if(summand1->numChannels != summand2->numChannels) {
		printf("Cannot sum images with a different number of channels\n");
		return NULL;
	}
	Image* dest = allocateImage(summand1->rows, summand1->cols, summand1->numChannels);
	//Channel model of created image defaults to that of the first summand
	dest->channelModel = summand1->channelModel;
	sumImages(dest, summand1, summand2);
	return dest;
}

int scaleImage(float scaler, Image* im) {
	if(!im) {
		printf("Cannot scale NULL image\n");
		return -1;
	}
	Image* dest = allocateImage(im->rows, im->cols, im->numChannels);
	size_t i;
	size_t numEntries = dest->rows * dest->cols * dest->numChannels;
	for(i = 0; i < numEntries; i++) {
		dest->data[i] = (im_t)(im->data[i] * scaler);
	}
	return 0;
}

int scaleImageChannel(float scaler, size_t channel, Image* im) {
	if(!im) {
		printf("Cannot scale NULL image\n");
		return -1;
	}
	if(channel >= im->numChannels) {
		printf("Channel index out of range in scaling image\n");
		return -1;
	}
	Image* dest = allocateImage(im->rows, im->cols, im->numChannels);
	size_t i;
	size_t numEntries = dest->rows * dest->cols * dest->numChannels;
	for(i = 0; i < numEntries; i+= im->numChannels) {
		dest->data[i + channel] = (im_t)(im->data[i + channel] * scaler);
	}
	return 0;
}

//operates on the values of the image channel by the affine interval map from [a1, b1] to [a2, b2]
int intervalMapImageChannel(im_t a1, im_t b1, im_t a2, im_t b2, size_t channel, Image* im) {
	if(!im) {
		printf("Cannot interval map NULL image\n");
		return -1;
	}
	if(channel >= im->numChannels) {
		printf("Channel index out of range in interval mapping image channel\n");
		return -1;
	}
	if(a1 == b1) {
		printf("Cannot interval map image with empty range (eg a1 == b1)");
		return -1;
	}
	
	float s = ((float)b2 - a2) / ((float)b1 - a1);
	float o = -s * a1 + a2;
	
	size_t i;
	size_t numEntries = im->rows * im->cols * im->numChannels;
	for(i = 0; i < numEntries; i+= im->numChannels) {
		im->data[i + channel] = (im_t)(im->data[i + channel] * s + o);
	}
	return 0;
}

//interval maps the pixels of the image from the range of the matrix to the desired range
//eg maps the mininum of m to minVal and the maximum of m to maxVal
int scaleRangeImageChannel(im_t minVal, im_t maxVal, size_t channel, Image* im) {
	if(!im) {
		printf("Cannot scale range of NULL image\n");
		return -1;
	}
	if(channel >= im->numChannels) {
		printf("Channel index out of range in scaling range of image channel\n");
		return -1;
	}
	im_t curMaxVal = getImageMaxChannelValue(channel, im);
	im_t curMinVal = getImageMinChannelValue(channel, im);
	
	//if all values of the image channel are equal, set all entries to the average of the desired min and max
	if(curMaxVal == curMinVal) {
		return setAllImageChannelValues((curMaxVal + curMinVal) / 2, channel, im);
	} else {
		return intervalMapImageChannel(curMinVal, curMaxVal, minVal, maxVal, channel, im);
	}
	
}

//scale all channels of the image independently into the given range
int scaleRangeImageIndependent(im_t minVal, im_t maxVal, Image* im) {
	if(!im) {
		printf("Cannot scale range of NULL image\n");
		return -1;
	}
	size_t i;
	bool success = true;
	for(i = 0; i < im->numChannels; i++) {
		success = success && (scaleRangeImageChannel(minVal, maxVal, i, im) >= 0);
	}
	return success ? 0 : -1;
}

//apply a uniform scaling operation across all channels into the given range
int scaleRangeImageUniform(im_t minVal, im_t maxVal, Image* im) {
	if(!im) {
		printf("Cannot scale range of NULL image\n");
		return -1;
	}
	im_t imageMin = getImageMinChannelValue(0, im);
	im_t imageMax = getImageMaxChannelValue(0, im);
	size_t i;
	for(i = 1; i< im->numChannels; i++) {
		im_t curMax = getImageMaxChannelValue(i, im);
		imageMax = curMax > imageMax ? curMax : imageMax;
		im_t curMin = getImageMinChannelValue(i, im);
		imageMin = curMin < imageMin ? curMin : imageMin;
	}
	
	float s = ((float)maxVal - minVal) / ((float)imageMax - imageMin);
	float o = -s * imageMin + minVal;
	
	size_t numEntries = im->rows * im->cols * im->numChannels;
	for(i = 0; i < numEntries; i++) {
		im->data[i] = (im_t)(im->data[i] * s + o);
	}
	return 0;
}

int applyChannelLookupTransform(im_t lookup[IMAGE_SCALE], size_t channel, Image* im) {
	if(!im) {
		printf("Cannot apply lookup transform to NULL image\n");
		return -1;
	}
	if(channel >= im->numChannels) {
		printf("Channel index out of range in lookup transform image\n");
		return -1;
	}
	size_t i;
	im_t* curVal;
	for(i = 0; i < im->rows * im->cols * im->numChannels; i += im->numChannels) {
		curVal = &im->data[i + channel];
		if(0 <= *curVal && *curVal < IMAGE_SCALE) {
			*curVal = lookup[*curVal];
		} else {
			*curVal = 0;
		}
	}
	return 0;
}

int applyLookupTransform(im_t lookup[IMAGE_SCALE], Image* im) {
	if(!im) {
		printf("Cannot apply lookup transform to NULL image\n");
		return -1;
	}
	size_t i;
	im_t* curVal;
	for(i = 0; i < im->rows * im->cols * im->numChannels; i++) {
		curVal = &im->data[i];
		if(0 <= *curVal && *curVal < IMAGE_SCALE) {
			*curVal = lookup[*curVal];
		} else {
			*curVal = 0;
		}
	}
	return 0;
}

int invertImage(Image* im) {
	if(!im) {
		printf("Cannot invert NULL image\n");
		return -1;
	}
	im_t lookup[IMAGE_SCALE];
	size_t i;
	for(i = 0; i < IMAGE_SCALE; i++) {
		lookup[i] = IMAGE_SCALE - 1 - i;
	}
	
	return applyLookupTransform(lookup, im);
}

int exponentiateImage(Image* im) {
	if(!im) {
		printf("Cannot exponentiate NULL image\n");
		return -1;
	}
	
	//the base of the exponential
	//defined by (IMAGE_SCALE + 1)^(1/IMAGE_SCALE) = 256^(1/255)
	float base = 1.0219839568909338615325105231135;
	im_t lookup[IMAGE_SCALE];
	size_t i;
	for(i = 0; i < IMAGE_SCALE; i++) {
		lookup[i] = (im_t)(pow(base, i) - 1);
	}
	
	return applyLookupTransform(lookup, im);
}

int logarithmImage(Image* im) {
	if(!im) {
		printf("Cannot log NULL image\n");
		return -1;
	}
	
	//the scaling factor for the logarithm
	//defined by (IMAGE_SCALE - 1) / ln(IMAGE_SCALE + 1) = 255 / ln(256)
	float mult = 45.985904428335708609597599206935;
	im_t lookup[IMAGE_SCALE];
	size_t i, j;
	for(i = 0; i < IMAGE_SCALE; i++) {
		lookup[i] = (im_t)(mult * log(i + 1));
	}
	
	return applyLookupTransform(lookup, im);
}


int powerLawImage(Image* im, float gamma) {
	if(!im) {
		printf("Cannot power law transform NULL image\n");
		return -1;
	}
	
	float c = pow(IMAGE_SCALE - 1, 1 - gamma);
	
	im_t lookup[IMAGE_SCALE];
	size_t i, j;
	for(i = 0; i < IMAGE_SCALE; i++) {
		lookup[i] = (im_t)(pow(i, gamma) * c);
	}

	return applyLookupTransform(lookup, im);
}

int computeImageChannelHistogram(float histogram[IMAGE_SCALE], size_t channel, Image* im) {
	if(!im) {
		printf("Cannot compute histogram of NULL image\n");
		return false;
	}
	if(channel >= im->numChannels) {
		printf("Channel index out of range in computing image channel histogram\n");
		return false;
	}
	size_t i;
	size_t imSize = im->rows * im->cols;
	//initialize the histogram
	for(i = 0; i < IMAGE_SCALE; i++) {
		histogram[i] = 0;
	}
	im_t curVal;
	//compute the histogram
	for(i = 0; i < im->numChannels * imSize; i += im->numChannels) {
		curVal = im->data[i];
		if(0 <= curVal && curVal < IMAGE_SCALE) {
			histogram[curVal]++;
		} else {
			printf("Image channel value out of range in computing histogram\n");
			return -1;
		}
	}
	//normalize the histogram by the number of pixels
	for(i = 0; i < IMAGE_SCALE; i++) {
		histogram[i] /= imSize;
	}
	return 0;
}

int computeAndWriteImageChannelHistogram(const char* path, size_t channel, Image* im) {
	if(!im) {
		printf("Cannot compute and write histogram of NULL image\n");
		return -1;
	}
	float histogram[IMAGE_SCALE];
	computeImageChannelHistogram(histogram, channel, im);
	
	FILE *histoFile;
    histoFile = fopen(path,"wb");
    if (histoFile == NULL) {
		printf("Could not open file to write histogram: %s\n", path);
        return -1;
	}

	size_t i;
	for(i = 0; i < IMAGE_SCALE - 1; i++) {
		fprintf(histoFile, "%f, ", histogram[i]);
	}
	fprintf(histoFile, "%f", histogram[i]);
	fclose(histoFile);
	
	return 0;
}

int histogramEqualizeImageChannel(size_t channel, Image* source) {
	
	if(!source) {
		printf("Cannot equalize NULL image");
		return -1;
	}
	if(channel >= source->numChannels) {
		printf("Channel index out of range in histogram equalizing image channel\n");
		return -1;
	}
	
	float histogram[IMAGE_SCALE];
	if(computeImageChannelHistogram(histogram, channel, source) < 0) {
		return -1;
	}
	
	size_t i, j;
	float cdf[IMAGE_SCALE];
	cdf[0] = histogram[0];
	for(i = 1; i < IMAGE_SCALE; i++) {
		cdf[i] = cdf[i - 1] + histogram[i];
	}
	im_t scaledCDF[IMAGE_SCALE];
	for(i = 0; i < IMAGE_SCALE; i++) {
		scaledCDF[i] = (im_t)((IMAGE_SCALE - 1) * cdf[i]);
	}
	
	applyChannelLookupTransform(scaledCDF, channel, source);
	//apply image scaling
	scaleRangeImageChannel(0, IMAGE_SCALE - 1, channel, source);
	
	return 0;	
}

int applyImageChannelKernel(size_t kernelRows, size_t kernelCols,
		float kernel[kernelRows][kernelCols], size_t channel, Image* im) {
	
	if(!im) {
		printf("Cannot apply kernel transform to NULL image\n");
		return false;
	}
	if(channel >= im->numChannels) {
		printf("Channel index out of range in applying image kernel\n");
		return false;
	}
	if(!kernel) {
		printf("Cannot apply NULL kernel transform to image\n");
		return false;
	}
	if(kernelRows % 2 != 1 || kernelCols % 2 != 1) {
		printf("Can only apply odd-dimensioned (symmetric) kernels\n");
		return false;
	}
	
	Image* tempIm = duplicateImage(im);
	
	size_t i, j, k, l;
	size_t kernelDRow = kernelRows / 2, kernelDCol = kernelCols / 2;
	
	float val;
	for(i = 0; i < im->rows; i++) {
		for(j = 0; j < im->cols; j++) {
			val = 0;
			for(k = 0; k < kernelRows; k++) {
				for(l = 0; l < kernelCols; l++) {
					if(i + k >= kernelDRow && i + k < im->rows + kernelDRow
						&& j + l >= kernelDCol && j + l < im->cols + kernelDCol) {
							im_t pixVal = getImageValue(i + k - kernelDRow, j + l - kernelDCol, channel, im);
							val += kernel[k][l] * pixVal;
					}
				}
			}
			setImageValue((im_t)val, i, j, channel, tempIm);
		}
	}
	
	copyImageData(im, tempIm);
	
	//scaleRangeImageChannel(0, IMAGE_SCALE - 1, channel, im);
	
	freeImage(tempIm);
	
	return 0;
	
}

Image* computeGradientImage(Image* im) {
	
	if(!im) {
		printf("Cannot compute gradient of NULL image\n");
		return NULL;
	}
	
	Image* gradIm = allocateImage(im->rows, im->cols, im->numChannels);
	gradIm->channelModel = im->channelModel;
	
	size_t i, j, channel;
	float val;
	float dx, dy;
	im_t xp, xn, yp, yn;
	
	for(i = 1; i < im->rows - 1; i++) {
		for(j = 1; j < im->cols - 1; j++) {
			for(channel = 0; channel < im->numChannels; channel++) {
				
				xp = getImageValue(i + 1, j, channel, im);
				xn = getImageValue(i - 1, j, channel, im);
				yp = getImageValue(i, j + 1, channel, im);
				yn = getImageValue(i, j - 1, channel, im);
				
				dx = xp - xn;
				dy = yp - yn;
				
				val = 0.25 * sqrt(dx * dx + dy * dy);
				setImageValue((im_t)val, i, j, channel, gradIm);
			}
		}
	}
	
	for(i = 0; i < im->rows; i++) {
		for(channel = 1; channel < im->numChannels; channel++) {
			setImageValue(0, i, 0, channel, gradIm);
			setImageValue(0, i, im->cols - 1, channel, gradIm);
		}
	}
	for(i = 0; i < im->cols; i++) {
		for(channel = 1; channel < im->numChannels; channel++) {		
			setImageValue(0, 0, i, channel, gradIm);
			setImageValue(0, im->rows - 1, i, channel, gradIm);
		}
	}

	return gradIm;
}

Image* computeGradientImageApprox(Image* im) {
	
	if(!im) {
		printf("Cannot compute gradient of NULL image\n");
		return NULL;
	}
	
	Image* gradIm = allocateImage(im->rows, im->cols, im->numChannels);
	gradIm->channelModel = im->channelModel;
	
	size_t i, j, channel;
	float val;
	float dx, dy;
	im_t xp, xn, yp, yn;
	
	for(i = 1; i < im->rows - 1; i++) {
		for(j = 1; j < im->cols - 1; j++) {
			for(channel = 1; channel < im->numChannels; channel++) {
				
				xp = getImageValue(i + 1, j, channel, im);
				xn = getImageValue(i - 1, j, channel, im);
				yp = getImageValue(i, j + 1, channel, im);
				yn = getImageValue(i, j - 1, channel, im);
				
				dx = xp - xn;
				dy = yp - yn;
				
				val = fabs(dx) + fabs(dy);
				
				setImageValue((im_t)val, i, j, channel, gradIm);
			}
		}
	}
	
	for(i = 0; i < im->rows; i++) {
		for(channel = 1; channel < im->numChannels; channel++) {
			setImageValue(0, i, 0, channel, gradIm);
			setImageValue(0, i, im->cols - 1, channel, gradIm);
		}
	}
	for(i = 0; i < im->cols; i++) {
		for(channel = 1; channel < im->numChannels; channel++) {		
			setImageValue(0, 0, i, channel, gradIm);
			setImageValue(0, im->rows - 1, i, channel, gradIm);
		}
	}

	return gradIm;
}

int imageAbsoluteValue(Image* im) {
	
	if(!im) {
		printf("Cannot take absolute value of NULL image\n");
		return -1;
	}
	
	size_t i = 0;
	for(i = 0; i < im->rows * im->cols * im->numChannels; i++) {
		im->data[i] = fabs(im->data[i]);
	}
	
	return 0;
}
