/*
 * Project 2b & 3
 * This program performs various image processing operations to video.
 * A Nuklear GUI is used to specify the operation and options as well as display the original and processed video
 * The LIBAV/FFMPEG libraries are used for video IO (reading and writing mpeg's)
 * All implementations of image operations are contained in image.c and image_processing.c in the lib/image directory
 *
 */

//include std libraries for Nuklear
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <math.h>
#include <limits.h>
#include <time.h>

//GLFW needed for windowing. Be sure to define GLFW_DLL if using the dll
#include <GL/glew.h>
#define GLFW_DLL
#include <GLFW/glfw3.h>

//Setup Nuklear variables and use glfw backend
#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_IMPLEMENTATION
#define NK_GLFW_GL3_IMPLEMENTATION
#include "nuklear.h"
#include "nuklear_glfw_gl3.h"

//buffer sizes for images
#define MAX_VERTEX_BUFFER 512 * 1024
#define MAX_ELEMENT_BUFFER 128 * 1024

//Project libraries
#include "ffmpeg_util.h"
#include "image.h"
#include "image_processing.h"

//The error callback for glfw
static void error_callback(int e, const char *d) {
	printf("Error %d: %s\n", e, d);
}

//the current window dimensions, updated by GLFW
int windowWidth = 0, windowHeight = 0;

//Initialize the GLFW window and Nuklear context. Must be called before using Nuklear
void initNuklearWindow(const char* title, GLFWwindow** winPointer, struct nk_context** ctxPointer) {
	
	/* GLFW */
	glfwSetErrorCallback(error_callback);
	if (!glfwInit()) {
		fprintf(stdout, "[GFLW] failed to init!\n");
		exit(1);
	}
	
	//Setup parameters for the GLFW window
	const GLFWvidmode * mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
    int desiredWidth = mode->width;
    int desiredHeight = mode->height;
	
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	//This must be set to maximize the window
	glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);
	
	//Create the GLFW Window
	*winPointer = glfwCreateWindow(desiredWidth, desiredHeight, title, NULL, NULL);
	glfwMakeContextCurrent(*winPointer);
	glfwGetWindowSize(*winPointer, &windowWidth, &windowHeight);
	
	//OpenGL init
	glViewport(0, 0, windowWidth, windowHeight);
	glewExperimental = 1;
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to setup GLEW\n");
		exit(1);
	}

	//Nuklear init (GLFW)
	*ctxPointer = nk_glfw3_init(*winPointer, NK_GLFW3_INSTALL_CALLBACKS);
	
	//Must enable opengl textures to display images/video
    glEnable(GL_TEXTURE_2D);
	{
		struct nk_font_atlas *atlas;
		nk_glfw3_font_stash_begin(&atlas);
		nk_glfw3_font_stash_end();
	}
}

//Update the window and Nuklear each frame before drawing Nuklear components
void preNuklearWindowUpdate(GLFWwindow* win, struct nk_context* ctx) {
	/* Input */
	glfwPollEvents();
	nk_glfw3_new_frame();
}

//Update the window and Nuklear each frame after drawing Nuklear components
void postNuklearWindowUpdate(GLFWwindow* win, struct nk_context* ctx) {
	/* Draw */
	glfwGetWindowSize(win, &windowWidth, &windowHeight);
	glViewport(0, 0, windowWidth, windowHeight);
	glClear(GL_COLOR_BUFFER_BIT);
	glClearColor(0, 0, 0, 1);
	/* IMPORTANT: `nk_glfw_render` modifies some global OpenGL state
	 * with blending, scissor, face culling, depth test and viewport and
	 * defaults everything back into a default state.
	 * Make sure to either a.) save and restore or b.) reset your own state after
	 * rendering the UI. */
	nk_glfw3_render(NK_ANTI_ALIASING_ON, MAX_VERTEX_BUFFER, MAX_ELEMENT_BUFFER);
	glfwSwapBuffers(win);
}

//Destroy and cleanup the window and GLFW
void destroyNuklearWindow(GLFWwindow* win, struct nk_context* ctx) {
	
	nk_glfw3_shutdown();
	nk_free(ctx);
	glfwTerminate();

}

//Create an image to display with Nuklear (allocate opengl texture)
struct nk_image createNuklearImage() {
	GLuint tex;
    glGenTextures(1, &tex);
    return nk_image_id((int)tex);
}

