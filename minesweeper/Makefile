CC := g++
CFLAGS := -Wall -std=c++2a -lncurses

PROJ_DEPS := game.h
PROJ_OBJS := main.o game.o

PROJ_NAME := minesweep

all: main

.PHONY: main clean

main: $(PROJ_NAME)

$(PROJ_NAME): $(PROJ_OBJS)
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f *.o
	rm -f $(PROJ_NAME)


