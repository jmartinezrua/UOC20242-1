CC = gcc
CFLAGS = -Wall -g
AR = ar
ARFLAGS = rcs

SRC = src/api.c src/csv.c src/date.c src/film.c src/person.c src/subscription.c
INCLUDES = -I./include

LIB_DIR = ../lib
LIB_NAME = libUOCPlay.a

all: $(LIB_DIR)/$(LIB_NAME)

$(LIB_DIR)/$(LIB_NAME): $(SRC)
	mkdir -p $(LIB_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -c $(SRC)
	$(AR) $(ARFLAGS) $(LIB_DIR)/$(LIB_NAME) *.o
	rm -f *.o

clean:
	rm -f *.o
	rm -f $(LIB_DIR)/$(LIB_NAME)

.PHONY: all clean