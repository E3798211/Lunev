
all: bin/a.out clean

bin/a.out: main.o reader.o writer.o common.o
	gcc -Wall main.o reader.o writer.o common.o -o bin/a.out

main.o: src/main.c
	gcc -Wall -c src/main.c

reader.o: src/reader.c
	gcc -Wall -c src/reader.c

writer.o: src/writer.c
	gcc -Wall -c src/writer.c

common.o: src/common.c
	gcc -Wall -c src/common.c

clean:
	rm -rf *.o


