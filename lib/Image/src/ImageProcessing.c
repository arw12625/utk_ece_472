#include "ImageProcessing.h"
#include <math.h>
#include <stdio.h>

bool sumImages(Image* dest, Image* summand1, Image* summand2) {
	if(!summand1 || !summand2) {
		printf("Cannot sum a NULL image\n");
		return false;
	}
	return sumMatrices_i16(dest->pixels, summand1->pixels, summand2->pixels);
}

/*
Image* createImageFromSum(Image* summand1, Image* summand2) {
	
	printf("createImageFromSum is not implemented yet");
	return NULL;
	
	if(!summand1 || !summand2) {
		printf("Cannot sum a NULL image\n");
		return NULL;
	}
	Matrix_u16* resultMatrix = createMatrixFromSum_u16(summand1->pixels, summand2->pixels);
	if(!resultMatrix) {
		return NULL;
	}
	Image* result = allocateEmptyImage();
	result->pixels = resultMatrix;
	return result;
	
}*/

bool scaleImage(Image* im, float scaler) {
	if(!im) {
		printf("Cannot scale NULL image\n");
		return false;
	}
	return scaleMatrix_i16(im->pixels, scaler);
}

//operates on the pixels of the image by the affine interval map from [a1, b1] to [a2, b2]
bool intervalMapImage(Image* im, im_t a1, im_t b1, im_t a2, im_t b2) {
	if(!im) {
		printf("Cannot interval map NULL image\n");
		return false;
	}
	return intervalMapMatrix_i16(im->pixels, a1, b1, a2, b2);
}

//interval maps the pixels of the image from the range of the matrix to the desired range
//eg maps the mininum of m to minVal and the maximum of m to maxVal
bool scaleRangeImage(Image* im, im_t minVal, im_t maxVal) {
	if(!im) {
		printf("Cannot scale range of NULL image\n");
		return false;
	}
	return scaleRangeMatrix_i16(im->pixels, minVal, maxVal);
}

//Image must be in range to apply lookup transform (eg 0 <= val < IMAGE_SCALE)
bool applyLookupTransform(Image* im, im_t lookup[IMAGE_SCALE]) {
	if(!im) {
		printf("Cannot apply lookup transform to NULL image\n");
		return false;
	}
	
	size_t i, j;
	size_t curVal;
	for(i = 0; i < im->pixels->rows; i++) {
		for(j = 0; j < im->pixels->cols; j++) {
			curVal = (size_t)getMatrixEntry_i16(i, j, im->pixels);
			if(curVal < IMAGE_SCALE) {
				curVal = lookup[curVal];
			} else {
				curVal = 0;
			}
			setMatrixEntry_i16((im_t)curVal, i, j, im->pixels);
		}
	}
	return true;
}

bool invertImagePixels(Image* im) {
	if(!im) {
		printf("Cannot invert NULL image\n");
		return false;
	}
	im_t lookup[IMAGE_SCALE];
	size_t i;
	for(i = 0; i < IMAGE_SCALE; i++) {
		lookup[i] = IMAGE_SCALE - 1 - i;
	}
	
	return applyLookupTransform(im, lookup);
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
	
	return applyLookupTransform(im, lookup);
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
	
	return applyLookupTransform(im, lookup);
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

	return applyLookupTransform(im, lookup);
}

bool computeImageHistogram(Image* im, double histogram[IMAGE_SCALE]) {
	if(!im) {
		printf("Cannot compute histogram of NULL image\n");
		return false;
	}
	size_t i;
	size_t imSize = im->pixels->rows * im->pixels->rows;
	for(i = 0; i < IMAGE_SCALE; i++) {
		histogram[i] = 0;
	}
	for(i = 0; i < imSize; i++) {
		histogram[im->pixels->mat[i]]++;
	}
	for(i = 0; i < IMAGE_SCALE; i++) {
		histogram[i] /= imSize;
	}
	
	return true;
}

bool computeAndWriteImageHistogram(Image* im, const char* path) {
	if(!im) {
		printf("Cannot compute and write histogram of NULL image\n");
		return false;
	}
	double histogram[IMAGE_SCALE];
	computeImageHistogram(im, histogram);
	
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
	
	Image* im = allocateImage(rows, cols);
	
	size_t i,j;
	for(j = 0; j < graphWidth; j++) {
		size_t barHeight = (size_t)(graphHeight - IMAGE_SCALE * histogram[j]);
		for(i = 0; i < barHeight; i++) {
			//setImagePixel(255 - j, i, j, im);
			setImagePixel(IMAGE_SCALE - 1, i, j, im);
		}
		for(; i < graphHeight; i++) {
			//setImagePixel(j, i, j, im);
			setImagePixel(0, i, j, im);
		}
		for(i = 0; i < baseHeight; i++) {
			setImagePixel(IMAGE_SCALE / 2, i + graphHeight, j, im);
		}
		for(i = 0; i < gapHeight; i++) {
			setImagePixel(IMAGE_SCALE - 1, i + graphHeight + baseHeight, j, im);
		}
		for(i = 0; i < gradientHeight; i++) {
			setImagePixel(j, i + graphHeight + baseHeight + gapHeight, j, im);
		}
	}
	
	return im;
	
}

