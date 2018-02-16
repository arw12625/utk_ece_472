#include "ImageIO.h"

#include <stdio.h>

#pragma pack(push, 1)

//Code modified from <https://stackoverflow.com/a/14279511>

typedef struct BMPFileHeader {
    char bmpType1; //first character of the type of bitmap (eg "BM")
	char bmpType2; //first character of the type of bitmap (eg "BM")
    uint32_t bmpSize; //size in bytes of the file
    uint16_t bmpReserved1; //reserved
    uint16_t bmpReserved2; //reserved
    uint32_t bOffBits; //offset in bytes from the start of the file to the bitmap itself
} BMPFileHeader;

//currently there is only support for loading bitmaps with a BITMAPINFOHEADER
typedef struct BMPInfoHeader {
    uint32_t biSize; //size of the info header in bytes (eg 40)
    int32_t biWidth; //width of the bitmap in pixels
    int32_t biHeight; //height of the bitmap in pixels
    uint16_t biPlanes; //number of color planes (must be 1)
    uint16_t biBitCount; //number of bits per pixel
    uint32_t biCompression; //type of compression (no compression is currently supported eg =0)
    uint32_t biSizeImage; //size of bitmap in bytes
    int32_t biXPelsPerMeter; //number of pixels per meter in x axis
    int32_t biYPelsPerMeter; //number of pixels per meter in y axis
    uint32_t biClrUsed; //number of colors used in the bitmap
    uint32_t biClrImportant; //number of colors that are important
} BMPInfoHeader;

typedef struct BMPColorEntry {
	uint8_t blue;
	uint8_t green;
	uint8_t red;
	uint8_t zero;
} BMPColorEntry;

#pragma pack(pop)



Image* readBMPImage(const char* path) {
	
	FILE *imageFile;
    BMPFileHeader bitmapFileHeader;
	BMPInfoHeader bitmapInfoHeader;
    
    imageFile = fopen(path,"rb");
    if (imageFile == NULL) {
		printf("Could not open file to read bitmap: %s\n", path);
        return NULL;
	}

    //read bitmap file header
	fread(&bitmapFileHeader, sizeof(BMPFileHeader), 1, imageFile);

    //verify the bitmap type is "BM"
    if (bitmapFileHeader.bmpType1 != 'B' || bitmapFileHeader.bmpType2 != 'M')
    {
		printf("Cannot load bitmap with type :%c%c\n", bitmapFileHeader.bmpType1, bitmapFileHeader.bmpType2);
        fclose(imageFile);
        return NULL;
    }

    //read the bitmap info header
    fread(&bitmapInfoHeader, sizeof(BMPInfoHeader), 1, imageFile);

	//printf("Width %d\nHeight %d\nCompression %d\nNum Colors %d\nBitCount %d\n", bitmapInfoHeader.biWidth, bitmapInfoHeader.biHeight, bitmapInfoHeader.biCompression, bitmapInfoHeader.biClrUsed, bitmapInfoHeader.biBitCount);
	
	if(bitmapInfoHeader.biCompression != 0) {
		printf("Cannot load bitmap with compression\n");
		fclose(imageFile);
        return NULL;
	}
	if(bitmapInfoHeader.biBitCount != 8 || (bitmapInfoHeader.biClrUsed != 0 && bitmapInfoHeader.biClrUsed != 256)) {
		printf("Cannot load bitmap with non 8bit color\n");
		fclose(imageFile);
        return NULL;
	}
	uint16_t colorsUsed = 256;
	
	BMPColorEntry colorTable[colorsUsed];
	fread(colorTable, sizeof(colorTable), 1, imageFile);
	
	im_t intensityTable[colorsUsed];
	size_t color;
	for(color = 0; color < colorsUsed; color++) {
		//set intensity to approximate luminosity
		//luminosity is computed as (0.3 R + 0.59 G + 0.11 B)
		intensityTable[color] = (im_t)(0.3 * colorTable[color].red + .59 * colorTable[color].red + 0.11 * colorTable[color].red);
	}
	
	uint8_t bmpData[bitmapInfoHeader.biSizeImage];
	
    //seek file to bitmap data
    fseek(imageFile, bitmapFileHeader.bOffBits, SEEK_SET);
	
	fread(bmpData, bitmapInfoHeader.biSizeImage, 1, imageFile);
	
	fclose(imageFile);
	
	size_t rows = bitmapInfoHeader.biHeight, cols = bitmapInfoHeader.biWidth;
	size_t fileCols = ((cols - 1) / 4 + 1) * 4;
	size_t imageSize = rows * cols;
	Image* bmpImage = allocateImage(rows, cols);
	im_t* imageData = (bmpImage->pixels)->mat;
	
	size_t i,j;
	for(i = 0; i < rows; i++) {
		for(j = 0; j < cols; j++) {
			//bmp stores image bottom to top
			imageData[i * cols + j] = intensityTable[bmpData[(rows - 1 - i) * fileCols + j]];
		}
	}
	
	return bmpImage;
}


