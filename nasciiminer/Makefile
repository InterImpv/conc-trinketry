CC := g++
CFLAGS := -Wall -std=c++2a -lncurses -pthread
DEPS := engine.h perlin.h
OBJS := main.o engine.o perlin.o

PROJ_NAME := main

all: build

.PHONY: build clean

build: $(PROJ_NAME)

$(PROJ_NAME): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^
	
%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c $<
	
clean:
	rm -f $(OBJS)	
