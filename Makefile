CC = gcc
CFLAGS = -Wall -g
AR = ar
ARFLAGS = rcs

LIB_DIR = lib
BIN_DIR = bin
SRC_DIR = src
UOCPLAY_DIR = UOCPlay

LIB_NAME = $(LIB_DIR)/libUOCPlay.a
TARGET = $(BIN_DIR)/UOC20242d

UOCPLAY_SRC = $(UOCPLAY_DIR)/src/api.c $(UOCPLAY_DIR)/src/film.c \
               $(UOCPLAY_DIR)/src/csv.c $(UOCPLAY_DIR)/src/date.c \
               $(UOCPLAY_DIR)/src/person.c $(UOCPLAY_DIR)/src/subscription.c

MAIN_SRC = $(SRC_DIR)/main.c test/src/test_suite.c test/src/test_pr1.c test/src/test.c

INCLUDES = -I./test/include -I./UOCPlay/include

all: $(TARGET)

# Rule to build the main executable
$(TARGET): $(MAIN_SRC) $(LIB_NAME)
	mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) $(MAIN_SRC) -o $(TARGET) -L$(LIB_DIR) -lUOCPlay

# Rule to build the static library
$(LIB_NAME): $(UOCPLAY_SRC)
	mkdir -p $(LIB_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -c $(UOCPLAY_SRC)
	$(AR) $(ARFLAGS) $(LIB_NAME) *.o
	rm -f *.o

clean:
	rm -rf $(BIN_DIR) $(LIB_DIR)/*.a *.o