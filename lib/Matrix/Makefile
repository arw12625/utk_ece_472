SOURCE = ./src/
INC = ./include/
BIN = ./bin/

CC = gcc
CPPFLAGS = -I$(SOURCE) -I$(INC)

LIB_NAME = Matrix

BASE_MATRIX = BaseMatrix
BASE_MATRIX_C = $(SOURCE)$(BASE_MATRIX).c
BASE_MATRIX_H = $(INC)$(BASE_MATRIX).h

OBJECTS := $(patsubst $(SOURCE)%.c, $(BIN)%.o, $(filter-out $(BASE_MATRIX_C), $(wildcard $(SOURCE)*.c)))

.PHONY : clean

$(BIN)$(LIB_NAME).a : $(OBJECTS)
	ar rcs $@ $^

$(BIN)%.o : $(SOURCE)%.c $(INC)%.h $(BASE_MATRIX_C) $(BASE_MATRIX_H)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

clean:
	cd $(BIN) && del *.o && del *.a