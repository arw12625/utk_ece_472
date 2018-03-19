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
	uint8_t alpha;
} BMPColorEntry;

#pragma pack(pop)

Image* readBMPImage(const char* path) {
	return readBMPImageChannelModel(path, CHANNEL_MODEL_RGB);
}

Image* readBMPImageChannelModel(const char* path, ImageChannelModel model) {
	
	size_t numChannels = 0;
	switch(model) {
		case CHANNEL_MODEL_GRAY :
			numChannels = 1; break;
		case CHANNEL_MODEL_RGB :
			numChannels = 3; break;
		default :
			printf("Unsupported image channel model\n"); return NULL;
	}
	
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
	
	if(bitmapInfoHeader.biBitCount == 8 && bitmapInfoHeader.biClrUsed == 256) {
		uint32_t colorsUsed = bitmapInfoHeader.biClrUsed;
		BMPColorEntry colorTable[colorsUsed];
		fread(colorTable, sizeof(colorTable), 1, imageFile);

		uint8_t bmpData[bitmapInfoHeader.biSizeImage];
		//seek file to bitmap data
		fseek(imageFile, bitmapFileHeader.bOffBits, SEEK_SET);
		fread(bmpData, bitmapInfoHeader.biSizeImage, 1, imageFile);
		fclose(imageFile);
		
		size_t rows = bitmapInfoHeader.biHeight, cols = bitmapInfoHeader.biWidth;
		size_t fileCols = ((cols - 1) / 4 + 1) * 4;
		size_t imageSize = rows * cols;
		Image* bmpImage = allocateImageWithModel(rows, cols, model);
		size_t i,j, channel;
		switch(model) {
			case CHANNEL_MODEL_GRAY : {
				im_t intensityTable[colorsUsed];
				size_t colorInd;
				for(colorInd = 0; colorInd < colorsUsed; colorInd++) {
					//set intensity to approximate luminosity
					//luminance is computed as (0.3 R + 0.59 G + 0.11 B)
					intensityTable[colorInd] = (im_t)(0.3 * colorTable[colorInd].red + .59 * colorTable[colorInd].green + 0.11 * colorTable[colorInd].blue);
				}
				for(i = 0; i < rows; i++) {
					for(j = 0; j < cols; j++) {
						//bmp stores image bottom to top
						//imageData[i * cols + j] = intensityTable[bmpData[(rows - 1 - i) * fileCols + j]];
						bmpImage->channels[CHANNEL_GRAY]->mat[i * cols + j] = intensityTable[bmpData[(rows - 1 - i) * fileCols + j]];
					}
				}
				break;
			} case CHANNEL_MODEL_RGB : {
			//case CHANNEL_MODEL_RGBA :
				BMPColorEntry color;
				for(i = 0; i < rows; i++) {
					for(j = 0; j < cols; j++) {
						//bmp stores image bottom to top
						color = colorTable[bmpData[(rows - 1 - i) * fileCols + j]];
						bmpImage->channels[CHANNEL_RED]->mat[i * cols + j] = color.red;
						bmpImage->channels[CHANNEL_GREEN]->mat[i * cols + j] = color.green;
						bmpImage->channels[CHANNEL_BLUE]->mat[i * cols + j] = color.blue;
						/*if(model == CHANNEL_MODEL_RGBA) {
							bmpImage->channels[CHANNEL_ALPHA]->mat[i * cols + j] = color.alpha;
						}*/
					}
				}
				break;
			}
			
		}
		return bmpImage;
	} else if(bitmapInfoHeader.biBitCount == 24 && bitmapInfoHeader.biClrUsed == 0) {
		uint8_t bmpData[bitmapInfoHeader.biSizeImage];
		//seek file to bitmap data
		fseek(imageFile, bitmapFileHeader.bOffBits, SEEK_SET);
		fread(bmpData, bitmapInfoHeader.biSizeImage, 1, imageFile);
		fclose(imageFile);
		
		size_t rows = bitmapInfoHeader.biHeight, cols = bitmapInfoHeader.biWidth;
		size_t fileCols = ((3 * cols - 1) / 4 + 1) * 4;
		size_t imageSize = rows * cols;
		Image* bmpImage = allocateImageWithModel(rows, cols, model);
		size_t i,j, channel;
		switch(model) {
			case CHANNEL_MODEL_GRAY : {
				for(i = 0; i < rows; i++) {
					for(j = 0; j < cols; j++) {
						//bmp stores image bottom to top
						uint8_t blue = bmpData[(rows - 1 - i) * fileCols + j * 3 + 0];
						uint8_t green = bmpData[(rows - 1 - i) * fileCols + j * 3 + 1];
						uint8_t red = bmpData[(rows - 1 - i) * fileCols + j * 3 + 2];
						im_t val = (im_t)(0.3 * red + .59 * green + 0.11 * blue);
						bmpImage->channels[CHANNEL_GRAY]->mat[i * cols + j] = val;
					}
				}
				break;
			} case CHANNEL_MODEL_RGB : {
				BMPColorEntry color;
				for(i = 0; i < rows; i++) {
					for(j = 0; j < cols; j++) {
						//bmp stores image bottom to top
						bmpImage->channels[CHANNEL_BLUE]->mat[i * cols + j] = bmpData[(rows - 1 - i) * fileCols + j * 3 + 0];
						bmpImage->channels[CHANNEL_GREEN]->mat[i * cols + j] = bmpData[(rows - 1 - i) * fileCols + j * 3 + 1];
						bmpImage->channels[CHANNEL_RED]->mat[i * cols + j] = bmpData[(rows - 1 - i) * fileCols + j * 3 + 2];
					}
				}
				break;
			}
		}
		return bmpImage;
	} else {
		printf("Unsupported BMP format\n");
		return NULL;
	}
	
}