//Set the image data to display (set the opengl texture data)
void setNuklearImage(struct nk_image image, size_t rows, size_t cols, size_t numChannels, uint8_t* data) {
    
    GLuint tex = image.handle.id;
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	if(numChannels == 4) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, cols, rows, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	} else if(numChannels == 3) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, cols, rows, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	}
    glGenerateMipmap(GL_TEXTURE_2D);
}

//Prototypes for functions used in processing
bool writePPM(AVFrame *pFrame, int width, int height, int frameNumber, char *fileName);
bool setKernel(char* kernelStr, int kernelStrLen, float* kernel, int kerRows, int kerCols, int maxKerSize);

//Global Variables for options for Nuklear and processing each video frame

//Processing state
bool startProcessing = false;
bool processing = false;
bool endProcessing = false;

//Display frame rate variables
uint64_t delta_us;
struct timespec lastTime, currentTime;
char framerateDispBuffer[32];
uint64_t delayFramerateDisp = 0;

//Video Display variables
struct nk_image nkOrigFrame;
struct nk_image nkProcFrame;

//Video Options variables
char inFileName[64] = "video/big_buck_bunny.mp4";
char outFileName[64] = "video/processed.mp4";
int inFileNameLen = 24, outFileNameLen = 19;
//The list of video processes available
//Each entry must also correspond to an index defined below
const char* videoProcesses[] = {"Pass Through", "Invert", "Exponential", "Logarithm", "Power/Gamma", "Kernel", "Gradient", "RGB/HSI Channel", "RGB/HSI Scaling"};
int processesLen = 9;
int processSelection = 0;
#define VID_PROC_PASS 0
#define VID_PROC_INV 1
#define VID_PROC_EXP 2
#define VID_PROC_LOG 3
#define VID_PROC_POW 4
#define VID_PROC_KER 5
#define VID_PROC_GRAD 6
#define VID_PROC_RGB_CHAN 7
#define VID_PROC_RGB_SCALE 8

//Options for renormalizing the image after processing
enum {RANGE_TRUNCATE, RANGE_SCALE};

//Gamma/Power variables
float gammaPower = 1;

//Kernel variables
#define maxKernelSize 9
int kernelRows = 1;
int kernelCols = 1;
int enteredKernelRows = 1;
int enteredKernelCols = 1;
float kernelData[maxKernelSize * maxKernelSize] = {1};
char kernelStr[256] = "1";
int kernelStrLen = 1;
int useAbsoluteValue = false;
int enteredUseAbsoluteValue = false;
int kernelRangeOption = RANGE_TRUNCATE;

//Gradient variables
int gradientRangeOption = RANGE_TRUNCATE;

//RGB/HSI isolate channel variables
const char* displayChannelOptions[] = {"Red", "Green", "Blue", "Hue", "Saturation", "Intensity"};
int selectedDisplayChannel = 0;
int numDisplayChannels = 6;
#define DISPLAY_CHANNEL_RED 0
#define DISPLAY_CHANNEL_GREEN 1
#define DISPLAY_CHANNEL_BLUE 2
#define DISPLAY_CHANNEL_HUE 3
#define DISPLAY_CHANNEL_SAT 4
#define DISPLAY_CHANNEL_INT 5

//RGB/HSI scaling variables
float rgbScalers[3] = {1,1,1};
float hsiScalers[3] = {1,1,1};

//Frame writing option variables
char outFrameFileName[64] = "video/frames/frame%u.ppm";
int outFrameFileNameLen = 24;
int frameToWrite = -1;

