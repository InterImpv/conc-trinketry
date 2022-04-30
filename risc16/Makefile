CC := gcc
CFLAGS := -Wall -std=gnu2x
DEPS := instructions.h modules.h
OBJS := main.o instructions.o modules.o

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


