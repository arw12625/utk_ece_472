#include <stdio.h>
#include <stdlib.h>

#include "ffmpeg_util.h"
#include "image.h"
#include "image_processing.h"

#define GLFW_DLL
#include "GLFW/glfw3.h"

void writePPM(AVFrame *pFrame, int width, int height, int frameNumber) {

  FILE *pFile;
  char szFilename[32];
  int  y;
  
  // Open file
  sprintf(szFilename, "images/frame%d.ppm", frameNumber);
  pFile=fopen(szFilename, "wb");
  if(pFile==NULL)
    return;
  
  // Write header
  fprintf(pFile, "P6\n%d %d\n255\n", width, height);
  
  // Write pixel data
  for(y=0; y<height; y++)
    fwrite(pFrame->data[0]+y*pFrame->linesize[0], 1, width*3, pFile);
  
  // Close file
  fclose(pFile);
}

bool processFrame(AVFrame *frame, int frameNumber, void* arg) {
	
	VideoWriteStruct* vws = (VideoWriteStruct*)arg;
	//printf("Frame #%u\n", frameNumber);
	
	int width = frame->width;
	int height = frame->height;
	
	//if(frameNumber < 300 && frameNumber > 9) {
		
		Image* frameImage = createImageFromBytes(height, width, 3, frame->data[0]);
		//printSubImage(5, 5, 40, 40, frameImage);
		//exponentiateImage(frameImage);
		Image* gradIm = computeGradientImage(frameImage);
		getBytesFromImage(frame->data[0], gradIm);
		scaleRangeImageUniform(0, 255, gradIm);
		freeImage(gradIm);
		freeImage(frameImage);
	//}
	
	writeVideoFrame(vws, frame, width, height, frameNumber);
	
	return true;
}

int main( int argc, char *argv[] )  {

	//parse command line arguments
	const char* inFileName;
	const char* outFileName;
	if(argc == 3) {
		inFileName = argv[1];
		outFileName = argv[2];
	} else if(argc == 1) {
		inFileName = "../../big_buck_bunny.mp4";
		outFileName = "../../result/yolo.mp4";
	} else {
		printf("Incorrect number of arguments.\n");
		return 1;
	}
	
	
	VideoWriteStruct* vws = NULL;
	VideoReadStruct* vrs = NULL;
	openVideoRead(&vrs, inFileName);
	
	
	openVideoWrite(&vws, outFileName, 
			vrs->codecContext->bit_rate, vrs->width, vrs->height, 24);
	
	processVideo(vrs, &processFrame, vws);
	closeVideoRead(&vrs);
	
	closeVideoWrite(&vws);
	
	
	return 0;
}
