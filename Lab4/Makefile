# Default target for building the program
all: task1

# Link object files
task1: task1.o 
	gcc -m32 -g -O0 -o task1 task1.o

# Compile task1.o
task1.o: task1.c
	gcc -m32 -g -O0 -c task1.c -o task1.o

# Clean up generated files
clean:
	rm -f task1 task1.o
