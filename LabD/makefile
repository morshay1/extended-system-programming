all: main

main: multi.o
	gcc -m32 -g -Wall -o multi multi.o

multi.o: multi.s
	nasm -f elf32 multi.s -o multi.o

clean: 
	rm -f *.o multi
