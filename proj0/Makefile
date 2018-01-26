SOURCE = ./src/
BIN = ./bin/

LIBRARIES = Matrix

LIB = ../lib/
LIB_INC = $(LIB)$(LIBRARIES)/include/
LIB_BIN = $(LIB)$(LIBRARIES)/bin/

CC = gcc
CPPFLAGS = -I$(SOURCE) -I$(LIB_INC)
OBJECTS := $(patsubst $(SOURCE)%.c, $(BIN)%.o, $(wildcard $(SOURCE)*.c))
LDLIBS = $(patsubst %, $(LIB_BIN)%.a, $(LIBRARIES))

PROG = proj0

.PHONY : ALL $(LIBRARIES)

ALL : $(patsubst %, $(BIN)%, $(PROG)) $(LIBRARIES)

$(LIBRARIES) :
	$(MAKE) -C $(LIB)$@/
	
$(BIN)%.o: $(SOURCE)%.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

clean:
	del $(BIN)*.o