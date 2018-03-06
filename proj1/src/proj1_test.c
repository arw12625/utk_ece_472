#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "Image.h"
#include "ImageIO.h"
#include "ImageProcessing.h"
#include <math.h>

int main( int argc, char *argv[] )  {

	/*char* basePath;
	if( argc == 2 ) {
		basePath = argv[1];
	} else {
		basePath = "images/source/woman.bmp";
	}
	Image* baseImage = readBMPImage(basePath);
	
	double histogram[IMAGE_SCALE];
	computeImageHistogram(baseImage, histogram);
	Image* histogramImage = generateHistogramPlot(histogram);
	writeBMPImage("images/histogram/baseHistogram.bmp", histogramImage);
	freeImage(histogramImage);
	Image* equalizedImage = duplicateImage(baseImage);
	histogramEqualizeImage(equalizedImage);
	writeBMPImage("images/result/equalized.bmp", equalizedImage);
	computeImageHistogram(equalizedImage, histogram);
	histogramImage = generateHistogramPlot(histogram);
	writeBMPImage("images/histogram/equalizedHistogram.bmp", histogramImage);
	freeImage(histogramImage);
	freeImage(equalizedImage);
	
	Image* expIm = duplicateImage(baseImage);
	exponentiateImagePixels(expIm);
	writeBMPImage("images/result/exp.bmp", expIm);
	computeImageHistogram(expIm, histogram);
	histogramImage = generateHistogramPlot(histogram);
	writeBMPImage("images/histogram/expHistogram.bmp", histogramImage);
	freeImage(histogramImage);	
	histogramEqualizeImage(expIm);
	writeBMPImage("images/result/expEqualized.bmp", expIm);
	computeImageHistogram(expIm, histogram);
	histogramImage = generateHistogramPlot(histogram);
	writeBMPImage("images/histogram/expEqualizedHistogram.bmp", histogramImage);
	freeImage(histogramImage);	
	freeImage(expIm);
	
	Image* logIm = duplicateImage(baseImage);
	logarithmImagePixels(logIm);
	writeBMPImage("images/result/log.bmp", logIm);
	computeImageHistogram(logIm, histogram);
	histogramImage = generateHistogramPlot(histogram);
	writeBMPImage("images/histogram/logHistogram.bmp", histogramImage);
	freeImage(histogramImage);	
	histogramEqualizeImage(logIm);
	writeBMPImage("images/result/logEqualized.bmp", logIm);
	computeImageHistogram(logIm, histogram);
	histogramImage = generateHistogramPlot(histogram);
	writeBMPImage("images/histogram/logEqualizedHistogram.bmp", histogramImage);
	freeImage(histogramImage);	
	freeImage(logIm);
	
	Image* scaleIm = duplicateImage(baseImage);
	scaleImage(scaleIm, 1.0 / 32);
	writeBMPImage("images/result/scale.bmp", scaleIm);
	computeImageHistogram(scaleIm, histogram);
	histogramImage = generateHistogramPlot(histogram);
	writeBMPImage("images/histogram/scaleHistogram.bmp", histogramImage);
	freeImage(histogramImage);
	scaleRangeImage(scaleIm, 0, IMAGE_SCALE - 1);
	writeBMPImage("images/result/scale_range.bmp", scaleIm);
	computeImageHistogram(scaleIm, histogram);
	histogramImage = generateHistogramPlot(histogram);
	writeBMPImage("images/histogram/fullScaleHistogram.bmp", histogramImage);
	freeImage(histogramImage);
	freeImage(scaleIm);
	
	size_t numGamma = 7;
	double gamma_val[] = {0.125, 0.25, 0.5, 1, 2, 4, 8};
	size_t i;
	for(i = 0; i < numGamma; i++) {
		Image* powerIm = duplicateImage(baseImage);	
		double gamma = gamma_val[i];
		powerLawImagePixels(powerIm, gamma);
		char path[64];
		sprintf(path, "images/gamma/gamma%.2f.bmp", gamma);
		writeBMPImage(path, powerIm);
		freeImage(powerIm);
	}
	
	freeImage(histogramImage);
	freeImage(baseImage);
	*/
}
