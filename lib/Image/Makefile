SOURCE = ./src/
INC = ./include/
BIN = ./bin/

LIBRARIES = Matrix

LIB = ../
LIB_INC = $(patsubst %, $(LIB)%/include/, $(LIBRARIES))
LIB_BIN = $(patsubst %, $(LIB)%/bin/, $(LIBRARIES))

LDLIBS = $(join $(LIB_BIN), $(addsuffix .a, $(LIBRARIES)))

CC = gcc
CPPFLAGS = -I$(SOURCE) -I$(INC) $(addprefix -I, $(LIB_INC))

LIB_NAME = Image

NO_HEADERS = BMPImageIO
NO_HEADERS_H = $(patsubst %, $(INC)%.h, $(NO_HEADERS))

OBJECTS := $(patsubst $(SOURCE)%.c, $(BIN)%.o, $(wildcard $(SOURCE)*.c))

.PHONY : All clean $(NO_HEADERS_H) $(LIBRARIES)

All : $(LIBRARIES) $(BIN)$(LIB_NAME).a

$(LIBRARIES) :
	$(MAKE) -C $(LIB)$@/

$(BIN)$(LIB_NAME).a : $(OBJECTS)
	ar rcs $@ $^

$(NO_HEADERS_H) :
	
$(BIN)%.o : $(SOURCE)%.c $(INC)%.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

	
clean:
	cd $(BIN) && del *.o && del *.a