//Update the Nuklear window for one frame
void updateNuklearWindow(GLFWwindow* win, struct nk_context* ctx) {
	
	//Update display frame rate times
	clock_gettime(CLOCK_MONOTONIC, &currentTime);
	delta_us = (currentTime.tv_sec - lastTime.tv_sec) * 1000000 + (currentTime.tv_nsec - lastTime.tv_nsec) / 1000;
	lastTime = currentTime;
	float currentFramerate = 1000000.0 / delta_us;
	
	//Must be called before drawing with Nuklear
	preNuklearWindowUpdate(win, ctx);
		
	//Nuklear GUI
	
	//Original Video Window
	if (nk_begin(ctx, "Original Video", nk_rect(0, 0, 700, 400),
		NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_SCALABLE|
		NK_WINDOW_MINIMIZABLE|NK_WINDOW_TITLE)) {
			
		struct nk_command_buffer *canvas;
		struct nk_color grid_color = nk_rgba(255, 255, 255, 255);
		struct nk_rect total_space = nk_window_get_content_region(ctx);
		canvas = nk_window_get_canvas(ctx);
		//display original video
		nk_draw_image(canvas, total_space, &nkOrigFrame, grid_color);
		
		nk_end(ctx);
	}
	
	//Processed Video Window
	if (nk_begin(ctx, "Processed Video", nk_rect(0, 420, 700, 400),
		NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_SCALABLE|
		NK_WINDOW_MINIMIZABLE|NK_WINDOW_TITLE)) {
			
		struct nk_command_buffer *canvas;
		struct nk_color grid_color = nk_rgba(255, 255, 255, 255);
		struct nk_rect total_space = nk_window_get_content_region(ctx);
		canvas = nk_window_get_canvas(ctx);
		//display processed video
		nk_draw_image(canvas, total_space, &nkProcFrame, grid_color);
		
		nk_end(ctx);
	}

	//Video Options
	if (nk_begin(ctx, "Video Options", nk_rect(720, 0, 500, 200),
		NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_SCALABLE|
		NK_WINDOW_MINIMIZABLE|NK_WINDOW_TITLE)) {
		
		//Video input file name text field
		nk_layout_row_dynamic(ctx, 30, 2);
		nk_label(ctx, "Input File Name:", NK_TEXT_LEFT);
		nk_edit_string(ctx, NK_EDIT_SIMPLE, inFileName, &inFileNameLen, 64, nk_filter_default);
		
		//Video output file name text field
		nk_layout_row_dynamic(ctx, 30, 2);
		nk_label(ctx, "Output File Name:", NK_TEXT_LEFT);
		nk_edit_string(ctx, NK_EDIT_SIMPLE, outFileName, &outFileNameLen, 64, nk_filter_default);
		
		//Button to start/stop video processing
		nk_layout_row_dynamic(ctx, 30, 2);
		char* startProcessingButtonString = "Start Processing";
		char* endProcessingButtonString = "End Processing";
		bool buttonReturn;
		//Select appropriate button text base on processing state
		if(processing) {
			buttonReturn = nk_button_label(ctx, endProcessingButtonString);
		} else {
			buttonReturn = nk_button_label(ctx, startProcessingButtonString);
		}
		if (buttonReturn) {
			// event handling
			if(!processing) {
				processing = true;
				startProcessing = true;
			} else {
				processing = false;
				endProcessing = true;
			}
		}
		
		//Video process selection is performed using a drop down combo box.
		//The selection text must correspond to the appropriate index defined above
		processSelection = nk_combo(ctx, videoProcesses, processesLen, processSelection, 25, nk_vec2(200, 200));
		
		nk_end(ctx);
	}
	
	//Frame Rate Display
	delayFramerateDisp += delta_us;
	if(delayFramerateDisp > 200000) {
		sprintf(framerateDispBuffer, "%f", currentFramerate);
		delayFramerateDisp = 0;
	}
	if (nk_begin(ctx, "Framerate", nk_rect(windowWidth - 64, windowHeight - 64, 64, 64), NK_WINDOW_NO_INPUT)) {
		nk_layout_row_dynamic(ctx, 30, 1);
		nk_label(ctx, framerateDispBuffer, NK_TEXT_LEFT);
		nk_end(ctx);
	}
	
	//Power/Gamma Options
	if(processSelection == VID_PROC_POW && nk_begin(ctx, "Power/Gamma Options", nk_rect(720, 220, 500, 200),
		NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_SCALABLE|
		NK_WINDOW_MINIMIZABLE|NK_WINDOW_TITLE)) {
		
		//Get value of gamma to use
		nk_layout_row_dynamic(ctx, 30, 1);
		nk_property_float(ctx, "Power", 0, &gammaPower, 50, gammaPower / 2.0, gammaPower / 10.0);
		
		nk_end(ctx);
	}
	
	//Kernel Options
	if (processSelection == VID_PROC_KER && nk_begin(ctx, "Kernel Options", nk_rect(720, 220, 500, 200),
		NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_SCALABLE|
		NK_WINDOW_MINIMIZABLE|NK_WINDOW_TITLE)) {
		
		//Get the kernel data to use in string form
		nk_layout_row_dynamic(ctx, 30, 2);
		nk_label(ctx, "Kernel:", NK_TEXT_LEFT);
		nk_edit_string(ctx, NK_EDIT_SIMPLE, kernelStr, &kernelStrLen, 64, nk_filter_default);
		
		//Get the number of kernel rows and columns
		nk_layout_row_dynamic(ctx, 30, 2);
		nk_property_int(ctx, "Kernel Rows", 1, &enteredKernelRows, maxKernelSize, 1, 1);
		nk_property_int(ctx, "Kernel Cols", 1, &enteredKernelCols, maxKernelSize, 1, 1);
		
		//Get renormalization options
		nk_layout_row_dynamic(ctx, 30, 3);
		nk_checkbox_label(ctx, "Absolute Value", &enteredUseAbsoluteValue);
		if(nk_option_label(ctx, "Truncate", kernelRangeOption == RANGE_TRUNCATE)) { kernelRangeOption = RANGE_TRUNCATE; }
		if(nk_option_label(ctx, "Scale", kernelRangeOption == RANGE_SCALE)) { kernelRangeOption = RANGE_SCALE; }
		
		//Button to set kernel data
		//The kernel is only updated after this is pressed
		nk_layout_row_dynamic(ctx, 30, 1);
		if (nk_button_label(ctx, "Set Kernel")) {
			// event handling
			if(setKernel(kernelStr, kernelStrLen, kernelData, enteredKernelRows, enteredKernelCols, maxKernelSize)) {
				kernelRows = enteredKernelRows;
				kernelCols = enteredKernelCols;
				useAbsoluteValue = enteredUseAbsoluteValue;
			}
		}
		nk_end(ctx);
	}
	
	//Gradient Options
	if (processSelection == VID_PROC_GRAD && nk_begin(ctx, "Gradient Options", nk_rect(720, 220, 500, 200),
		NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_SCALABLE|
		NK_WINDOW_MINIMIZABLE|NK_WINDOW_TITLE)) {
		
		//Get renormalization options
		nk_layout_row_dynamic(ctx, 30, 2);
		if(nk_option_label(ctx, "Truncate", gradientRangeOption == RANGE_TRUNCATE)) { gradientRangeOption = RANGE_TRUNCATE; }
		if(nk_option_label(ctx, "Scale", gradientRangeOption == RANGE_SCALE)) { gradientRangeOption = RANGE_SCALE; }

		nk_end(ctx);
	}
	
	//RGB/HSI Channel Options
	if (processSelection == VID_PROC_RGB_CHAN && nk_begin(ctx, "RGB/HSI Scaling Options", nk_rect(720, 220, 500, 250),
		NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_SCALABLE|
		NK_WINDOW_MINIMIZABLE|NK_WINDOW_TITLE)) {
			
		nk_layout_row_dynamic(ctx, 30, 2);
		nk_label(ctx, "Video Channel", NK_TEXT_LEFT);
		selectedDisplayChannel = nk_combo(ctx, displayChannelOptions, numDisplayChannels, selectedDisplayChannel, 25, nk_vec2(200, 200));
		nk_end(ctx);
	}
	
	//RGB/HSI Scaling Options
	if (processSelection == VID_PROC_RGB_SCALE && nk_begin(ctx, "RGB/HSI Scaling Options", nk_rect(720, 220, 500, 250),
		NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_SCALABLE|
		NK_WINDOW_MINIMIZABLE|NK_WINDOW_TITLE)) {
		
		//Get slider values for RGB
		nk_layout_row_dynamic(ctx, 30, 2);
		nk_label(ctx, "R", NK_TEXT_LEFT);
		nk_slider_float(ctx, 0, &(rgbScalers[CHANNEL_RED]), 1,  1.0 / (IMAGE_SCALE - 1));
		nk_layout_row_dynamic(ctx, 30, 2);
		nk_label(ctx, "G", NK_TEXT_LEFT);
		nk_slider_float(ctx, 0, &(rgbScalers[CHANNEL_GREEN]), 1,  1.0 / (IMAGE_SCALE - 1));
		nk_layout_row_dynamic(ctx, 30, 2);
		nk_label(ctx, "B", NK_TEXT_LEFT);
		nk_slider_float(ctx, 0, &(rgbScalers[CHANNEL_BLUE]), 1,  1.0 / (IMAGE_SCALE - 1));
		
		//Get slider values for HSI
		nk_layout_row_dynamic(ctx, 30, 2);
		nk_label(ctx, "H", NK_TEXT_LEFT);
		nk_slider_float(ctx, 0, &(hsiScalers[CHANNEL_HUE]), 1,  1.0 / (IMAGE_SCALE - 1));
		nk_layout_row_dynamic(ctx, 30, 2);
		nk_label(ctx, "S", NK_TEXT_LEFT);
		nk_slider_float(ctx, 0, &(hsiScalers[CHANNEL_SATURATION]), 1, 1.0 / (IMAGE_SCALE - 1));
		nk_layout_row_dynamic(ctx, 30, 2);
		nk_label(ctx, "I", NK_TEXT_LEFT);
		nk_slider_float(ctx, 0, &(hsiScalers[CHANNEL_INTENSITY]), 1,  1.0 / (IMAGE_SCALE - 1));
		
		nk_end(ctx);
	}
	
	//Frame Writing Options
	if (nk_begin(ctx, "Frame Writing Options", nk_rect(720, 570, 500, 250),
		NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_SCALABLE|
		NK_WINDOW_MINIMIZABLE|NK_WINDOW_TITLE)) {
		
		//Get output file name for frame
		nk_layout_row_dynamic(ctx, 30, 2);
		nk_label(ctx, "Output Frame File Name:", NK_TEXT_LEFT);
		nk_edit_string(ctx, NK_EDIT_SIMPLE, outFrameFileName, &outFrameFileNameLen, 64, nk_filter_default);
		
		//Get the number of the frame to write
		nk_layout_row_dynamic(ctx, 30, 1);
		nk_property_int(ctx, "Frame # to write", -1, &frameToWrite, 100000, 1, 1);
		
		nk_end(ctx);
	}
	
	//Must be called after drawing to Nuklear each frame
	postNuklearWindowUpdate(win, ctx);
}

