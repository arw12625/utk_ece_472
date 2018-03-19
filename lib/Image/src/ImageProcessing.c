#include "ImageProcessing.h"
#include <math.h>
#include <stdio.h>

bool sumImageChannels(size_t destChannel, Image* dest, size_t summand1Channel,
	Image* summand1, size_t summand2Channel, Image* summand2) {
	if(!summand1 || !summand2) {
		printf("Cannot sum a NULL image\n");
		return false;
	}
	if(!dest) {
		printf("Cannot sum into a NULL destination image\n");
		return false;
	}
	if(!imageHasSameSize(dest, summand1) || !imageHasSameSize(summand1, summand2)) {
		printf("Cannot sum images with different dimensions\n");
		return false;
	}
	if(destChannel >= dest->numChannels || summand1Channel >= summand1->numChannels
		|| summand2Channel >= summand2->numChannels) {
		printf("Channel index out of range in summing image channels\n");
		return false;
	}
	return sumMatrices_i16(dest->channels[destChannel],
			summand1->channels[summand1Channel], summand2->channels[summand2Channel]);
}

bool sumImages(Image* dest, Image* summand1, Image* summand2) {
	if(!summand1 || !summand2) {
		printf("Cannot sum a NULL image\n");
		return false;
	}
	if(!dest) {
		printf("Cannot sum into a NULL destination image\n");
		return false;
	}
	if(!imageHasSameSize(dest, summand1) || !imageHasSameSize(summand1, summand2)) {
		printf("Cannot sum images with different dimensions\n");
		return false;
	}
	if(dest->numChannels != summand1->numChannels || summand1->numChannels != summand2->numChannels) {
		printf("Cannot sum images with a different number of channels\n");
		return false;
	}
	size_t channel;
	bool success = true;
	for(channel = 0; channel < dest->numChannels; channel++) {
		success = success && sumMatrices_i16(dest->channels[channel],
			summand1->channels[channel], summand2->channels[channel]);
	}
	return success;
}


Image* createImageFromSum(Image* summand1, Image* summand2) {
	if(!summand1 || !summand2) {
		printf("Cannot sum a NULL image\n");
		return false;
	}
	if(!imageHasSameSize(summand1, summand2)) {
		printf("Cannot sum images with different dimensions\n");
		return false;
	}
	if(summand1->numChannels != summand2->numChannels) {
		printf("Cannot sum images with a different number of channels\n");
		return false;
	}
	Image* dest = allocateImage(summand1->rows, summand1->cols, summand1->numChannels);
	//Channel model of created image defaults to that of the first summand
	dest->channelModel = summand1->channelModel;
	sumImages(dest, summand1, summand2);
	return dest;
	
}

bool scaleImageChannel(float scaler, size_t channel, Image* im) {
	if(!im) {
		printf("Cannot scale NULL image\n");
		return false;
	}
	if(channel >= im->numChannels) {
		printf("Channel index out of range in scaling image\n");
		return false;
	}
	return scaleMatrix_i16(im->channels[channel], scaler);
}

//operates on the values of the image channel by the affine interval map from [a1, b1] to [a2, b2]
bool intervalMapImageChannel(im_t a1, im_t b1, im_t a2, im_t b2, size_t channel, Image* im) {
	if(!im) {
		printf("Cannot interval map NULL image\n");
		return false;
	}
	if(channel >= im->numChannels) {
		printf("Channel index out of range in interval mapping image channel\n");
		return false;
	}
	return intervalMapMatrix_i16(im->channels[channel], a1, b1, a2, b2);
}

//interval maps the pixels of the image from the range of the matrix to the desired range
//eg maps the mininum of m to minVal and the maximum of m to maxVal
bool scaleRangeImageChannel(im_t minVal, im_t maxVal, size_t channel, Image* im) {
	if(!im) {
		printf("Cannot scale range of NULL image\n");
		return false;
	}
	if(channel >= im->numChannels) {
		printf("Channel index out of range in scaling range of image channel\n");
		return false;
	}
	return scaleRangeMatrix_i16(im->channels[channel], minVal, maxVal);
}

