#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "Matrix_d.h"
#include "Image.h"
#include "ImageIO.h"
#include "ImageProcessing.h"
#include <math.h>

int main()  {

	const char* ker6Path = "kernels/ker6.dat";
	const char* ker7Path = "kernels/ker7.dat";
	char pathBuffer[256];
	size_t index;
	
	for(index = 0; index < 1; index++) {
		
		sprintf(pathBuffer, "creepy_source/%u.bmp", index);
		Image* frame = readBMPImage(pathBuffer);
		if(!frame) {
			printf("Could not load frame %u\n", index);
			return 1;
		}
		
		applyImageKernelFromFile(frame, ker6Path);
		sprintf(pathBuffer, "creepy_mod/%u.bmp", index);
		writeBMPImage(pathBuffer, frame);
		
		freeImage(frame);

	}
	return 0;
}
