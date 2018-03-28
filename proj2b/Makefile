SOURCE = ./src/
BIN = ./bin/

LIB_INC = ../../ffmpeg-dev/include/ ../../glfw/include/ ../../glew-2.1.0/include ../../nuklear/
LDLIBS = -lavformat -lavcodec -lswscale -lavutil -lglfw3 -lopengl32 -lm  -lGLU32 -lGLEW32

LDFLAGS = -L../../ffmpeg-dev/lib/ -L../../glfw/build/src/ -L../../glew-2.1.0/
CC = gcc
CPPFLAGS = -I$(SOURCE) $(addprefix -I, $(LIB_INC)) -std=c99 -pedantic -O2

.PHONY = proj2b gui_test clean vid_test

proj2b : $(BIN)proj2b

gui_test : $(BIN)gui_test

vid_test : $(BIN)video_gui

$(BIN)proj2b : $(BIN)proj2b.o $(BIN)ffmpeg_util.o $(BIN)image.o $(BIN)image_processing.o

$(BIN)gui_test : $(BIN)gui_test.o

$(BIN)video_gui : $(BIN)video_gui.o $(BIN)ffmpeg_util.o $(BIN)image.o $(BIN)image_processing.o

$(BIN)%.o: $(SOURCE)%.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

$(BIN)proj2b.o $(BIN)video_gui.o $(BIN)ffmpeg_util.o : $(SOURCE)ffmpeg_util.c $(SOURCE)ffmpeg_util.h

$(BIN)proj2b.o $(BIN)video_gui.o $(BIN)image.o : $(SOURCE)image.c $(SOURCE)image.h

$(BIN)proj2b.o $(BIN)video_gui.o $(BIN)image_processing.o : $(SOURCE)image_processing.c $(SOURCE)image_processing.h

clean:
	cd $(BIN) & del *.o