//Structure to hold data for processing video not declared globally
struct ProcessData {
	VideoWriteStruct* vws;
	GLFWwindow* win;
	struct nk_context* ctx;
};

//The callback used to process a video frame
bool processFrame(AVFrame *frame, int frameNumber, void* arg) {
	
	//unpack process data
	struct ProcessData* pd = (struct ProcessData*)arg;
	VideoWriteStruct* vws = pd->vws;
	GLFWwindow* win = pd->win;
	struct nk_context* ctx = pd->ctx;
	
	printf("Frame #%u\n", frameNumber);
	
	//get video frame dimensions
	int width = frame->width;
	int height = frame->height;
		
	//Create an image from the data held in the frame
	Image* frameImage = createImageFromBytes(height, width, 3, frame->data[0]);
	frameImage->channelModel = CHANNEL_MODEL_RGB;
	
	//Set the data for the original video display from the frame
	setNuklearImage(nkOrigFrame, height, width, 3, frame->data[0]);
	
	
	//Video frame processing methods
	//The frame is processed according to the selected processing methods
	
	//Pass Through
	if(processSelection == VID_PROC_PASS) {
		//no processing necessary
	}
	//Invert
	if(processSelection == VID_PROC_INV) {
		invertImage(frameImage);
		//set the frame data to display the processed video
		getBytesFromImage(frame->data[0], frameImage);
	}
	//Exponential
	if(processSelection == VID_PROC_EXP) {
		exponentiateImage(frameImage);
		getBytesFromImage(frame->data[0], frameImage);
	}
	//Logarithm
	if(processSelection == VID_PROC_LOG) {
		logarithmImage(frameImage);
		getBytesFromImage(frame->data[0], frameImage);
	}
	//Power
	if(processSelection == VID_PROC_POW) {
		powerLawImage(frameImage, gammaPower);
		getBytesFromImage(frame->data[0], frameImage);
	}
	//Kernel
	if(processSelection == VID_PROC_KER) {
		
		//Create the kernel matrix from the kernel data
		float kernelMatrix[kernelRows][kernelCols];
		size_t i,j;
		for(i = 0; i < kernelRows; i++) {
			for(j = 0; j < kernelCols; j++) {
				kernelMatrix[i][j] = kernelData[j + i * kernelCols];
				//printf("%f, ", kernelMatrix[i][j]);
			}
			//printf("\n");
		}
		//apply the kernel to the frame image
		applyImageKernel(kernelRows, kernelCols, kernelMatrix, frameImage);
		
		if(useAbsoluteValue) {
			imageAbsoluteValue(frameImage);
		}
		
		//renormalize as specified
		if(kernelRangeOption == RANGE_TRUNCATE) {
			truncateImageRange(0, 255, frameImage);
		} else {
			scaleRangeImageUniform(0, 255, frameImage);
		}
		
		getBytesFromImage(frame->data[0], frameImage);
	}
	//Gradient
	if(processSelection == VID_PROC_GRAD) {
		
		Image* gradIm = computeGradientImage(frameImage);
		//renormalize as specified
		if(gradientRangeOption == RANGE_TRUNCATE) {
			truncateImageRange(0, 255, frameImage);
		} else {
			scaleRangeImageUniform(0, 255, gradIm);
		}
		getBytesFromImage(frame->data[0], gradIm);
		freeImage(gradIm);
	}
	//RGB HSI Display Channel
	if(processSelection == VID_PROC_RGB_CHAN) {
		
		Image* channelImage = allocateImageWithModel(frameImage->rows, frameImage->cols, frameImage->channelModel);
		
		//get channel index from selection for channel to display
		size_t channelIndex = 0;	
		switch(selectedDisplayChannel) {
			case DISPLAY_CHANNEL_RED : { channelIndex = CHANNEL_RED; break;}
			case DISPLAY_CHANNEL_GREEN : { channelIndex = CHANNEL_GREEN; break;}
			case DISPLAY_CHANNEL_BLUE : { channelIndex = CHANNEL_BLUE; break;}
			case DISPLAY_CHANNEL_HUE : { channelIndex = CHANNEL_HUE; break;}
			case DISPLAY_CHANNEL_SAT : { channelIndex = CHANNEL_SATURATION; break;}
			case DISPLAY_CHANNEL_INT : { channelIndex = CHANNEL_INTENSITY; break;}
		}
		
		if(selectedDisplayChannel == DISPLAY_CHANNEL_HUE || selectedDisplayChannel == DISPLAY_CHANNEL_SAT || selectedDisplayChannel == DISPLAY_CHANNEL_INT) {
			//if one of the HSI channels is selected, first convert the frame to HSI
			Image* hsiImage = NULL;
			hsiImage = convertImageRGB_HSI(frameImage);
			//Then copy that channel into the red, green, and blue channels of the display image 
			copyImageChannelData(CHANNEL_RED, channelImage, channelIndex, hsiImage);
			copyImageChannelData(CHANNEL_GREEN, channelImage, channelIndex, hsiImage);
			copyImageChannelData(CHANNEL_BLUE, channelImage, channelIndex, hsiImage);
			freeImage(hsiImage);
		} else {
			//Copy the selected channel into the red, green, and blue channels of the display image 
			copyImageChannelData(CHANNEL_RED, channelImage, channelIndex, frameImage);
			copyImageChannelData(CHANNEL_GREEN, channelImage, channelIndex, frameImage);
			copyImageChannelData(CHANNEL_BLUE, channelImage, channelIndex, frameImage);
		}
		getBytesFromImage(frame->data[0], channelImage);
		freeImage(channelImage);
	}
	//RGB HSI Scale
	if(processSelection == VID_PROC_RGB_SCALE) {
		
		//perform RGB scaling first
		scaleImageChannels(rgbScalers, frameImage);
		
		//convert result to HSI
		Image* hsiImage = convertImageRGB_HSI(frameImage);
		//next perform scaling on the HSI image
		scaleImageChannels(hsiScalers, hsiImage);
		
		//convert result back to RGB
		Image* rgbImage = convertImageHSI_RGB(hsiImage);
		getBytesFromImage(frame->data[0], rgbImage);
		
		freeImage(hsiImage);
		freeImage(rgbImage);
	}
	
	freeImage(frameImage);
	
	//Set the data for the processed video display from the frame
	setNuklearImage(nkProcFrame, height, width, 3, frame->data[0]);

	//Write the frame if the current frame number is the one specified
	if(frameNumber == frameToWrite) {
		//Null terminate the file name as Nuklear does not
		outFrameFileName[outFrameFileNameLen] = 0;
		writePPM(frame, width, height, frameNumber, outFrameFileName);
	}
	
	//Update the GUI 
	//This must be called each frame for the GUI to remain responsive
	updateNuklearWindow(win, ctx);
	
	//Write the processed frame to the output stream
	writeVideoFrame(vws, frame, width, height, frameNumber);
	
	//Returns whether or not to continue processing
	return !glfwWindowShouldClose(win) && !endProcessing;
}

