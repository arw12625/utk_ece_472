

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

int setImageChannelBorder(im_t val, size_t borderRows, size_t borderCols, size_t channel, Image* im) {
	
	if(!im) {
		printf("Cannot set border of NULL image\n");
		return -1;
	}
	
	size_t i;
	
	for(i = 0; i < im->rows; i++) {
		setImageValue(0, i, 0, channel, im);
		setImageValue(0, i, im->cols - 1, channel, im);
	}
	for(i = 0; i < im->cols; i++) {
		setImageValue(0, 0, i, channel, im);
		setImageValue(0, im->rows - 1, i, channel, im);
	}
	
	return 0;
}

int setImageBorder(im_t val, size_t borderRows, size_t borderCols, Image* im) {
	
	if(!im) {
		printf("Cannot set border of NULL image\n");
		return -1;
	}
	
	size_t i, channel;
	
	for(i = 0; i < im->rows; i++) {
		for(channel = 0; channel < im->numChannels; channel++) {
			setImageValue(0, i, 0, channel, im);
			setImageValue(0, i, im->cols - 1, channel, im);
		}
	}
	for(i = 0; i < im->cols; i++) {
		for(channel = 0; channel < im->numChannels; channel++) {		
			setImageValue(0, 0, i, channel, im);
			setImageValue(0, im->rows - 1, i, channel, im);
		}
	}
	
	return 0;
}

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
	size_t i;
	size_t numEntries = im->rows * im->cols * im->numChannels;
	for(i = 0; i < numEntries; i++) {
		im->data[i] = (im_t)(im->data[i] * scaler);
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
	size_t i;
	size_t numEntries = im->rows * im->cols * im->numChannels;
	for(i = 0; i < numEntries; i+= im->numChannels) {
		im->data[i + channel] = (im_t)(im->data[i + channel] * scaler);
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
	
	Image* tempIm = allocateImage(im->rows, im->cols, 1);
	
	size_t i, j, k, l;
	size_t kernelDRow = kernelRows / 2, kernelDCol = kernelCols / 2;
	
	setImageBorder(0, kernelDRow, kernelDCol, tempIm);
	
	float val;
	for(i = kernelDRow; i < im->rows - kernelDRow; i++) {
		for(j = kernelDCol; j < im->cols - kernelDCol; j++) {
			val = 0;
			for(k = 0; k < kernelRows; k++) {
				for(l = 0; l < kernelCols; l++) {
					im_t pixVal = getImageValue(i + k - kernelDRow, j + l - kernelDCol, channel, im);
					val += kernel[k][l] * pixVal;
				}
			}
			setImageValue((im_t)val, i, j, 0, tempIm);
		}
	}
	
	/*
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
	}*/
	
	copyImageChannelData(channel, im, 0, tempIm);
	
	//scaleRangeImageChannel(0, IMAGE_SCALE - 1, channel, im);
	
	freeImage(tempIm);
	
	return 0;
	
}

int applyImageKernel(size_t kernelRows, size_t kernelCols,
		float kernel[kernelRows][kernelCols], Image* im) {
	
	if(!im) {
		printf("Cannot apply kernel transform to NULL image\n");
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
	
	Image* tempIm = allocateImage(im->rows, im->cols, im->numChannels);
	tempIm->channelModel = im->channelModel;
	
	size_t i, j, k, l, channel;
	size_t kernelDRow = kernelRows / 2, kernelDCol = kernelCols / 2;
	
	setImageBorder(0, kernelDRow, kernelDCol, tempIm);
	
	float val;
	for(i = kernelDRow; i < im->rows - kernelDRow; i++) {
		for(j = kernelDCol; j < im->cols - kernelDCol; j++) {
			for(channel = 0; channel < im->numChannels; channel++) {
				val = 0;
				for(k = 0; k < kernelRows; k++) {
					for(l = 0; l < kernelCols; l++) {
						im_t pixVal = getImageValue(i + k - kernelDRow, j + l - kernelDCol, channel, im);
						val += kernel[k][l] * pixVal;
					}
				}
				setImageValue((im_t)val, i, j, channel, tempIm);
			}
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
	
	setImageBorder(0, 1, 1, gradIm);

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
	
	setImageBorder(0, 1, 1, gradIm);

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


int truncateImageRange(im_t minVal, im_t maxVal, Image* im) {
	
	if(!im) {
		printf("Cannot truncate NULL image\n");
		return -1;
	}
	if(minVal > maxVal) {
		printf("Cannot truncate image. Minimum value greater than maximum value\n");
	}
	
	size_t i;
	for(i = 0; i < im->numChannels * im->rows * im->cols; i++) {
		im_t *val = &im->data[i];
		if(*val > maxVal) {
			*val = maxVal;
		}
		if(*val < minVal) {
			*val = minVal;
		}
	}
	return 0;
	
}

float PI = 3.14159;

int convertScalerRGB_HSI(float* destHSI, float* sourceRGB) {
	float r,g,b;
	float h, s, i, w, t;
	
	r = sourceRGB[CHANNEL_RED];
	g = sourceRGB[CHANNEL_GREEN];
	b = sourceRGB[CHANNEL_BLUE];
	
	t = r + g + b;
	i = t / 3.0;
	
	if(r == g && g == b) {
		s = 0;
		h = 0;
	} else {
		w = 0.5 * (r - g + r - b) / sqrt((r - g) * (r - g) + (r - b) * (g - b));
		if(w > 1) { w = 1;}
		if(w < -1) { w = -1;}
		h = acos(w);
		if(b > g) { h = 2 * PI - h;}
		if(r <= g && r <= b) { s = 1 - r / i;}
		if(g <= r && g <= b) { s = 1 - g / i;}
		if(b <= r && b <= g) { s = 1 - b / i;}
	}
	
	destHSI[CHANNEL_HUE] = h / 2 / PI;
	destHSI[CHANNEL_SATURATION] = s;
	destHSI[CHANNEL_INTENSITY] = i;
	
	return 0;
}

int convertScalerHSI_RGB(float* destRGB, float* sourceHSI) {
	
	float h,s,i;
	float r, g, b;
	
	h = sourceHSI[CHANNEL_HUE] * 2 * PI;
	s = sourceHSI[CHANNEL_SATURATION];
	i = sourceHSI[CHANNEL_INTENSITY];
	
	if(s > 1) {s = 1;}
	if(i > 1) {i = 1;}
	if(s == 0) {
		r = g = b = i;
	} else {
		if (h >= 0 && h < 2 * PI / 3) {
			b = (1 - s) / 3;
			r = (1 + s * cos(h) / cos(PI / 3 - h)) / 3;
			g = 1 - r - b;
		} else if(h >= 2 * PI / 3 && h < 4 * PI / 3) {
			h = h - 2 * PI / 3;
		r = (1 - s) / 3;
		g = (1 + s * cos(h) / cos(PI / 3 - h)) / 3;
			b = 1 - r - g;
		} else if(h >= 4 * PI / 3 && h < 2 * PI) {
			h = h - 4 * PI / 3;
			g = (1 - s) / 3;
			b = (1 + s * cos(h) / cos(PI / 3 - h)) / 3;
			r = 1 - b - g;
		}
		
		if(r < 0) {r = 0;}
		if(g < 0) {g = 0;}
		if(b < 0) {b = 0;}
		r *= i * 3; g *= i * 3; b *= i * 3;
		if(r > 1) {r = 1;}
		if(g > 1) {g = 1;}
		if(b > 1) {b = 1;}
		
	}
		
	destRGB[CHANNEL_RED] = r;
	destRGB[CHANNEL_GREEN] = g;
	destRGB[CHANNEL_BLUE] = b;
}

int convertPixelRGB_HSI(im_t* destHSI, im_t* sourceRGB) {
	
	float r,g,b;
	float h, s, i, w, t;
	
	r = ((float)sourceRGB[CHANNEL_RED]) / (IMAGE_SCALE - 1);
	g = ((float)sourceRGB[CHANNEL_GREEN]) / (IMAGE_SCALE - 1);
	b = ((float)sourceRGB[CHANNEL_BLUE]) / (IMAGE_SCALE - 1);
	
	t = r + g + b;
	i = t / 3.0;
	
	if(r == g && g == b) {
		s = 0;
		h = 0;
	} else {
		w = 0.5 * (r - g + r - b) / sqrt((r - g) * (r - g) + (r - b) * (g - b));
		if(w > 1) { w = 1;}
		if(w < -1) { w = -1;}
		h = acos(w);
		if(b > g) { h = 2 * PI - h;}
		if(r <= g && r <= b) { s = 1 - r / i;}
		if(g <= r && g <= b) { s = 1 - g / i;}
		if(b <= r && b <= g) { s = 1 - b / i;}
	}
	
	destHSI[CHANNEL_HUE] = (im_t)(h / 2 / PI * (IMAGE_SCALE - 1));
	destHSI[CHANNEL_SATURATION] = (im_t)(s * (IMAGE_SCALE - 1));
	destHSI[CHANNEL_INTENSITY] = (im_t)(i * (IMAGE_SCALE - 1));
	
	return 0;
}

int convertPixelHSI_RGB(im_t* destRGB, im_t* sourceHSI) {
	
	float h,s,i;
	float r, g, b;
	
	h = ((float)sourceHSI[CHANNEL_HUE]) / (IMAGE_SCALE - 1) * 2 * PI;
	s = ((float)sourceHSI[CHANNEL_SATURATION]) / (IMAGE_SCALE - 1);
	i = ((float)sourceHSI[CHANNEL_INTENSITY]) / (IMAGE_SCALE - 1);
	
	if(s > 1) {s = 1;}
	if(i > 1) {i = 1;}
	if(s == 0) {
		r = g = b = i;
	} else {
		if (h >= 0 && h < 2 * PI / 3) {
			b = (1 - s) / 3;
			r = (1 + s * cos(h) / cos(PI / 3 - h)) / 3;
			g = 1 - r - b;
		} else if(h >= 2 * PI / 3 && h < 4 * PI / 3) {
			h = h - 2 * PI / 3;
		r = (1 - s) / 3;
		g = (1 + s * cos(h) / cos(PI / 3 - h)) / 3;
			b = 1 - r - g;
		} else if(h >= 4 * PI / 3 && h < 2 * PI) {
			h = h - 4 * PI / 3;
			g = (1 - s) / 3;
			b = (1 + s * cos(h) / cos(PI / 3 - h)) / 3;
			r = 1 - b - g;
		}
		
		if(r < 0) {r = 0;}
		if(g < 0) {g = 0;}
		if(b < 0) {b = 0;}
		r *= i * 3; g *= i * 3; b *= i * 3;
		if(r > 1) {r = 1;}
		if(g > 1) {g = 1;}
		if(b > 1) {b = 1;}
		
	}
		
	destRGB[CHANNEL_RED] = (im_t)(r * (IMAGE_SCALE - 1));
	destRGB[CHANNEL_GREEN] = (im_t)(g * (IMAGE_SCALE - 1));
	destRGB[CHANNEL_BLUE] = (im_t)(b * (IMAGE_SCALE - 1));
}


Image* convertImageRGB_HSI(Image* im) {
	if(!im) {
		printf("Cannot convert NULL image\n");
		return NULL;
	}
	if(im->channelModel != CHANNEL_MODEL_RGB) {
		printf("Cannot convert image from RGB to HSI. Image channel model is not RGB\n");
		return NULL;
	}
	
	Image* hsiImage = allocateImageWithModel(im->rows, im->cols, CHANNEL_MODEL_HSI);
	
	size_t index;
	float r,g,b;
	float h, s, i, w, t;
	for(index = 0; index < im->rows * im->cols * 3; index += im->numChannels) {
		
		r = ((float)im->data[index + CHANNEL_RED]) / (IMAGE_SCALE - 1);
		g = ((float)im->data[index + CHANNEL_GREEN]) / (IMAGE_SCALE - 1);
		b = ((float)im->data[index + CHANNEL_BLUE]) / (IMAGE_SCALE - 1);
		
		t = r + g + b;
		i = t / 3.0;
		
		if(r == g && g == b) {
			s = 0;
			h = 0;
		} else {
			w = 0.5 * (r - g + r - b) / sqrt((r - g) * (r - g) + (r - b) * (g - b));
			if(w > 1) { w = 1;}
			if(w < -1) { w = -1;}
			h = acos(w);
			if(b > g) { h = 2 * PI - h;}
			if(r <= g && r <= b) { s = 1 - r / i;}
			if(g <= r && g <= b) { s = 1 - g / i;}
			if(b <= r && b <= g) { s = 1 - b / i;}
		}
		
		hsiImage->data[index + CHANNEL_HUE] = (im_t)(h / 2 / PI * (IMAGE_SCALE - 1));
		hsiImage->data[index + CHANNEL_SATURATION] = (im_t)(s * (IMAGE_SCALE - 1));
		hsiImage->data[index + CHANNEL_INTENSITY] = (im_t)(i * (IMAGE_SCALE - 1));
	}
	return hsiImage;
}


Image* convertImageHSI_RGB(Image* im) {
	if(!im) {
		printf("Cannot convert NULL image\n");
		return NULL;
	}
	if(im->channelModel != CHANNEL_MODEL_HSI) {
		printf("Cannot convert image from HSI to RGB. Image channel model is not HSI\n");
		return NULL;
	}
	
	Image* rgbImage = allocateImageWithModel(im->rows, im->cols, CHANNEL_MODEL_RGB);
	
	size_t index;
	float h,s,i;
	float r, g, b;
	for(index = 0; index < im->rows * im->cols * 3; index += im->numChannels) {
		h = ((float)im->data[index + CHANNEL_HUE]) / (IMAGE_SCALE - 1) * 2 * PI;
		s = ((float)im->data[index + CHANNEL_SATURATION]) / (IMAGE_SCALE - 1);
		i = ((float)im->data[index + CHANNEL_INTENSITY]) / (IMAGE_SCALE - 1);
		
		if(s > 1) {s = 1;}
		if(i > 1) {i = 1;}
		if(s == 0) {
			r = g = b = i;
		} else {
			if (h >= 0 && h < 2 * PI / 3) {
				b = (1 - s) / 3;
				r = (1 + s * cos(h) / cos(PI / 3 - h)) / 3;
				g = 1 - r - b;
			} else if(h >= 2 * PI / 3 && h < 4 * PI / 3) {
				h = h - 2 * PI / 3;
				r = (1 - s) / 3;
				g = (1 + s * cos(h) / cos(PI / 3 - h)) / 3;
				b = 1 - r - g;
			} else if(h >= 4 * PI / 3 && h < 2 * PI) {
				h = h - 4 * PI / 3;
				g = (1 - s) / 3;
				b = (1 + s * cos(h) / cos(PI / 3 - h)) / 3;
				r = 1 - b - g;
			}
			
			if(r < 0) {r = 0;}
			if(g < 0) {g = 0;}
			if(b < 0) {b = 0;}
			r *= i * 3; g *= i * 3; b *= i * 3;
			if(r > 1) {r = 1;}
			if(g > 1) {g = 1;}
			if(b > 1) {b = 1;}
			
		}
		
		rgbImage->data[index + CHANNEL_RED] = (im_t)(r * (IMAGE_SCALE - 1));
		rgbImage->data[index + CHANNEL_GREEN] = (im_t)(g * (IMAGE_SCALE - 1));
		rgbImage->data[index + CHANNEL_BLUE] = (im_t)(b * (IMAGE_SCALE - 1));
		
	}
	return rgbImage;
}