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

#define WINDOW_WIDTH 1200
#define WINDOW_HEIGHT 800

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

struct nk_image createNuklearImage(size_t rows, size_t cols, size_t numChannels, uint8_t* data) {
    
    GLuint tex;
    glGenTextures(1, &tex);
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
    return nk_image_id((int)tex);
}

int main(void)
{
	/* Platform */
	GLFWwindow *win;
	struct nk_context *ctx;

	initNuklearWindow("GUI TEST", WINDOW_WIDTH, WINDOW_HEIGHT, &win, &ctx);

	size_t imWidth = 640, imHeight = 360, numChannels = 3;
	uint8_t data[imWidth * imHeight * numChannels];
	size_t i,j;
	for(i = 0; i < imWidth * imHeight * numChannels; i+= numChannels) {
		data[i + 0] = 0;
		data[i + 1] = 150;
		data[i + 2] = 0;
	}
	for(i = 100; i < 150; i++) {
		for(j = 150; j < 200; j++) {
			data[numChannels * (i * imWidth + j)] = 200;
		}
	}
	
    glEnable(GL_TEXTURE_2D);
	struct nk_image image = createNuklearImage(imHeight, imWidth, numChannels, data);
	
	while (!glfwWindowShouldClose(win))
	{
		preNuklearWindowUpdate(win, ctx);
		
		/* GUI */
		if (nk_begin(ctx, "Image", nk_rect(0, 0, 700, 400),
			NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_SCALABLE|
			NK_WINDOW_MINIMIZABLE|NK_WINDOW_TITLE)) {
				
			struct nk_command_buffer *canvas;
			struct nk_color grid_color = nk_rgba(255, 255, 255, 255);
			struct nk_rect total_space = nk_window_get_content_region(ctx);
			canvas = nk_window_get_canvas(ctx);
			nk_draw_image(canvas, total_space, &image, grid_color);
		}
		nk_end(ctx);

		postNuklearWindowUpdate(win, ctx);
		
	}
	
	destroyNuklearWindow(win, ctx);
	return 0;
	
}