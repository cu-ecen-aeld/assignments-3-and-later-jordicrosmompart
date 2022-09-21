#This Makefile is used to compile "writer.c".
#If "make CROSS_COMPILE=aarch64-none-linux-gnu-" is used, the program will be cross-compiled for the arm64-bit architecture

INCLUDES = ./writer.c
OUTPUT = ./writer

CROSS_COMPILE=
CC=$(CROSS_COMPILE)gcc

all: $(INCLUDES)
	$(CC) $(INCLUDES) -o $(OUTPUT)

clean:
	rm -f $(OUTPUT)
	rm -f *.o