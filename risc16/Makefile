CC := g++
CFLAGS := -Wall -std=c++2a -lncurses
DEPS := modules.h cmdi.h winpos.h gen-err.h
OBJS := main.o modules.o cmdi.o

PROJ_NAME := main
ROM_NAME := hello

all: build

.PHONY: build clean asmc

build: $(PROJ_NAME)

$(PROJ_NAME): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.cpp $(DEPS)
	$(CC) $(CFLAGS) -c $<

asmc:
	assembler/assembler asm/$(ROM_NAME).asm asm/$(ROM_NAME).o

clean:
	rm -f *.o
	rm -f asm/*.o
	rm -f $(PROJ_NAME)


