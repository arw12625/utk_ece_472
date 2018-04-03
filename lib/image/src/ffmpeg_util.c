/*
 * Code modified from example code at
 * http://dranger.com/ffmpeg/tutorial01.html
 * https://github.com/libav/libav/tree/master/doc/examples
 */

 #include "ffmpeg_util.h"

#include <libavutil/imgutils.h>
#include <libavformat/avio.h>
#include <libavutil/opt.h>


int allocateVideoReadStruct(VideoReadStruct** readStruct) {
	VideoReadStruct* vrs = malloc(sizeof(VideoReadStruct));
	if(!vrs) {
		return -1;
	}
	(*readStruct) = vrs;
	vrs->formatContext = NULL;
	vrs->codecContext = NULL;
	vrs->codec = NULL;
	vrs->videoIndex = -1;
	return 0;
}

int freeVideoReadStruct(VideoReadStruct** vrs) {
	free(*vrs);
	*vrs = NULL;
	return 0;
}

bool openVideoRead(VideoReadStruct** readStruct, const char* inFileName) {
	
	if(allocateVideoReadStruct(readStruct) < 0) {
		printf("Could not allocate video read structure\n");
		return false;
	}
	
	VideoReadStruct* vrs = *readStruct;

	// Open video file
	if(avformat_open_input(&(vrs->formatContext), inFileName, NULL, NULL) != 0) {
		printf("Could not open file: %s\n", inFileName);
		return false; // Couldn't open file
	}
	
	// Retrieve stream information
	if(avformat_find_stream_info(vrs->formatContext, NULL) < 0) {
		printf("Could not find stream info\n");
		return false; // Couldn't find stream information
	}
	
	// Dump information about file onto standard error
	av_dump_format(vrs->formatContext, 0, inFileName, 0);
	
	
	// Find the best video stream
	vrs->videoIndex = av_find_best_stream(vrs->formatContext,
							AVMEDIA_TYPE_VIDEO, -1, -1, &(vrs->codec), 0);
	
	if(vrs->videoIndex == -1 || vrs->codec == NULL) {
		printf("Could not find video stream and codex from context\n");
		return false; // Didn't find a video stream with the right type or codec
	}
	
	// Get a pointer to the codec context
	vrs->codecContext = avcodec_alloc_context3(vrs->codec);
	if(!(vrs->codecContext)) {
		printf("Could not allocate codec context\n");
		return false;
	}
	
	if(avcodec_parameters_to_context(vrs->codecContext,
			vrs->formatContext->streams[vrs->videoIndex]->codecpar) < 0) {
		printf("Could not copy parameters to context\n");
		return false;
	}
	
	// Open codec
	if (avcodec_open2(vrs->codecContext, vrs->codec, NULL) < 0) {
        printf("Could not open codec.\n");
        return false;
    }
	
	vrs->width =  vrs->codecContext->width;
	vrs->height = vrs->codecContext->height;
	if(vrs->width <= 0 || vrs->height <= 0) {
		printf("Video has zero width or height\n");
	}
	
	return true;
}

bool closeVideoRead(VideoReadStruct** videoRead) {
	
	VideoReadStruct* vrs = *videoRead;
	
	// Close the codec
	avcodec_free_context(&(vrs->codecContext));

	// Close the video file
	avformat_close_input(&(vrs->formatContext));

	//free the structure
	freeVideoReadStruct(videoRead);
	
	return true;
}


