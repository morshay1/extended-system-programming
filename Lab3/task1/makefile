all: clean start

start:
	nasm -f elf32 start.s -o start.o
	gcc -m32 -c -o util.o util.c
	gcc -m32 -o start start.o util.o
	
clean:
	rm -f *.o task0 start task0