bool scaleRangeImageIndependent(im_t minVal, im_t maxVal, Image* im) {
	if(!im) {
		printf("Cannot scale range of NULL image\n");
		return false;
	}
	size_t i;
	bool success = true;
	for(i = 0; i < im->numChannels; i++) {
		success = success && scaleRangeImageChannel(minVal, maxVal, i, im);
	}
	return success;
}

bool scaleRangeImageUniform(im_t minVal, im_t maxVal, Image* im) {
	if(!im) {
		printf("Cannot scale range of NULL image\n");
		return false;
	}
	im_t imageMin = getMinEntryMatrix_i16(im->channels[0]);
	im_t imageMax = getMaxEntryMatrix_i16(im->channels[0]);
	size_t i;
	for(i = 1; i< im->numChannels; i++) {
		im_t curMax = getMaxEntryMatrix_i16(im->channels[i]);
		imageMax = curMax > imageMax ? curMax : imageMax;
		im_t curMin = getMinEntryMatrix_i16(im->channels[i]);
		imageMin = curMin < imageMin ? curMin : imageMin;
	}
	bool success = true;
	for(i = 0; i < im->numChannels; i++) {
		success = success && intervalMapImageChannel(imageMin, imageMax, minVal, maxVal, i, im);
	}
	return success;
}

//Image must be in range to apply lookup transform (eg 0 <= val < IMAGE_SCALE)
bool applyChannelLookupTransform(im_t lookup[IMAGE_SCALE], size_t channel, Image* im) {
	if(!im) {
		printf("Cannot apply lookup transform to NULL image\n");
		return false;
	}
	if(channel >= im->numChannels) {
		printf("Channel index out of range in lookup transform image\n");
		return false;
	}
	size_t i, j;
	size_t curVal;
	for(i = 0; i < im->rows; i++) {
		for(j = 0; j < im->cols; j++) {
			curVal = (size_t)getMatrixEntry_i16(i, j, im->channels[channel]);
			if(curVal < IMAGE_SCALE) {
				curVal = lookup[curVal];
			} else {
				curVal = 0;
			}
			setMatrixEntry_i16((im_t)curVal, i, j, im->channels[channel]);
		}
	}
	return true;
}

bool applyLookupTransform(im_t lookup[IMAGE_SCALE], Image* im) {
	if(!im) {
		printf("Cannot apply lookup transform to NULL image\n");
		return false;
	}
	size_t i;
	bool success = true;
	for(i = 0; i < im->numChannels; i++) {
		success = success && applyChannelLookupTransform(lookup, i, im);
	}
	return success;
}

bool invertImage(Image* im) {
	if(!im) {
		printf("Cannot invert NULL image\n");
		return false;
	}
	im_t lookup[IMAGE_SCALE];
	size_t i;
	for(i = 0; i < IMAGE_SCALE; i++) {
		lookup[i] = IMAGE_SCALE - 1 - i;
	}
	
	return applyLookupTransform(lookup, im);
}

bool exponentiateImagePixels(Image* im) {
	if(!im) {
		printf("Cannot exponentiate NULL image\n");
		return false;
	}
	
	//the base of the exponential
	//defined by (IMAGE_SCALE + 1)^(1/IMAGE_SCALE) = 256^(1/255)
	double base = 1.0219839568909338615325105231135;
	im_t lookup[IMAGE_SCALE];
	size_t i;
	for(i = 0; i < IMAGE_SCALE; i++) {
		lookup[i] = (im_t)(pow(base, i) - 1);
	}
	
	return applyLookupTransform(lookup, im);
}