bool processVideo(VideoReadStruct* vrs, ProcessFrameCallback processFrame, void* arg) {
	
	// Allocate video frame for the original video
	// and another frame for the converted RGB frame
	AVFrame *pFrame = av_frame_alloc();
	AVFrame *pFrameRGB = av_frame_alloc();
	
	if(pFrame == NULL || pFrameRGB == NULL) {
		return false;
	}

	// Determine required buffer size and allocate buffer
	if(av_image_alloc(pFrameRGB->data, pFrameRGB->linesize, vrs->width,
			vrs->height, AV_PIX_FMT_RGB24, 32) < 0) {
		printf("Could not allocate frame.\n");
		return false;
	}
	
	
	AVPacket *packet;
	packet = av_packet_alloc();
    if (!packet) {
		printf("Could not allocate packet\n");
        return false;
	}
	
	struct SwsContext *sws_ctx = NULL;
	// initialize SWS context for software scaling
	sws_ctx = sws_getContext(vrs->width, vrs->height, vrs->codecContext->pix_fmt,
		vrs->width, vrs->height, AV_PIX_FMT_RGB24,
		SWS_BILINEAR, NULL, NULL, NULL
	);
	
	int i = 0;
	bool processRet = true;
	while(processRet && av_read_frame(vrs->formatContext, packet) >= 0) {
		// Is this a packet from the video stream?
		if(packet->stream_index == vrs->videoIndex) {
			if(avcodec_send_packet(vrs->codecContext, packet)) {
				printf("Could not send packet to decoder.\n");
				return false;
			}
			
			while(processRet && avcodec_receive_frame(vrs->codecContext, pFrame) >= 0) {
				// Convert the image from its native format to RGB
				sws_scale(sws_ctx, (uint8_t const * const *)pFrame->data,
							pFrame->linesize, 0, vrs->height,
							pFrameRGB->data, pFrameRGB->linesize);
				
				pFrameRGB->width = pFrame->width;
				pFrameRGB->height = pFrame->height;
				
				pFrameRGB->pts = pFrame->pts;
				// Process the frame with the provided callback
				processRet = (*processFrame)(pFrameRGB, i, arg);
			}
			
			i++;
		}

		// Free the packet that was allocated by av_read_frame
		av_packet_unref(packet);
		
	}
	
	// Free the RGB image
	av_freep(&pFrameRGB->data[0]);
	av_frame_free(&pFrameRGB);

	// Free the YUV frame
	av_frame_free(&pFrame);
}


int allocateVideoWriteStruct(VideoWriteStruct** videoWrite) {
	VideoWriteStruct* vws = malloc(sizeof(VideoWriteStruct));
	if(!vws) {
		return -1;
	}
	(*videoWrite) = vws;
	vws->formatContext = NULL;
	vws->codecContext = NULL;
	vws->codec = NULL;
	vws->videoIndex = -1;
	
	vws->packet == NULL;
	return 0;
}

int freeVideoWriteStruct(VideoWriteStruct** vws) {
	free(*vws);
	*vws = NULL;
	return 0;
}