bool writeBMPImage(const char* path, Image* im) {
	
	FILE *imageFile;
   	imageFile = fopen(path,"wb");
    if (imageFile == NULL) {
		printf("Could not open file to write bitmap: %s\n", path);
        return false;
	}

	uint32_t colorsUsed = 256;
	
	BMPColorEntry colorTable[colorsUsed];
	
	size_t color;
	for(color = 0; color < colorsUsed; color++) {
		colorTable[color].blue = color;
		colorTable[color].green = color;
		colorTable[color].red = color;
		colorTable[color].zero = 0;
	}
	
	size_t rows = im->pixels->rows, cols = im->pixels->cols;
	uint32_t headerSize = sizeof(BMPFileHeader) + sizeof(BMPInfoHeader) + sizeof(colorTable);
	uint32_t bitmapSize = sizeof(uint8_t) * rows * cols;
	
	BMPFileHeader bitmapFileHeader;
	bitmapFileHeader.bmpType1 = 'B';
	bitmapFileHeader.bmpType2 = 'M';
	bitmapFileHeader.bmpSize = headerSize + bitmapSize;
	bitmapFileHeader.bmpReserved1 = 0;
	bitmapFileHeader.bmpReserved2 = 0;
	bitmapFileHeader.bOffBits = headerSize;
	
	BMPInfoHeader bitmapInfoHeader;
	bitmapInfoHeader.biSize = 40;
	bitmapInfoHeader.biWidth = cols;
	bitmapInfoHeader.biHeight = rows;
	bitmapInfoHeader.biPlanes = 1;
	bitmapInfoHeader.biBitCount = 8;
	bitmapInfoHeader.biCompression = 0;
	bitmapInfoHeader.biSizeImage = bitmapSize;
	bitmapInfoHeader.biXPelsPerMeter = 1024;
	bitmapInfoHeader.biYPelsPerMeter = 1024;
	bitmapInfoHeader.biClrUsed = colorsUsed;
	bitmapInfoHeader.biClrImportant = 0;
	
	size_t fileCols = ((cols - 1) / 4 + 1) * 4;
	im_t* imageData = im->pixels->mat;
	uint8_t byteData[rows * fileCols];
	size_t i, j;
	for(i = 0; i < rows; i++) {
		for(j = 0; j < cols; j++) {
			byteData[i * fileCols + j] = (uint8_t)imageData[(rows - i - 1) * cols + j];
		}
		for(j = cols; j < fileCols; j++) {
			byteData[i * fileCols + j] = 0;
		}
	}
	
	fwrite(&bitmapFileHeader, sizeof(BMPFileHeader), 1, imageFile);
	fwrite(&bitmapInfoHeader, sizeof(BMPInfoHeader), 1, imageFile);
	fwrite(colorTable, sizeof(colorTable), 1, imageFile);
	fwrite(byteData, bitmapSize, 1, imageFile);
	
	fclose(imageFile);
	
	return true;
}