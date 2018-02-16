#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "Image.h"
#include "ImageIO.h"
#include "ImageProcessing.h"
#include <math.h>

int main( int argc, char *argv[] )  {

	//parse command line arguments
	char* basePath;
	if( argc == 2 ) {
		basePath = argv[1];
	} else {
		basePath = "images/source/woman.bmp";
	}
	
	//load base image
	Image* baseImage = readBMPImage(basePath);
	if(!baseImage) {
		return 1;
	}
	
	//compute and write base image histogram to file
	computeAndWriteImageHistogram(baseImage, "images/result/histogram/base_hist.dat");
	
	//PART 1 - divide intensity by 2
	//duplicate base image for scaling
	Image* scaleIm = duplicateImage(baseImage);
	//scale intensity by 0.5
	scaleImage(scaleIm, 0.5);
	//write image to a BMP file
	writeBMPImage("images/result/scale.bmp", scaleIm);
	//compute and write image histogram to file
	computeAndWriteImageHistogram(scaleIm, "images/result/histogram/scale_hist.dat");
	
	//PART 2 - image scaling
	//scale the image to the range 0 - 255
	//the image that had its intensity scaled by 0.5 is used instead of the original image, as it already occupies the range 0-255
	scaleRangeImage(scaleIm, 0, IMAGE_SCALE - 1);
	//write image to a BMP file
	writeBMPImage("images/result/scale_range.bmp", scaleIm);
	//compute and write image histogram to file
	computeAndWriteImageHistogram(scaleIm, "images/result/histogram/scale_range_hist.dat");
	//free the image
	freeImage(scaleIm);
	
	//PART 3 - inversion
	//duplicate base image for inversion
	Image* invIm = duplicateImage(baseImage);
	//invert image
	invertImagePixels(invIm);
	//write image to a BMP file
	writeBMPImage("images/result/inv.bmp", invIm);
	//compute and write image histogram to file
	computeAndWriteImageHistogram(invIm, "images/result/histogram/inv_hist.dat");
	//free the image
	freeImage(invIm);
	
	//PART 4 - exponentiation
	//duplicate the base image for exponentiation
	Image* expIm = duplicateImage(baseImage);
	//exponentiate image
	exponentiateImagePixels(expIm);
	//write image to a BMP file
	writeBMPImage("images/result/exp.bmp", expIm);
	//compute and write image histogram to file
	computeAndWriteImageHistogram(expIm, "images/result/histogram/exp_hist.dat");
	//perform histogram equalization
	histogramEqualizeImage(expIm);
	//write equalized image to a BMP file
	writeBMPImage("images/result/expEqualized.bmp", expIm);
	//compute and write image histogram to file
	computeAndWriteImageHistogram(expIm, "images/result/histogram/exp_equ_hist.dat");
	//free the image
	freeImage(expIm);
	
	//PART 5 - logarithm
	//duplicate the base image to apply the logarithm
	Image* logIm = duplicateImage(baseImage);
	//perform the logarithm transform on the image
	logarithmImagePixels(logIm);
	//write image to a file
	writeBMPImage("images/result/log.bmp", logIm);
	//compute and write image histogram to file
	computeAndWriteImageHistogram(logIm, "images/result/histogram/log_hist.dat");
	//perform histogram equalization
	histogramEqualizeImage(logIm);
	//write the equalized image to a BMP file
	writeBMPImage("images/result/logEqualized.bmp", logIm);
	//compute and write image histogram to file
	computeAndWriteImageHistogram(logIm, "images/result/histogram/log_equ_hist.dat");
	//free the image
	freeImage(logIm);
	
	//PART 6 - gamma power law
	//perform the power law transform over a range of gamma values
	size_t numGamma = 7;
	double gamma_val[] = {0.125, 0.25, 0.5, 1, 2, 4, 8};
	size_t i;
	Image* powerIm;
	for(i = 0; i < numGamma; i++) {
		//duplicate the base image for the power law
		powerIm = duplicateImage(baseImage);
		double gamma = gamma_val[i];
		//apply the power law transform with the given gamma value
		powerLawImagePixels(powerIm, gamma);
		//format the path to write the image to
		char path[64];
		sprintf(path, "images/result/gamma/gamma%.2f.bmp", gamma);
		//write the image to a file
		writeBMPImage(path, powerIm);
		//format the path to write the histogram to
		
		sprintf(path, "images/result/histogram/gamma/gamma%.2f_hist.dat", gamma);
		//compute and write image histogram to file
		computeAndWriteImageHistogram(powerIm, path);
		
		//perform histogram equalization
		histogramEqualizeImage(powerIm);
		//format the path to write the equalized image to
		sprintf(path, "images/result/gamma/gamma%.2f_Equalized.bmp", gamma);
		//write the equalized image to a file
		writeBMPImage(path, powerIm);
		//format the path to write the histogram to
		
		sprintf(path, "images/result/histogram/gamma/gamma%.2f_equ_hist.dat", gamma);
		//compute and write image histogram to file
		computeAndWriteImageHistogram(powerIm, path);
		//free the image
		freeImage(powerIm);
	}
	
	//free the base image
	freeImage(baseImage);
	
	return 0;
}
