#ifndef _IMAGEIO_H_
#define _IMAGEIO_H_

#include "Image.h"
#include <stdbool.h>

Image* readBMPImage(const char* path);
Image* readBMPImageChannelModel(const char* path, ImageChannelModel model);
bool writeBMPImageChannel8Bit(const char* path, size_t channel, Image* im);
bool writeBMPImage(const char* path, Image* im);

#endif
