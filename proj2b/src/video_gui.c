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


#include <GL/glew.h>
#define GLFW_DLL
#include <GLFW/glfw3.h>

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

#include "ffmpeg_util.h"
#include "image.h"
#include "image_processing.h"

#define MAX_VERTEX_BUFFER 512 * 1024
#define MAX_ELEMENT_BUFFER 128 * 1024

static void error_callback(int e, const char *d) {
	printf("Error %d: %s\n", e, d);
}

int windowWidth = 0, windowHeight = 0;

void initNuklearWindow(const char* title, GLFWwindow** winPointer, struct nk_context** ctxPointer) {
	
	/* GLFW */
	glfwSetErrorCallback(error_callback);
	if (!glfwInit()) {
		fprintf(stdout, "[GFLW] failed to init!\n");
		exit(1);
	}
	const GLFWvidmode * mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
    int desiredWidth = mode->width;
    int desiredHeight = mode->height;
	
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);
	
	*winPointer = glfwCreateWindow(desiredWidth, desiredHeight, title, NULL, NULL);
	glfwMakeContextCurrent(*winPointer);
	glfwGetWindowSize(*winPointer, &windowWidth, &windowHeight);
	
	/* OpenGL */
	glViewport(0, 0, windowWidth, windowHeight);
	glewExperimental = 1;
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to setup GLEW\n");
		exit(1);
	}

	*ctxPointer = nk_glfw3_init(*winPointer, NK_GLFW3_INSTALL_CALLBACKS);
	
    glEnable(GL_TEXTURE_2D);
	{
		struct nk_font_atlas *atlas;
		nk_glfw3_font_stash_begin(&atlas);
		nk_glfw3_font_stash_end();
	}
}

void preNuklearWindowUpdate(GLFWwindow* win, struct nk_context* ctx) {
	/* Input */
	glfwPollEvents();
	nk_glfw3_new_frame();
}

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

void destroyNuklearWindow(GLFWwindow* win, struct nk_context* ctx) {
	
	nk_glfw3_shutdown();
	nk_free(ctx);
	glfwTerminate();

}

struct nk_image createNuklearImage() {
	GLuint tex;
    glGenTextures(1, &tex);
    return nk_image_id((int)tex);
}

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

struct nk_image nkOrigFrame;
struct nk_image nkProcFrame;
char inFileName[64] = "video/big_buck_bunny.mp4";
char outFileName[64] = "video/processed.mp4";
int inFileNameLen = 24, outFileNameLen = 19;
bool startProcessing = false;
bool processing = false;
bool endProcessing = false;

uint64_t delta_us;
struct timespec lastTime, currentTime;
char framerateDispBuffer[32];
uint64_t delayFramerateDisp = 0;

const char* videoProcesses[] = {"Pass Through", "Invert", "Exponential", "Logarithm", "Power/Gamma", "Kernel", "Gradient", "RGB-HSI Adjustment"};
int processesLen = 8;
int processSelection = 0;

#define VID_PROC_PASS 0
#define VID_PROC_INV 1
#define VID_PROC_EXP 2
#define VID_PROC_LOG 3
#define VID_PROC_POW 4
#define VID_PROC_KER 5
#define VID_PROC_GRAD 6
#define VID_PROC_RGB 7

float gammaPower = 1;

enum {RANGE_TRUNCATE, RANGE_SCALE};

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

bool setKernel(char* kernelStr, int kernelStrLen, float* kernel, int kerRows, int kerCols, int maxKerSize);

int gradientRangeOption = RANGE_TRUNCATE;

float rgbScalers[3] = {1,1,1};
float hsiScalers[3] = {1,1,1};
float old_rgbScalers[3];
float old_hsiScalers[3];

