#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "Matrix_d.h"
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
		printf("Could not load base image\n");
		return 1;
	}
	
	char pathBuffer[256];
	size_t kerIndex;
	Image* kerIm;
	for(kerIndex = 1; kerIndex < 8; kerIndex++) {
		kerIm = duplicateImage(baseImage);
		sprintf(pathBuffer, "kernels/ker%u.dat", kerIndex);
		applyImageKernelFromFile(kerIm, pathBuffer);
		sprintf(pathBuffer, "images/result/ker%u.bmp", kerIndex);
		writeBMPImage(pathBuffer, kerIm);
	}
	freeImage(kerIm);
	
	double boostKerEntries[] = {-1, -1, -1, -1, 0, -1, -1, -1, -1};
	Matrix_d* boostKerMat = createMatrixFromEntries_d(3, 3, boostKerEntries);
	double boostValues[] = {0, 1, 2, 4, 8, 16};
	size_t numBoostValues = 6;
	size_t boostIndex;
	Image* boostImage;
	for(boostIndex = 0; boostIndex < numBoostValues; boostIndex++) {
		setMatrixEntry_d(8 + boostValues[boostIndex], 1, 1, boostKerMat);
		boostImage = duplicateImage(baseImage);
		applyImageKernel(boostImage, boostKerMat);
		sprintf(pathBuffer, "images/result/boost_ker_%f.bmp", boostValues[boostIndex]);
		writeBMPImage(pathBuffer, boostImage);
	}
	free(boostImage);
	freeMatrix_d(boostKerMat);
	
	
	Image* medianIm = duplicateImage(baseImage);
	applyImageMedianFilter(medianIm, 3, 3);
	writeBMPImage("images/result/median.bmp", medianIm);
	freeImage(medianIm);
	
	//free the base image
	freeImage(baseImage);
	
	return 0;
}