//write a single frame to a PPM file
bool writePPM(AVFrame *pFrame, int width, int height, int frameNumber, char *fileName) {

  FILE *pFile;
  char fileNameBuffer[128];
  
  // Open file
  sprintf(fileNameBuffer, fileName, frameNumber);
  pFile=fopen(fileNameBuffer, "wb");
  if(pFile==NULL) {
    return false;
  }
  
  // Write header
  fprintf(pFile, "P6\n%d %d\n255\n", width, height);
  
  // Write pixel data
  int  i;
  for(i = 0; i < height; i++) {
    fwrite(pFrame->data[0] + i * pFrame->linesize[0], 1, width * 3, pFile);
  }
  
  // Close file
  fclose(pFile);
  
  return true;
}

//set the floating point kernel data using the string input
bool setKernel(char* kernelStr, int kernelStrLen, float* kernel, int kerRows, int kerCols, int maxKerSize) {
	
	char kernelStrBuf[256];
	kernelStr[kernelStrLen] = 0;
	sprintf(kernelStrBuf, "%s", kernelStr);
	
	if(kerRows >= maxKerSize || kerCols >= maxKerSize) {
		printf("Rows or Columns exceed maximum kernel dimension\n");
		return false;
	}
	if(kerRows % 2 != 1 || kerCols % 2 != 1) {
		printf("Kernel Dimensions must be odd\n");
		return false;
	}
	
	char *delim = " ,\n\r";
	char *token = NULL;
	int index = 0;
	
	int maxNumEntries = kerRows * kerCols;
	
	for (token = strtok(kernelStrBuf, delim); token != NULL; token = strtok(NULL, delim)) {
		if(index == maxNumEntries) {
			printf("Max kernel exceeded\n");
			return false;
		}
		kernel[index++] = (float)strtod(token, NULL);
	}
	
	if(index != kerRows * kerCols) {
		printf("Number of kernel entries does not match entered value\n");
		return false;
	}
	
	/*
	size_t i,j;
	for(i = 0; i < kerRows; i++) {
		for(j = 0; j < kerCols; j++) {
			printf("%f, ", kernel[j + i * kerCols]);
		}
		printf("\n");
	}
	*/
	
	return true;
}