void updateNuklearWindow(GLFWwindow* win, struct nk_context* ctx) {
	clock_gettime(CLOCK_MONOTONIC, &currentTime);
	delta_us = (currentTime.tv_sec - lastTime.tv_sec) * 1000000 + (currentTime.tv_nsec - lastTime.tv_nsec) / 1000;
	lastTime = currentTime;
	float currentFramerate = 1000000.0 / delta_us;
	
	preNuklearWindowUpdate(win, ctx);
		
	/* GUI */
	//Original Video Window
	if (nk_begin(ctx, "Original Video", nk_rect(0, 0, 700, 400),
		NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_SCALABLE|
		NK_WINDOW_MINIMIZABLE|NK_WINDOW_TITLE)) {
			
		struct nk_command_buffer *canvas;
		struct nk_color grid_color = nk_rgba(255, 255, 255, 255);
		struct nk_rect total_space = nk_window_get_content_region(ctx);
		canvas = nk_window_get_canvas(ctx);
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
		nk_draw_image(canvas, total_space, &nkProcFrame, grid_color);
		
		nk_end(ctx);
	}

	//Video Options
	if (nk_begin(ctx, "Video Options", nk_rect(720, 0, 500, 200),
		NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_SCALABLE|
		NK_WINDOW_MINIMIZABLE|NK_WINDOW_TITLE)) {
		
		nk_layout_row_dynamic(ctx, 30, 2);
		nk_label(ctx, "Input File Name:", NK_TEXT_LEFT);
		nk_edit_string(ctx, NK_EDIT_SIMPLE, inFileName, &inFileNameLen, 64, nk_filter_default);
		nk_layout_row_dynamic(ctx, 30, 2);
		nk_label(ctx, "Output File Name:", NK_TEXT_LEFT);
		nk_edit_string(ctx, NK_EDIT_SIMPLE, outFileName, &outFileNameLen, 64, nk_filter_default);
		
		nk_layout_row_dynamic(ctx, 30, 2);
		char* startProcessingButtonString = "Start Processing";
		char* endProcessingButtonString = "End Processing";
		bool buttonReturn;
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
		
		nk_layout_row_dynamic(ctx, 30, 1);
		nk_property_float(ctx, "Power", 0, &gammaPower, 50, gammaPower / 2.0, gammaPower / 10.0);
		
		nk_end(ctx);
	}
	
	//Kernel Options
	if (processSelection == VID_PROC_KER && nk_begin(ctx, "Kernel Options", nk_rect(720, 220, 500, 200),
		NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_SCALABLE|
		NK_WINDOW_MINIMIZABLE|NK_WINDOW_TITLE)) {
		
		nk_layout_row_dynamic(ctx, 30, 2);
		nk_label(ctx, "Kernel:", NK_TEXT_LEFT);
		nk_edit_string(ctx, NK_EDIT_SIMPLE, kernelStr, &kernelStrLen, 64, nk_filter_default);
		
		nk_layout_row_dynamic(ctx, 30, 2);
		nk_property_int(ctx, "Kernel Rows", 1, &enteredKernelRows, maxKernelSize, 1, 1);
		nk_property_int(ctx, "Kernel Cols", 1, &enteredKernelCols, maxKernelSize, 1, 1);
		
		nk_layout_row_dynamic(ctx, 30, 3);
		nk_checkbox_label(ctx, "Absolute Value", &enteredUseAbsoluteValue);
		if(nk_option_label(ctx, "Truncate", kernelRangeOption == RANGE_TRUNCATE)) { kernelRangeOption = RANGE_TRUNCATE; }
		if(nk_option_label(ctx, "Scale", kernelRangeOption == RANGE_SCALE)) { kernelRangeOption = RANGE_SCALE; }
		
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
		
		nk_layout_row_dynamic(ctx, 30, 2);
		if(nk_option_label(ctx, "Truncate", gradientRangeOption == RANGE_TRUNCATE)) { gradientRangeOption = RANGE_TRUNCATE; }
		if(nk_option_label(ctx, "Scale", gradientRangeOption == RANGE_SCALE)) { gradientRangeOption = RANGE_SCALE; }

		nk_end(ctx);
	}
	
	//Scaling Options
	if (processSelection == VID_PROC_RGB && nk_begin(ctx, "Scaling Options", nk_rect(720, 220, 500, 250),
		NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_SCALABLE|
		NK_WINDOW_MINIMIZABLE|NK_WINDOW_TITLE)) {
		
		nk_layout_row_dynamic(ctx, 30, 2);
		nk_label(ctx, "R", NK_TEXT_LEFT);
		nk_slider_float(ctx, 0, &(rgbScalers[CHANNEL_RED]), 1,  1.0 / (IMAGE_SCALE - 1));
		nk_layout_row_dynamic(ctx, 30, 2);
		nk_label(ctx, "G", NK_TEXT_LEFT);
		nk_slider_float(ctx, 0, &(rgbScalers[CHANNEL_GREEN]), 1,  1.0 / (IMAGE_SCALE - 1));
		nk_layout_row_dynamic(ctx, 30, 2);
		nk_label(ctx, "B", NK_TEXT_LEFT);
		nk_slider_float(ctx, 0, &(rgbScalers[CHANNEL_BLUE]), 1,  1.0 / (IMAGE_SCALE - 1));
		
		nk_layout_row_dynamic(ctx, 30, 2);
		nk_label(ctx, "H", NK_TEXT_LEFT);
		nk_slider_float(ctx, 0, &(hsiScalers[CHANNEL_HUE]), 1,  1.0 / (IMAGE_SCALE - 1));
		nk_layout_row_dynamic(ctx, 30, 2);
		nk_label(ctx, "S", NK_TEXT_LEFT);
		nk_slider_float(ctx, 0, &(hsiScalers[CHANNEL_SATURATION]), 1, 1.0 / (IMAGE_SCALE - 1));
		nk_layout_row_dynamic(ctx, 30, 2);
		nk_label(ctx, "I", NK_TEXT_LEFT);
		nk_slider_float(ctx, 0, &(hsiScalers[CHANNEL_INTENSITY]), 1,  1.0 / (IMAGE_SCALE - 1));

		
		size_t scaleIndex;
		bool rgbChanged = false;
		bool hsiChanged = false;
		for(scaleIndex = 0; scaleIndex < 3; scaleIndex++) {
			rgbChanged = rgbChanged || (old_rgbScalers[scaleIndex] != rgbScalers[scaleIndex]);
			old_rgbScalers[scaleIndex] = rgbScalers[scaleIndex];
			hsiChanged = hsiChanged || (old_hsiScalers[scaleIndex] != hsiScalers[scaleIndex]);
			old_hsiScalers[scaleIndex] = hsiScalers[scaleIndex];
		}
		if(rgbChanged) {
			convertScalerRGB_HSI(hsiScalers, rgbScalers);
			for(scaleIndex = 0; scaleIndex < 3; scaleIndex++) {
				old_hsiScalers[scaleIndex] = hsiScalers[scaleIndex];
			}
		} else if(hsiChanged) {
			convertScalerHSI_RGB(rgbScalers, hsiScalers);
			for(scaleIndex = 0; scaleIndex < 3; scaleIndex++) {
				old_rgbScalers[scaleIndex] = rgbScalers[scaleIndex];
			}
		}
		
		nk_end(ctx);
	}
	
	postNuklearWindowUpdate(win, ctx);
}

