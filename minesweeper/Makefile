CC := gcc
CFLAGS := -Wall -std=gnu2x -lncurses

DEPS := game.h
OBJS := main.o game.o

PROJ_NAME := minesweep

all: build

.PHONY: build clean

build: $(PROJ_NAME)

$(PROJ_NAME): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^
	
%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c $<
	
clean:
	rm -f $(OBJS)
	rm -f $(PROJ_NAME)

