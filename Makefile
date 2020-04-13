all: lib examples

core.o: lib/core.c
	gcc -c -static lib/core.c -o core.o -Ilib/include 

lib: core.o
	ar rcs webthing.a core.o

examples: single_thing

single_thing:
	gcc -c examples/single_thing.c -o single_thing.o -Ilib/include
	gcc single_thing.o webthing.a -o single_thing

clean:
	rm *.a *.o single_thing
