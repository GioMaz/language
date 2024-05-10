CC=gcc
CFLAGS=
LLVM_CFLAGS= $$(llvm-config --cflags --ldflags --libs core)

all: interpreter codegen

interpreter: interpreter.o parser.o lexer.o
	$(CC) $(CFLAGS) -o interpreter interpreter.o parser.o lexer.o

codegen: codegen.o analyzer.o parser.o lexer.o
	$(CC) $(LLVM_CFLAGS) -o codegen codegen.o analyzer.o parser.o lexer.o

#run: interpreter
#	./interpreter code.l

run: codegen
	./codegen code.l
	#./codegen code.l | llvm-as | opt -passes=mem2reg | llvm-dis
	./codegen code.l | lli ; echo $$?

clean:
	rm -rf *.o interpreter codegen