bool logarithmImagePixels(Image* im) {
	if(!im) {
		printf("Cannot log NULL image\n");
		return false;
	}
	
	//the scaling factor for the logarithm
	//defined by (IMAGE_SCALE - 1) / ln(IMAGE_SCALE + 1) = 255 / ln(256)
	double mult = 45.985904428335708609597599206935;
	im_t lookup[IMAGE_SCALE];
	size_t i, j;
	for(i = 0; i < IMAGE_SCALE; i++) {
		lookup[i] = (im_t)(mult * log(i + 1));
	}
	
	return applyLookupTransform(lookup, im);
}

bool powerLawImagePixels(Image* im, double gamma) {
	if(!im) {
		printf("Cannot power law transform NULL image\n");
		return false;
	}
	
	double c = pow(IMAGE_SCALE - 1, 1 - gamma);
	
	im_t lookup[IMAGE_SCALE];
	size_t i, j;
	for(i = 0; i < IMAGE_SCALE; i++) {
		lookup[i] = (im_t)(pow(i, gamma) * c);
	}

	return applyLookupTransform(lookup, im);
}

bool computeImageChannelHistogram(double histogram[IMAGE_SCALE], size_t channel, Image* im) {
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
	for(i = 0; i < IMAGE_SCALE; i++) {
		histogram[i] = 0;
	}
	for(i = 0; i < imSize; i++) {
		histogram[im->channels[channel]->mat[i]]++;
	}
	for(i = 0; i < IMAGE_SCALE; i++) {
		histogram[i] /= imSize;
	}
	
	return true;
}

bool computeAndWriteImageChannelHistogram(const char* path, size_t channel, Image* im) {
	if(!im) {
		printf("Cannot compute and write histogram of NULL image\n");
		return false;
	}
	double histogram[IMAGE_SCALE];
	computeImageChannelHistogram(histogram, channel, im);
	
	FILE *histoFile;
    histoFile = fopen(path,"wb");
    if (histoFile == NULL) {
		printf("Could not open file to write histogram: %s\n", path);
        return false;
	}

	size_t i;
	for(i = 0; i < IMAGE_SCALE - 1; i++) {
		fprintf(histoFile, "%f, ", histogram[i]);
	}
	fprintf(histoFile, "%f", histogram[i]);
	
	fclose(histoFile);
	
	return true;	
}

Image* generateHistogramPlot(double histogram[IMAGE_SCALE]) {
	
	double maxHist = 0;
	size_t ind;
	for(ind = 0; ind < IMAGE_SCALE; ind++) {
		if(histogram[ind] > maxHist) {
			maxHist = histogram[ind];
		}
	}
	
	size_t graphWidth = IMAGE_SCALE;
	size_t graphHeight = (size_t)(IMAGE_SCALE * maxHist) + 1;
	size_t baseHeight = 0;
	size_t gapHeight = 0;
	size_t gradientHeight = 16;
	size_t rows = graphHeight + gapHeight + gradientHeight + baseHeight;
	size_t cols = graphWidth;
	
	Image* im = allocateImageWithModel(rows, cols, CHANNEL_MODEL_GRAY);	
	
	size_t i,j;
	for(j = 0; j < graphWidth; j++) {
		size_t barHeight = (size_t)(graphHeight - IMAGE_SCALE * histogram[j]);
		for(i = 0; i < barHeight; i++) {
			//setImagePixel(255 - j, i, j, im);
			setImageChannelValue(IMAGE_SCALE - 1, i, j, CHANNEL_GRAY, im);
		}
		for(; i < graphHeight; i++) {
			//setImagePixel(j, i, j, im);
			setImageChannelValue(0, i, j, CHANNEL_GRAY, im);
		}
		for(i = 0; i < baseHeight; i++) {
			setImageChannelValue(IMAGE_SCALE / 2, i + graphHeight, j, CHANNEL_GRAY, im);
		}
		for(i = 0; i < gapHeight; i++) {
			setImageChannelValue(IMAGE_SCALE - 1, i + graphHeight + baseHeight, j, CHANNEL_GRAY, im);
		}
		for(i = 0; i < gradientHeight; i++) {
			setImageChannelValue(j, i + graphHeight + baseHeight + gapHeight, j, CHANNEL_GRAY, im);
		}
	}
	
	return im;
	
}

