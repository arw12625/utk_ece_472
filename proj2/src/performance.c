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
	Image* baseImage = readBMPImageChannelModel(basePath, CHANNEL_MODEL_GRAY);
	if(!baseImage) {
		printf("Could not load base image\n");
		return 1;
	}
	
	size_t numIter = 30;
	size_t i;
	
	clock_t startTime, endTime;
	
	startTime = clock();
	for(i = 0; i < numIter; i++) {
		Image* dup = duplicateImage(baseImage);
		invertImage(dup);
		writeBMPImageChannel8Bit("images/result/test.bmp", CHANNEL_GRAY, dup);
		freeImage(dup);
	}
	 endTime = clock();
	
	double opTime = ((double)(endTime - startTime) / 1000000.0d) * 1000;
	
	printf("The image operation completed in %f seconds\n", opTime);
	
	freeImage(baseImage);
}
