SOURCE = ./src/
INC = ./include/
BIN = ./bin/

CC = gcc
CPPFLAGS = -I$(SOURCE) -I$(INC)

LIB_NAME = Matrix
OBJECTS := $(patsubst $(SOURCE)%.c, $(BIN)%.o, $(wildcard $(SOURCE)*.c))

.PHONY : clean

$(BIN)$(LIB_NAME).a : $(OBJECTS)
	ar rcs $@ $^

$(BIN)%.o : $(SOURCE)%.c $(INC)%.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

clean:
	cd $(BIN) && del *.o && del *.a