#ifndef _IMAGEIO_H_
#define _IMAGEIO_H_

#include "Image.h"
#include <stdbool.h>

Image* readBMPImage(const char* path);
bool writeBMPImage(const char* path, Image* im);

#endif
