SOURCE = ./src/
BIN = ./bin/
INC = ./include/
ARC_NAME = image

LIB_INC = $(INC) ../../../ffmpeg-dev/include/

CC = gcc
CPPFLAGS = -I$(SOURCE) $(addprefix -I, $(LIB_INC)) -std=c99 -pedantic -O2

OBJECTS = $(patsubst %, $(BIN)%.o, image image_processing ffmpeg_util)

.PHONY = All clean

ALL : $(BIN)$(ARC_NAME).a

$(BIN)$(ARC_NAME).a : $(OBJECTS)
	ar rcs $@ $^

$(BIN)%.o: $(SOURCE)%.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

$(BIN)ffmpeg_util.o : $(INC)ffmpeg_util.h $(SOURCE)ffmpeg_util.c

$(BIN)image.o $(BIN)image_processing.o: $(INC)image.h $(SOURCE)image.c

$(BIN)image_processing.o : $(INC)image_processing.h $(SOURCE)image_processing.c

clean:
	cd $(BIN) & del *.o & del *.a