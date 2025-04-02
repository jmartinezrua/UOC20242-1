CC = gcc
CFLAGS = -Wall -g -O0 -DDEBUG_TRACE
AR = ar
ARFLAGS = rcs

SRCS = api.c src/film.c src/csv.c src/date.c src/person.c src/subscription.c OBJS = $(SRCS:.c=.o) LIB = libUOCPlay.a

all: $(LIB)

$(LIB): $(OBJS) @mkdir -p lib $(AR) rcs $(LIB) $(OBJS)

%.o: %.c $(CC) $(CFLAGS) -c $< -o $@

clean: rm -f $(OBJS) $(LIB)

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