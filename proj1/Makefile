SOURCE = ./src/
BIN = ./bin/

LIBRARIES = Image Matrix

LIB = ../lib/
LIB_INC = $(patsubst %, $(LIB)%/include/, $(LIBRARIES))
LIB_BIN = $(patsubst %, $(LIB)%/bin/, $(LIBRARIES))

LDLIBS = $(join $(LIB_BIN), $(addsuffix .a, $(LIBRARIES)))
CC = gcc
CPPFLAGS = -I$(SOURCE) $(addprefix -I, $(LIB_INC))
OBJECTS := $(patsubst $(SOURCE)%.c, $(BIN)%.o, $(wildcard $(SOURCE)*.c))


PROG = proj1 proj1_test

.PHONY : ALL $(LIBRARIES)

ALL : $(patsubst %, $(BIN)%, $(PROG))

$(LIBRARIES) :
	$(MAKE) -C $(LIB)$@/
	
$(BIN)%.o: $(SOURCE)%.c $(LIBRARIES)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

clean:
	del $(BIN)*.o