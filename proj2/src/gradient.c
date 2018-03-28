#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>

#include "Image.h"
#include "ImageIO.h"
#include "ImageProcessing.h"
#include <math.h>

int main()  {
	const char* basePath = "images/source/woman.bmp";
	//load base image
	Image* baseImage = readBMPImage(basePath);
	if(!baseImage) {
		printf("Could not load base image\n");
		return 1;
	}
	
	size_t numIter = 2;
	size_t i;
	
	clock_t startTime, endTime;
	
	startTime = clock();
	for(i = 0; i < numIter; i++) {
		Image* gradientSqrtIm = computeGradientImage(baseImage);
		if(!gradientSqrtIm) {
			printf("EHEHEH");
		}
		freeImage(gradientSqrtIm);
	}
	 endTime = clock();
	
	double sqrtTime = ((double)(endTime - startTime) / 1000000.0d) * 1000;
	
	startTime = clock();
	for(i = 0; i < numIter; i++) {
		Image* gradientAbsIm = computeGradientImageApprox(baseImage);
		freeImage(gradientAbsIm);
	}
	endTime = clock();
	
	double absTime = ((double)(endTime - startTime) / 1000000.0d) * 1000;
	
	printf("The sqrt gradient calculation completed in %f seconds\n", sqrtTime);
	printf("The abs gradient calculation completed in %f seconds\n", absTime);
	
	freeImage(baseImage);
}
