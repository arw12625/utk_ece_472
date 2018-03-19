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
	Image* baseImage = readBMPImageChannelModel(basePath, CHANNEL_MODEL_GRAY);
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
		applyImageChannelKernelFromFile(pathBuffer, CHANNEL_GRAY, kerIm);
		sprintf(pathBuffer, "images/result/ker%u.bmp", kerIndex);
		writeBMPImageChannel8Bit(pathBuffer, CHANNEL_GRAY, kerIm);
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
		applyImageChannelKernel(boostKerMat, CHANNEL_GRAY, boostImage);
		sprintf(pathBuffer, "images/result/boost_ker_%f.bmp", boostValues[boostIndex]);
		writeBMPImageChannel8Bit(pathBuffer, CHANNEL_GRAY, boostImage);
	}
	freeImage(boostImage);
	freeMatrix_d(boostKerMat);
	
	
	Image* medianIm = duplicateImage(baseImage);
	applyImageChannelMedianFilter(3, 3, CHANNEL_GRAY, medianIm);
	writeBMPImageChannel8Bit("images/result/median.bmp", CHANNEL_GRAY, medianIm);
	freeImage(medianIm);
	
	//free the base image
	freeImage(baseImage);
	
	return 0;
}
