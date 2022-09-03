#This Makefile is used to compile "writer.c".
#If "make CROSS_COMPILER=aarch64-none-linux-gnu-" is used, the program will be cross-compiled for the arm64-bit architecture

INCLUDES = ./writer.c
OUTPUT = ./writer

CROSS_COMPILE=

all: $(INCLUDES)
	$(CROSS_COMPILE)gcc $(INCLUDES) -o $(OUTPUT)

clean:
	rm -f $(OUTPUT)
	rm -f *.o