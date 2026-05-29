CFLAGS = -g -m32

lab1-3 : menu_map.o
	cc $(CFLAGS) -o lab1-3 menu_map.o

menu_map.o : menu_map.c
	cc $(CFLAGS) -c menu_map.c

functions.o : function.h
	cc $(CFLAGS) -c function.h

clean :
	rm -f menu_map.o functions.o lab1-3