bool histogramEqualizeImageChannel(size_t channel, Image* source) {
	if(!source) {
		printf("Cannot equalize NULL image");
		return false;
	}
	if(channel >= source->numChannels) {
		printf("Channel index out of range in histogram equalizing image channel\n");
		return false;
	}
	
	double histogram[IMAGE_SCALE];
	if(!computeImageChannelHistogram(histogram, channel, source)) {
		return false;
	}
	
	size_t i, j;
	
	double cdf[IMAGE_SCALE];
	cdf[0] = histogram[0];
	for(i = 1; i < IMAGE_SCALE; i++) {
		cdf[i] = cdf[i - 1] + histogram[i];
	}
	
	im_t scaledCDF[IMAGE_SCALE];
	for(i = 0; i < IMAGE_SCALE; i++) {
		scaledCDF[i] = (IMAGE_SCALE - 1) * cdf[i];
	}
	
	applyChannelLookupTransform(scaledCDF, channel, source);
	
	//apply image scaling
	scaleRangeImageChannel(0, 255, channel, source);
	
	return true;
	
}

bool applyImageChannelKernel(Matrix_d* kernel, size_t channel, Image* im) {
	
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
	if(kernel->rows % 2 != 1 || kernel->cols % 2 != 1) {
		printf("Can only apply odd-dimensioned (symmetric) kernels\n");
		return false;
	}
	
	Image* tempIm = allocateImageWithModel(im->rows, im->rows, im->channelModel);
	
	size_t i, j, k, l;
	size_t kernelDRow = kernel->rows / 2, kernelDCol = kernel->cols / 2;
	
	double val;
	for(i = 0; i < im->rows; i++) {
		for(j = 0; j < im->cols; j++) {
			val = 0;
			for(k = 0; k < kernel->rows; k++) {
				for(l = 0; l < kernel->cols; l++) {
					if(i + k >= kernelDRow && i + k < im->rows + kernelDRow
						&& j + l >= kernelDCol && j + l < im->cols + kernelDCol) {
							im_t pixVal = getImageChannelValue(i + k - kernelDRow, j + l - kernelDCol, channel, im);
							val += (getMatrixEntry_d(k, l, kernel)* pixVal);
					}
				}
			}
			setImageChannelValue((im_t)val, i, j, channel, tempIm);
		}
	}
	
	copyImageData(im, tempIm);
	
	scaleRangeImageChannel(0, IMAGE_SCALE - 1, channel, im);
	
	freeImage(tempIm);
	return true;
	
}

bool applyImageChannelKernelFromFile(const char* kernelPath, size_t channel, Image* im) {
	
	if(!im) {
		printf("Cannot apply kernel transform from file to NULL image\n");
		return false;
	}
	if(channel >= im->numChannels) {
		printf("Channel index out of range in applying image kernel\n");
		return false;
	}
	Matrix_d* kernel = readMatrix_d(kernelPath);
	if(!kernel) {
		printf("Could not open file to read kernel\n");
		return false;
	}
	
	bool success = applyImageChannelKernel(kernel, channel, im);
	freeMatrix_d(kernel);
	
	if(!success) {
		printf("Did not successfully apply Image kernel from file\n");
		return false;
	}
	return true;
}

int pixelComp(const void *a,const void *b) {
	im_t *x = (im_t *) a;
	im_t *y = (im_t *) b;
	if (*x < *y) {
		return -1;
	}
	else if (*x > *y) {
		return 1;
	}
	return 0;
}