struct ProcessData {
	VideoWriteStruct* vws;
	GLFWwindow* win;
	struct nk_context* ctx;
};

bool processFrame(AVFrame *frame, int frameNumber, void* arg) {
	
	struct ProcessData* pd = (struct ProcessData*)arg;
	VideoWriteStruct* vws = pd->vws;
	GLFWwindow* win = pd->win;
	struct nk_context* ctx = pd->ctx;
	printf("Frame #%u\n", frameNumber);
	
	int width = frame->width;
	int height = frame->height;
	
	//if(frameNumber < 300 && frameNumber > 9) {
		
		
		Image* frameImage = createImageFromBytes(height, width, 3, frame->data[0]);
		frameImage->channelModel = CHANNEL_MODEL_RGB;
		
		setNuklearImage(nkOrigFrame, height, width, 3, frame->data[0]);
		
		//Pass Through
		if(processSelection == VID_PROC_PASS) {
			
		}
		//Invert
		if(processSelection == VID_PROC_INV) {
			invertImage(frameImage);
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
			
			float kernelMatrix[kernelRows][kernelCols];
			size_t i,j;
			for(i = 0; i < kernelRows; i++) {
				for(j = 0; j < kernelCols; j++) {
					kernelMatrix[i][j] = kernelData[j + i * kernelCols];
					//printf("%f, ", kernelMatrix[i][j]);
				}
				//printf("\n");
			}
			applyImageKernel(kernelRows, kernelCols, kernelMatrix, frameImage);
			if(useAbsoluteValue) {
				imageAbsoluteValue(frameImage);
			}
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
			if(gradientRangeOption == RANGE_TRUNCATE) {
				truncateImageRange(0, 255, frameImage);
			} else {
				scaleRangeImageUniform(0, 255, gradIm);
			}
			getBytesFromImage(frame->data[0], gradIm);
			freeImage(gradIm);
		}
		//RGB HSI Scale
		if(processSelection == VID_PROC_RGB) {
			
			
			scaleImageChannel(rgbScalers[CHANNEL_RED], CHANNEL_RED, frameImage);
			scaleImageChannel(rgbScalers[CHANNEL_GREEN], CHANNEL_GREEN, frameImage);
			scaleImageChannel(rgbScalers[CHANNEL_BLUE], CHANNEL_BLUE, frameImage);
			//Image* hsiImage = convertImageRGB_HSI(frameImage);
			//Image* rgbImage = convertImageHSI_RGB(hsiImage);
			getBytesFromImage(frame->data[0], frameImage);
			//freeImage(hsiImage);
			//freeImage(rgbImage);
		}
		
		freeImage(frameImage);
		
		setNuklearImage(nkProcFrame, height, width, 3, frame->data[0]);
	//}
	
	updateNuklearWindow(win, ctx);
	
	writeVideoFrame(vws, frame, width, height, frameNumber);
	
	return !glfwWindowShouldClose(win) && !endProcessing;
}

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
	/* Platform */
	GLFWwindow *win;
	struct nk_context *ctx;

	initNuklearWindow("Video Processing", &win, &ctx);
	
	nkOrigFrame = createNuklearImage();
	nkProcFrame = createNuklearImage();
	uint8_t* tempFill = calloc(640 * 360 * 3, sizeof(uint8_t));
	setNuklearImage(nkOrigFrame, 360, 640, 3, tempFill);
	setNuklearImage(nkProcFrame, 360, 640, 3, tempFill);
	free(tempFill);
	
	clock_gettime(CLOCK_MONOTONIC, &lastTime);
	long frameCount = 0;
	while (!glfwWindowShouldClose(win))
	{
		
		if(!startProcessing) {
			updateNuklearWindow(win, ctx);
		} else {
			startProcessing = false;
			VideoWriteStruct* vws = NULL;
			VideoReadStruct* vrs = NULL;
			inFileName[inFileNameLen] = 0;
			outFileName[outFileNameLen] = 0;
			if(!openVideoRead(&vrs, inFileName)) {
				continue;
			}
			if(!openVideoWrite(&vws, outFileName, 
					vrs->codecContext->bit_rate, vrs->width, vrs->height, 24)) {
						continue;
			}
			
			struct ProcessData pd;
			pd.vws = vws; pd.win = win; pd.ctx = ctx;
			processVideo(vrs, &processFrame, &pd);
			closeVideoRead(&vrs);
			
			closeVideoWrite(&vws);
			
			processing = false;
			endProcessing = false;
		}
		frameCount++;
	}
	
	destroyNuklearWindow(win, ctx);
	return 0;
	
}