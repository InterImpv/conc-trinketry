CC := gcc
CFLAGS := -Wall -std=gnu2x -lncurses
DEPS := instructions.h modules.h
OBJS := main.o instructions.o modules.o

PROJ_NAME := main
ROM_NAME := hello
RAM_NAME := helloram

all: build

.PHONY: build clean asmc

build: $(PROJ_NAME)

$(PROJ_NAME): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c $<

asmc:
	assembler/assembler asm/$(ROM_NAME).asm asm/$(ROM_NAME).o
	assembler/assembler asm/$(RAM_NAME).asm asm/$(RAM_NAME).o

clean:
	rm -f $(OBJS)
	rm -f $(PROJ_NAME)


