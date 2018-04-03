#ifndef _FFMPEG_H_
#define _FFMPEG_H_

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h> 

#include <stdbool.h>
#include <stdint.h>

typedef struct VideoReadStruct {
	AVFormatContext* formatContext;
	AVCodecContext* codecContext;
	AVCodec* codec;
	int videoIndex;
	int width;
	int height;
} VideoReadStruct;

typedef struct VideoWriteStruct {
	AVFormatContext* formatContext;
	AVCodecContext* codecContext;
	AVCodec* codec;
	int videoIndex;
	int width;
	int height;
	AVPacket* packet;
	AVStream* stream;
	AVFrame* frame;
	struct SwsContext *swsContext;
} VideoWriteStruct;


//function pointer typedef for callback for processing a single frame
typedef bool (*ProcessFrameCallback)(AVFrame *frame, int frameNumber, void* arg);


bool openVideoRead(VideoReadStruct** vrs, const char* inFileName);
bool processVideo(VideoReadStruct* vrs, ProcessFrameCallback processFrame, void* arg);
bool closeVideoRead(VideoReadStruct** vrs);

bool openVideoWrite(VideoWriteStruct** videoWrite, const char* outFileName, 
						int64_t bitrate, int width, int height, int framerate);
bool writeVideoFrame(VideoWriteStruct* vws, AVFrame *frame, int width, int height, int timestamp);
bool closeVideoWrite(VideoWriteStruct** videoWrite);

#endif