bool openVideoWrite(VideoWriteStruct** videoWrite, const char* outFileName, 
						int64_t bitrate, int width, int height, int framerate) {
	
	
	if(allocateVideoWriteStruct(videoWrite) < 0) {
		printf("Could not allocate video write structure\n");
		return false;
	}
	VideoWriteStruct *vws = *videoWrite;
	
	avformat_alloc_output_context2(&(vws->formatContext), NULL, "mpeg", outFileName);
	if(!vws->formatContext) {
		printf("Could not allocate format context\n");
		return false;
	}
	
	AVOutputFormat* outputFormat = vws->formatContext->oformat;
	
	vws->codec = avcodec_find_encoder(outputFormat->video_codec);
	if(!vws->codec) {
		printf("Could not find H264 codec\n");
		return false;
	}
	
	vws->stream = avformat_new_stream(vws->formatContext, vws->codec);
	if(!vws->stream) {
		printf("Could not create new stream\n/");
		return false;
	}
	vws->stream->id = vws->formatContext->nb_streams - 1;
	vws->stream->time_base = (AVRational){1, framerate};
	
	vws->codecContext = avcodec_alloc_context3(vws->codec);
	if(!vws->codecContext) {
		printf("Could not allocate codec context");
		return false;
	}
	//const AVCodecID encoderID = AV_CODEC_ID_H264;
	
	
	if(width <= 0 || height <= 0) {
		printf("Video has zero width or height\n");
	}
	
	vws->width = width;
	vws->height = height;
	
	vws->codecContext->bit_rate = bitrate;
	vws->codecContext->width = width;
	vws->codecContext->height = height;
	vws->codecContext->time_base = (AVRational){1, framerate};
	vws->codecContext->framerate = (AVRational){framerate, 1};
	
	vws->codecContext->gop_size = 10;
	vws->codecContext->max_b_frames = 1;
	vws->codecContext->pix_fmt = AV_PIX_FMT_YUV420P;

	av_opt_set(vws->codecContext->priv_data, "preset", "slow", 0);
	
	if(avcodec_open2(vws->codecContext, vws->codec, NULL) < 0) {
		printf("Could not open codec\n");
		return false;
	}
	
	if(avcodec_parameters_from_context(vws->stream->codecpar, vws->codecContext) < 0) {
		printf("Could not get parameters from context\n");
		return false;
	}
	AVCPBProperties *props;
props = (AVCPBProperties*) av_stream_new_side_data(
    vws->stream, AV_PKT_DATA_CPB_PROPERTIES, sizeof(*props));
props->buffer_size = 1024 * 1024;
props->max_bitrate = 0;
props->min_bitrate = 0;
props->avg_bitrate = 0;
props->vbv_delay = UINT64_MAX;
	
	if(avio_open(&(vws->formatContext->pb), outFileName, AVIO_FLAG_WRITE) < 0) {
		printf("Could not open file: %s\n", outFileName);
		return false;
	}
	
	if(avformat_write_header(vws->formatContext, NULL) < 0) {
		printf("Could not write header\n");
		return false;
	}
	
	vws->frame = av_frame_alloc();
	if(!vws->frame) {
		printf("Could not allocate frame\n");
		return false;
	}
	vws->frame->format = vws->codecContext->pix_fmt;
	vws->frame->width = vws->width;
	vws->frame->height = vws->height;
	
	if(av_frame_get_buffer(vws->frame, 32) < 0) {
		printf("Could not create buffer for frame\n");
		return false;
	}
	av_frame_make_writable(vws->frame);
	
	vws->packet = av_packet_alloc();
	if(!vws->packet) {
		printf("Could not allocate packet\n");
		return false;
	}
	
	vws->swsContext = sws_getContext(vws->width, vws->height, AV_PIX_FMT_RGB24,
		vws->width, vws->height, vws->codecContext->pix_fmt,
		SWS_BILINEAR, NULL, NULL, NULL
	);
	
	return true;
	
}

bool writeVideoFrame(VideoWriteStruct* vws, AVFrame *frameRGB, int width, int height, int timestamp) {
	
	sws_scale(vws->swsContext, (uint8_t const * const *)frameRGB->data,
							frameRGB->linesize, 0, vws->height,
							vws->frame->data, vws->frame->linesize);
	
	vws->frame->pts = timestamp;//frameRGB->pts;
	
	if(avcodec_send_frame(vws->codecContext, vws->frame) < 0) {
		printf("Could not send frame\n");
		return false;
	}
	
	int ret = 0;
	while(ret >= 0) {
		ret = avcodec_receive_packet(vws->codecContext, vws->packet);
		
	//printf("DTS2: %d\n", vws->packet->dts);
		if(ret >= 0) {
		av_packet_rescale_ts(vws->packet, vws->codecContext->time_base, vws->stream->time_base);
			vws->packet->stream_index = vws->stream->index;
			av_interleaved_write_frame(vws->formatContext, vws->packet);
		}
		av_packet_unref(vws->packet);
	}
	
	return true;
	
}

bool closeVideoWrite(VideoWriteStruct** videoWrite) {
	VideoWriteStruct* vws = *videoWrite;
	av_write_trailer(vws->formatContext);
	
	avcodec_free_context(&(vws->codecContext));
	
	avio_closep(&(vws->formatContext->pb));
	
	avformat_free_context((vws->formatContext));
	
	freeVideoWriteStruct(videoWrite);
	return true;
	
    //av_frame_free(&ost->frame);
    //sws_freeContext(ost->sws_ctx);
    //swr_free(&ost->swr_ctx);
}

//need to free swsContext?