bool histogramEqualizeImage(Image* source) {
	
	if(!source) {
		printf("Cannot equalize NULL image");
		return false;
	}
	
	double histogram[IMAGE_SCALE];
	if(!computeImageHistogram(source, histogram)) {
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
	
	applyLookupTransform(source, scaledCDF);
	
	//apply image scaling
	//scaleRangeImage(source, 0, 255);
	
	return true;
	
}

bool applyImageKernel(Image* im, Matrix_d* kernel) {
	
	if(!im) {
		printf("Cannot apply kernel transform to NULL image\n");
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
	
	Image* tempIm = allocateImage(im->pixels->rows, im->pixels->rows);
	
	size_t i, j, k, l;
	size_t kernelDRow = kernel->rows / 2, kernelDCol = kernel->cols / 2;
	
	double val;
	for(i = 0; i < im->pixels->rows; i++) {
		for(j = 0; j < im->pixels->cols; j++) {
			val = 0;
			for(k = 0; k < kernel->rows; k++) {
				for(l = 0; l < kernel->cols; l++) {
					if(i + k >= kernelDRow && i + k < im->pixels->rows + kernelDRow
						&& j + l >= kernelDCol && j + l < im->pixels->cols + kernelDCol) {
							im_t pixVal = getImagePixel(i + k - kernelDRow, j + l - kernelDCol, im);
							val += (getMatrixEntry_d(k, l, kernel)* pixVal);
					}
				}
			}
			setImagePixel((im_t)val, i, j, tempIm);
		}
	}
	
	copyImagePixels(im, tempIm);
	
	scaleRangeImage(im, 0, IMAGE_SCALE - 1);
	
	freeImage(tempIm);
	return true;
	
}

bool applyImageKernelFromFile(Image* im, const char* kernelPath) {
	
	if(!im) {
		printf("Cannot apply kernel transform from file to NULL image\n");
		return false;
	}
	Matrix_d* kernel = readMatrix_d(kernelPath);
	if(!kernel) {
		printf("Could not open file to read kernel\n");
		return false;
	}
	bool success = applyImageKernel(im, kernel);
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

bool applyImageMedianFilter(Image* im, size_t filterRows, size_t filterCols) {
	
	if(!im) {
		printf("Cannot apply kernel transform to NULL image\n");
		return false;
	}
	if(filterRows % 2 != 1 || filterCols % 2 != 1) {
		printf("Can only apply odd-dimensioned (symmetric) median filter\n");
		return false;
	}
	
	Image* tempIm = allocateImage(im->pixels->rows, im->pixels->rows);
	size_t halfFilterRows = filterRows / 2, halfFilterCols = filterCols / 2;
	
	size_t i, j;
	
	for(i = 0; i < im->pixels->rows; i++) {
		for(j = 0; j < im->pixels->cols; j++) {
			//obtain the neighborhood of the pixel padding with zero if necessary
			Matrix_i16* nbhd = createSubmatrixPadZero_i16(filterRows, filterCols,
				i - halfFilterRows, j - halfFilterCols, im->pixels);
			im_t* data = nbhd->mat;
			//sort the pixels of the neighborhood in place
			qsort(data, filterRows * filterCols, sizeof(im_t), pixelComp);
			//the median is the middle element of the sorted neighborhood
			//Note there are less computationally complex methods for finding the median
			im_t medianVal = (im_t)data[filterRows * filterCols / 2 + 1];
			freeMatrix_i16(nbhd);
			setImagePixel(medianVal, i, j, tempIm);
		}
	}
	
	copyImagePixels(im, tempIm);
	freeImage(tempIm);
	return true;
}


Image* computeGradientImage(Image* im) {
	
	Image* gradIm = allocateImage(im->pixels->rows, im->pixels->cols);
	
	size_t i, j;
	double val;
	for(i = 1; i < im->pixels->rows - 1; i++) {
		for(j = 1; j < im->pixels->cols - 1; j++) {
			val = 0.25 * sqrt(
				(getImagePixel(i + 1,j, im) - getImagePixel(i - 1,j, im)) * (getImagePixel(i + 1,j, im) - getImagePixel(i - 1,j, im))
				+ (getImagePixel(i,j + 1, im) - getImagePixel(i,j - 1, im)) * (getImagePixel(i,j + 1, im) - getImagePixel(i,j - 1, im)));
			setImagePixel((im_t)val, i, j, gradIm);
		}
	}
	return gradIm;
}

Image* computeGradientImageApprox(Image* im) {
	Image* gradIm = allocateImage(im->pixels->rows, im->pixels->cols);
	
	size_t i, j;
	double val;
	for(i = 1; i < im->pixels->rows - 1; i++) {
		for(j = 1; j < im->pixels->cols - 1; j++) {
			val = 0.5 * (fabs(getImagePixel(i + 1,j, im) - getImagePixel(i - 1,j, im))
				+ fabs(getImagePixel(i,j + 1, im) - getImagePixel(i,j - 1, im)));
			setImagePixel((im_t)val, i, j, gradIm);
		}
	}
	return gradIm;
}