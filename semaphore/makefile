
all: makedir bin/reader bin/writer clean

makedir:
#	mkdir bin

bin/reader:	reader.o common.o
	gcc -Wall reader.o common.o -o bin/reader

bin/writer: src/writer.o src/common.o
	gcc -Wall src/writer.o common.o -o bin/writer

reader.o: src/reader.c
	gcc -Wall -c src/reader.c

writer.o: src/writer.c
	gcc -Wall -c src/writer.c

common.o: src/common.c
	gcc -Wall -c src/common.c

clean:
	rm -rf *.o 