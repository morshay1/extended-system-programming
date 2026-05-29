all: lab5
lab5:
	gcc -m32 -c loader.c -o loader.o
	nasm -f elf32 -o start.o start.s
	ld -o loader startup.o loader.o start.o -L/usr/lib32 -lc -T linking_script -dynamic-linker /lib32/ld-linux.so.2

clean:
	rm -f loader start.o loader.o