int main( )
{
	/* Declare GLFW window and Nuklear context */
	GLFWwindow *win;
	struct nk_context *ctx;

	//Initialize the window and Nuklear
	initNuklearWindow("Video Processing", &win, &ctx);
	
	//Allocate and clear images for display
	//One for the original and one for the processed
	nkOrigFrame = createNuklearImage();
	nkProcFrame = createNuklearImage();
	uint8_t* tempFill = calloc(640 * 360 * 3, sizeof(uint8_t));
	setNuklearImage(nkOrigFrame, 360, 640, 3, tempFill);
	setNuklearImage(nkProcFrame, 360, 640, 3, tempFill);
	free(tempFill);
	
	//Initialize the clock for display frame rate approximation
	clock_gettime(CLOCK_MONOTONIC, &lastTime);
	
	//Main loop for GUI and processing
	while (!glfwWindowShouldClose(win))
	{
		
		if(!startProcessing) {
			//until processing is requested, update the gui
			updateNuklearWindow(win, ctx);
		} else {
			//when processing is requested
			startProcessing = false;
			
			//declare the structures for video reading/writing
			VideoWriteStruct* vws = NULL;
			VideoReadStruct* vrs = NULL;
			
			//Null terminate the file names as Nuklear does not
			inFileName[inFileNameLen] = 0;
			outFileName[outFileNameLen] = 0;
			
			//open the video read and write streams
			if(!openVideoRead(&vrs, inFileName)) {
				continue;
			}
			if(!openVideoWrite(&vws, outFileName, 
					vrs->codecContext->bit_rate, vrs->width, vrs->height, 24)) {
						continue;
			}
			
			//Setup struct for video processing
			//All data needed for processing that is not declared globally must be in this
			struct ProcessData pd;
			pd.vws = vws; pd.win = win; pd.ctx = ctx;
			
			//Begin processing the video with the provided process frame callback
			//Note that this call only returns when the video is processed
			//Thus the gui must be updated in the process callback
			processVideo(vrs, &processFrame, &pd);
			
			//when processing has finished, close the video streams
			closeVideoRead(&vrs);
			closeVideoWrite(&vws);
			
			processing = false;
			endProcessing = false;
		}
	}
	
	//cleanup window after close is requested
	destroyNuklearWindow(win, ctx);
	return 0;
	
}