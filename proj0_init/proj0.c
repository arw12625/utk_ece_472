#include <stdio.h>
#include <stdlib.h>

typedef unsigned char uint8;

void initializeImage(uint8 val, int height, int width, uint8 dest[height][width]);
void writeImage(const char *filename, int height, int width,  uint8 image[height][width]);
void readImage(const char *filename, int height, int width, uint8 image[height][width]);
void addImages(int height, int width,  uint8 image1[height][width],  uint8 image2[height][width], uint8 dest[height][width]);
void printImagePortion(int height, int width,  uint8 image[height][width], int py, int px, int pheight, int pwidth);


int main() {
	
	int width = 640, height = 480;
	
	uint8 A[height][width];
	initializeImage(25, height, width, A);
	writeImage("A.img", height, width, A);
	readImage("A.img", height, width, A);
	printf("A:\n");
	printImagePortion(height, width, A, 0, 0, 8, 16);
	
	uint8 B[height][width];
	printf("B:\n");
	initializeImage(100, height, width, B);
	writeImage("B.img", height, width, B);
	readImage("B.img", height, width, B);
	printImagePortion(height, width, B, 0, 0, 8, 16);
	
	
	printf("C:\n");
	uint8 C[height][width];
	addImages(height, width, A, B, C);
	writeImage("C.img", height, width, C);
	readImage("C.img", height, width, C);
	printImagePortion(height, width, C, 0, 0, 8, 16);
	
}

void initializeImage(uint8 val, int height, int width, uint8 dest[height][width]) {
	int i,j;
	for(i = 0; i < height; i++) {
		for(j = 0; j < width; j++) {
			dest[i][j] = val;
		}
	}
}

void writeImage(const char *filename, int height, int width,  uint8 image[height][width]) {
	
	FILE* fp = fopen(filename, "w");
	if(!fp) {
		printf("Could not open file to write image to: %s", filename);
		return;
	}
	
	int numWritten = fwrite(image, sizeof(uint8), width * height * sizeof(uint8), fp);
	if(width * height * sizeof(uint8) != numWritten) {
		printf("Error when writing image to: %s", filename);
	}
}

void readImage(const char *filename, int height, int width, uint8 image[height][width]) {
	
	FILE* fp = fopen(filename, "r");
	if(!fp) {
		printf("Could not open file to read image from: %s", filename);
		return;
	}
	
	int numRead = fread(image, sizeof(uint8), width * height * sizeof(uint8), fp);
	if(width * height * sizeof(uint8) != numRead) {
		printf("Error when read image from: %s", filename);
	}
}

void addImages(int height, int width,  uint8 image1[height][width],  uint8 image2[height][width], uint8 dest[height][width]) {
	int i,j;
	for(i = 0; i < height; i++) {
		for(j = 0; j < width; j++) {
			dest[i][j] = image1[i][j] + image2[i][j];
		}
	}
}

void printImagePortion(int height, int width,  uint8 image[height][width], int py, int px, int pheight, int pwidth) {
	
	int i,j;
	for(i = py; i < py + pheight; i++) {
		for(j = px; j < px + pwidth - 1; j++) {
			printf("%i, ", image[i][j]);
		}
		printf("%i\n", image[i][j]);
	}
}
