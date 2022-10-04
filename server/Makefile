#This Makefile is used to compile "aesdsocket.c".
#If "make CROSS_COMPILE=aarch64-none-linux-gnu-" is used, the program will be cross-compiled for the arm64-bit architecture

INCLUDES = ./aesdsocket.c
OUTPUT = ./aesdsocket

CROSS_COMPILE=
CC=$(CROSS_COMPILE)gcc

all: $(INCLUDES)
	$(CC) -Werror -Wall -lrt -lpthread -g $(INCLUDES) -o $(OUTPUT)

clean:
	rm -f $(OUTPUT)
	rm -f *.o