CC := gcc
CFLAGS := -Wall -std=gnu2x
LIBFLAGS := -lm -lncurses

DEPS := error.h game.h utils.h vector.h rand.h perlin.h
OBJS := main.o game.o utils.o vector.o rand.o perlin.o

PROJ_NAME := demo1

all: build

.PHONY: build clean

build: $(PROJ_NAME)

$(PROJ_NAME): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBFLAGS)
	
%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c $<
	
clean:
	rm -f $(OBJS)
	rm -f $(PROJ_NAME)

