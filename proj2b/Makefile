SOURCE = ./src/
BIN = ./bin/

LIB_INC = ../lib/image/include/ ../../ffmpeg-dev/include/ ../../glfw/include/ ../../glew-2.1.0/include ../../nuklear/
LDLIBS = ../lib/image/bin/image.a -lavformat -lavcodec -lswscale -lavutil -lglfw3 -lopengl32 -lm  -lGLU32 -lGLEW32

LDFLAGS = -L../../ffmpeg-dev/lib/ -L../../glfw/build/src/ -L../../glew-2.1.0/
CC = gcc
CPPFLAGS = -I$(SOURCE) $(addprefix -I, $(LIB_INC)) -std=c99 -pedantic -O2

.PHONY = proj2b gui_test clean vid_test image_lib

vid_test : image_lib $(BIN)video_gui

proj2b : image_lib $(BIN)proj2b

gui_test : image_lib $(BIN)gui_test

$(BIN)proj2b : $(BIN)proj2b.o

$(BIN)gui_test : $(BIN)gui_test.o

$(BIN)video_gui : $(BIN)video_gui.o

$(BIN)%.o: $(SOURCE)%.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

image_lib :
	$(MAKE) -C ../lib/image/

clean:
	cd $(BIN) & del *.o