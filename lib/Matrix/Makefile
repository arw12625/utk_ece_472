SOURCE = ./src/
INC = ./include/
BIN = ./bin/

CC = gcc
CPPFLAGS = -I$(SOURCE) -I$(INC)

LIB_NAME = Matrix

BASE_MATRIX = BaseMatrix
BASE_MATRIX_C = $(SOURCE)$(BASE_MATRIX).c
BASE_MATRIX_H = $(INC)$(BASE_MATRIX).h

MATRIX_TYPES = Matrix_d Matrix_u8 Matrix_u16 Matrix_i16
MATRIX_OBJECTS := $(patsubst %, $(BIN)%.o, $(MATRIX_TYPES))

OTHER = MatrixUtil
OTHER_OBJECTS = $(patsubst %, $(BIN)%.o, $(OTHER))

.PHONY : clean

$(BIN)$(LIB_NAME).a : $(OTHER_OBJECTS) $(MATRIX_OBJECTS)
	ar rcs $@ $^

$(OTHER_OBJECTS) : $(BIN)%.o : $(SOURCE)%.c $(INC)%.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

$(MATRIX_OBJECTS) : $(BIN)%.o : $(SOURCE)%.c $(INC)%.h $(BASE_MATRIX_C) $(BASE_MATRIX_H)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<
	
clean:
	cd $(BIN) && del *.o && del *.a