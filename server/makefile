
all: bin/main clean

bin/main: main.o common.o
	gcc -Wall main.o common.o -o bin/main

main.o: src/main.c
	gcc -c src/main.c -o main.o

common.o: src/common.c
	gcc -c src/common.c -o common.o

clean:
	rm -f *.o
