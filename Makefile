CC=gcc
# CFLAGS=-Wall

all: interpreter analyzer

interpreter: interpreter.o parser.o lexer.o
	$(CC) $(CFLAGS) -o interpreter interpreter.o parser.o lexer.o

analyzer : analyzer.o parser.o lexer.o
	$(CC) $(CFLAGS) -o analyzer analyzer.o parser.o lexer.o

#run: analyzer
#	./analyzer

run: interpreter
	./interpreter code.l

clean:
	rm -rf *.o interpreter analyzer