bool writeBMPImageChannel8Bit(const char* path, size_t channel, Image* im) {
	
	FILE *imageFile;
   	imageFile = fopen(path,"wb");
    if (imageFile == NULL) {
		printf("Could not open file to write bitmap: %s\n", path);
        return false;
	}
	
	size_t colorsUsed = 256;
	BMPColorEntry colorTable[colorsUsed];
	size_t color;
	for(color = 0; color < colorsUsed; color++) {
		colorTable[color].blue = color;
		colorTable[color].green = color;
		colorTable[color].red = color;
		colorTable[color].alpha = 0;
	}
	
	size_t rows = im->rows, cols = im->cols;
	uint32_t headerSize = sizeof(BMPFileHeader) + sizeof(BMPInfoHeader) + sizeof(colorTable);
	size_t fileCols = ((cols - 1) / 4 + 1) * 4;
	uint32_t bitmapSize = sizeof(uint8_t) * rows * fileCols;
	
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
	
	uint8_t byteData[rows * fileCols];
	size_t i, j;
	for(i = 0; i < rows; i++) {
		for(j = 0; j < cols; j++) {
			byteData[i * fileCols + j] = (uint8_t)im->channels[channel]->mat[(rows - i - 1) * cols + j];
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

bool writeBMPImage(const char* path, Image* im) {
	
	if(im->channelModel != CHANNEL_MODEL_GRAY
		&& im->channelModel != CHANNEL_MODEL_RGB) {
			printf("Unsupported image channel model\n");
			return NULL;
	}
	
	FILE *imageFile;
   	imageFile = fopen(path,"wb");
    if (imageFile == NULL) {
		printf("Could not open file to write bitmap: %s\n", path);
        return false;
	}
	
	size_t rows = im->rows, cols = im->cols;
	uint32_t headerSize = sizeof(BMPFileHeader) + sizeof(BMPInfoHeader);
	size_t fileCols = ((3 * cols - 1) / 4 + 1) * 4;
	uint32_t bitmapSize = sizeof(uint8_t) * rows * fileCols;
	
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
	bitmapInfoHeader.biBitCount = 24;
	bitmapInfoHeader.biCompression = 0;
	bitmapInfoHeader.biSizeImage = bitmapSize;
	bitmapInfoHeader.biXPelsPerMeter = 1024;
	bitmapInfoHeader.biYPelsPerMeter = 1024;
	bitmapInfoHeader.biClrUsed = 0;
	bitmapInfoHeader.biClrImportant = 0;
	
	uint8_t byteData[rows * fileCols];
	size_t i, j;
	for(i = 0; i < rows; i++) {
		for(j = 0; j < cols; j++) {
			byteData[i * fileCols + j * 3 + 0] = (uint8_t)im->channels[CHANNEL_BLUE]->mat[(rows - i - 1) * cols + j * 3 + 0];
			byteData[i * fileCols + j * 3 + 1] = (uint8_t)im->channels[CHANNEL_GREEN]->mat[(rows - i - 1) * cols + j * 3 + 1];
			byteData[i * fileCols + j * 3 + 2] = (uint8_t)im->channels[CHANNEL_RED]->mat[(rows - i - 1) * cols + j * 3 + 2];
		}
		for(j = 3 * cols; j < fileCols; j++) {
			byteData[i * fileCols + j] = 0;
		}
	}
	
	fwrite(&bitmapFileHeader, sizeof(BMPFileHeader), 1, imageFile);
	fwrite(&bitmapInfoHeader, sizeof(BMPInfoHeader), 1, imageFile);
	fwrite(byteData, bitmapSize, 1, imageFile);
	
	fclose(imageFile);
	
	return true;
}