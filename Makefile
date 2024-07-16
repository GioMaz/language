CC=gcc
LLVMVERSION=18
LLVMCONFIG=llvm-config-$(LLVMVERSION)
LLI=lli-$(LLVMVERSION)
LLVMAS=llvm-as-$(LLVMVERSION)
LLVMDIS=llvm-dis-$(LLVMVERSION)
OPT=opt-$(LLVMVERSION)
CFLAGS=$$($(LLVMCONFIG) --cflags --ldflags --libs core)

.INTERMEDIATE: interpreter.o parser.o lexer.o codegen.o analyzer.o

all: interpreter codegen

interpreter: interpreter.o parser.o lexer.o
	$(CC) -o interpreter interpreter.o parser.o lexer.o

codegen: codegen.o analyzer.o parser.o lexer.o
	$(CC) -o codegen codegen.o analyzer.o parser.o lexer.o $(CFLAGS)

#run: interpreter
#	./interpreter code.l

run: codegen
	#./codegen code.l
	./codegen code.l | $(LLVMAS) | $(OPT) -passes=mem2reg | $(LLVMDIS)
	#./codegen code.l | $(LLI); echo $$?

clean:
	rm -rf *.o interpreter codegen
