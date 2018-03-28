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

#define WINDOW_WIDTH 1600
#define WINDOW_HEIGHT 700

#define MAX_VERTEX_BUFFER 512 * 1024
#define MAX_ELEMENT_BUFFER 128 * 1024

static void error_callback(int e, const char *d) {
	printf("Error %d: %s\n", e, d);
}

void initNuklearWindow(const char* title, int desiredWidth, int desiredHeight, GLFWwindow** winPointer, struct nk_context** ctxPointer) {
	
	int width = 0, height = 0;
	/* GLFW */
	glfwSetErrorCallback(error_callback);
	if (!glfwInit()) {
		fprintf(stdout, "[GFLW] failed to init!\n");
		exit(1);
	}
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	*winPointer = glfwCreateWindow(desiredWidth, desiredHeight, title, NULL, NULL);
	glfwMakeContextCurrent(*winPointer);
	glfwGetWindowSize(*winPointer, &width, &height);
	
	/* OpenGL */
	glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
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
	int width = 0, height = 0;
	/* Draw */
	glfwGetWindowSize(win, &width, &height);
	glViewport(0, 0, width, height);
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

const char* videoProcesses[] = {"Pass Through", "Exponential", "Kernel", "Gradient"};
int processesLen = 4;
int processSelection = 0;

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

bool setKernel(char* kernelStr, float* kernel, int kerRows, int kerCols, int maxKerSize);

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
	}
	nk_end(ctx);
	
	//Processed Video Window
	if (nk_begin(ctx, "Processed Video", nk_rect(720, 0, 700, 400),
		NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_SCALABLE|
		NK_WINDOW_MINIMIZABLE|NK_WINDOW_TITLE)) {
			
		struct nk_command_buffer *canvas;
		struct nk_color grid_color = nk_rgba(255, 255, 255, 255);
		struct nk_rect total_space = nk_window_get_content_region(ctx);
		canvas = nk_window_get_canvas(ctx);
		nk_draw_image(canvas, total_space, &nkProcFrame, grid_color);
	}
	nk_end(ctx);

	//Video Options
	if (nk_begin(ctx, "Video Options", nk_rect(0, 420, 700, 185),
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
		
	}
	nk_end(ctx);
	
	//Frame Rate Display
	delayFramerateDisp += delta_us;
	if(delayFramerateDisp > 200000) {
		sprintf(framerateDispBuffer, "%f", currentFramerate);
		delayFramerateDisp = 0;
	}
	if (nk_begin(ctx, "Framerate", nk_rect(WINDOW_WIDTH - 64, WINDOW_HEIGHT - 64, 64, 64), NK_WINDOW_NO_INPUT)) {
		nk_layout_row_dynamic(ctx, 30, 1);
		nk_label(ctx, framerateDispBuffer, NK_TEXT_LEFT);
	}
	nk_end(ctx);
	
	//Kernel Options
	if (nk_begin(ctx, "Kernel Options", nk_rect(720, 420, 700, 185),
		NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_SCALABLE|
		NK_WINDOW_MINIMIZABLE|NK_WINDOW_TITLE)) {
		
		nk_layout_row_dynamic(ctx, 30, 2);
		nk_label(ctx, "Kernel:", NK_TEXT_LEFT);
		nk_edit_string(ctx, NK_EDIT_SIMPLE, kernelStr, &kernelStrLen, 64, nk_filter_default);
		
		nk_layout_row_dynamic(ctx, 30, 2);
		nk_property_int(ctx, "Kernel Rows", 1, &enteredKernelRows, maxKernelSize, 1, 1);
		nk_property_int(ctx, "Kernel Cols", 1, &enteredKernelCols, maxKernelSize, 1, 1);
		
		nk_layout_row_dynamic(ctx, 30, 1);
		nk_checkbox_label(ctx, "Absolute Value", &enteredUseAbsoluteValue);
		
		nk_layout_row_dynamic(ctx, 30, 1);
		if (nk_button_label(ctx, "Set Kernel")) {
			// event handling
			if(setKernel(kernelStr, kernelData, enteredKernelRows, enteredKernelCols, maxKernelSize)) {
				kernelRows = enteredKernelRows;
				kernelCols = enteredKernelCols;
				useAbsoluteValue = enteredUseAbsoluteValue;
			}
		}
		
	}
	nk_end(ctx);
	
	
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
		
		setNuklearImage(nkOrigFrame, height, width, 3, frame->data[0]);
		
		//Exponential
		if(processSelection == 1) {
			exponentiateImage(frameImage);
			getBytesFromImage(frame->data[0], frameImage);
		}
		//Kernel
		if(processSelection == 2) {
			
			float kernelMatrix[kernelRows][kernelCols];
			size_t i,j;
			for(i = 0; i < kernelRows; i++) {
				for(j = 0; j < kernelCols; j++) {
					kernelMatrix[i][j] = kernelData[j + i * kernelCols];
					//printf("%f, ", kernelMatrix[i][j]);
				}
				//printf("\n");
			}
			applyImageChannelKernel(kernelRows, kernelCols, kernelMatrix, 0, frameImage);
			applyImageChannelKernel(kernelRows, kernelCols, kernelMatrix, 1, frameImage);
			applyImageChannelKernel(kernelRows, kernelCols, kernelMatrix, 2, frameImage);
			if(useAbsoluteValue) {
				imageAbsoluteValue(frameImage);
			}
			scaleRangeImageUniform(0, 255, frameImage);
			
			getBytesFromImage(frame->data[0], frameImage);
		}
		//Gradient
		if(processSelection == 3) {
			
			Image* gradIm = computeGradientImage(frameImage);
			scaleRangeImageUniform(0, 255, gradIm);
			getBytesFromImage(frame->data[0], gradIm);
			freeImage(gradIm);
		}			
		
		freeImage(frameImage);
		
		setNuklearImage(nkProcFrame, height, width, 3, frame->data[0]);
	//}
	
	updateNuklearWindow(win, ctx);
	
	writeVideoFrame(vws, frame, width, height, frameNumber);
	
	return !glfwWindowShouldClose(win) && !endProcessing;
}

bool setKernel(char* kernelStr, float* kernel, int kerRows, int kerCols, int maxKerSize) {
	
	char kernelStrBuf[256];
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

	initNuklearWindow("Video Processing", WINDOW_WIDTH, WINDOW_HEIGHT, &win, &ctx);
	
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