bool applyImageChannelMedianFilter(size_t filterRows, size_t filterCols, size_t channel, Image* im) {
	
	if(!im) {
		printf("Cannot apply kernel transform to NULL image\n");
		return false;
	}
	if(channel >= im->numChannels) {
		printf("Channel index out of range in applying median filter\n");
		return false;
	}
	if(filterRows % 2 != 1 || filterCols % 2 != 1) {
		printf("Can only apply odd-dimensioned (symmetric) median filter\n");
		return false;
	}
	
	Image* tempIm = allocateImageWithModel(im->rows, im->rows, im->channelModel);
	size_t halfFilterRows = filterRows / 2, halfFilterCols = filterCols / 2;
	
	size_t i, j;
	
	for(i = 0; i < im->rows; i++) {
		for(j = 0; j < im->cols; j++) {
			//obtain the neighborhood of the pixel padding with zero if necessary
			Matrix_i16* nbhd = createSubmatrixPadZero_i16(filterRows, filterCols,
				i - halfFilterRows, j - halfFilterCols, im->channels[channel]);
			im_t* data = nbhd->mat;
			//sort the pixels of the neighborhood in place
			qsort(data, filterRows * filterCols, sizeof(im_t), pixelComp);
			//the median is the middle element of the sorted neighborhood
			//Note there are less computationally complex methods for finding the median
			im_t medianVal = (im_t)data[filterRows * filterCols / 2 + 1];
			freeMatrix_i16(nbhd);
			setImageChannelValue(medianVal, i, j, channel, tempIm);
		}
	}
	
	copyImageData(im, tempIm);
	freeImage(tempIm);
	return true;
}

bool applyImageKernelIndependent(Matrix_d* kernel, size_t channel, Image* im) {
	if(!im) {
		printf("Cannot apply kernel transform to NULL image\n");
		return false;
	}
	if(!kernel) {
		printf("Cannot apply NULL kernel transform to image\n");
		return false;
	}
	size_t i;
	bool success = true;
	for(i = 0; i < im->numChannels; i++) {
		success = success && applyImageChannelKernel(kernel, i, im);
	}
	return success;
}

bool applyImageKernelFromFileIndependent(const char* kernelPath, size_t channel, Image* im) {
	if(!im) {
		printf("Cannot apply kernel transform to NULL image\n");
		return false;
	}
	size_t i;
	bool success = true;
	for(i = 0; i < im->numChannels; i++) {
		success = success && applyImageChannelKernelFromFile(kernelPath, i, im);
	}
	return success;
}


Image* computeGradientImage(Image* im) {
	
	Image* gradIm = allocateImageWithModel(im->rows, im->cols, im->channelModel);
	
	size_t i, j, channel;
	double val;
	for(channel = 0; channel < im->numChannels; channel++) {
		for(i = 1; i < im->rows - 1; i++) {
			for(j = 1; j < im->cols - 1; j++) {
				val = 0.25 * sqrt(
					(getImageChannelValue(i + 1,j, channel, im) - getImageChannelValue(i - 1,j, channel, im)) * (getImageChannelValue(i + 1,j, channel, im) - getImageChannelValue(i - 1,j, channel, im))
					+ (getImageChannelValue(i,j + 1, channel, im) - getImageChannelValue(i,j - 1, channel, im)) * (getImageChannelValue(i,j + 1, channel, im) - getImageChannelValue(i,j - 1, channel, im)));
				setImageChannelValue((im_t)val, i, j, channel, gradIm);
			}
		}
	}
	return gradIm;
}

Image* computeGradientImageApprox(Image* im) {
	Image* gradIm = allocateImageWithModel(im->rows, im->cols, im->channelModel);
	
	size_t i, j, channel;
	double val;
	for(channel = 0; channel < im->numChannels; channel++) {
		for(i = 1; i < im->rows - 1; i++) {
			for(j = 1; j < im->cols - 1; j++) {
				val = 0.5 * (fabs(getImageChannelValue(i + 1,j, channel, im) - getImageChannelValue(i - 1,j, channel, im))
					+ fabs(getImageChannelValue(i,j + 1, channel, im) - getImageChannelValue(i,j - 1,channel, im)));
				setImageChannelValue((im_t)val, i, j, channel, gradIm);
			}
		}
	}
	return gradIm;
}