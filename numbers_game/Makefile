CC := gcc
CFLAGS := -Wall -std=gnu11
DEPS := rand.h game.h
OBJS := main.o rand.o game.o

PROJ_NAME := numguess

all: build

.PHONY: build clean ds

build: $(PROJ_NAME)

$(PROJ_NAME): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^
	
%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c $<
	
clean:
	rm -f $(OBJS)
	
ds:
	rm -f data.sav
	
