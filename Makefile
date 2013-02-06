sources=src/main.cpp \
	temp/parser.cpp \
	temp/token.cpp \
	src/ast/expression/binarylogicexpr.cpp \
	src/ast/expression/binaryopexpr.cpp \
	src/ast/expression/funcinvoke.cpp \
	src/ast/expression/identexpr.cpp \
	src/ast/expression/simpleexpr.cpp \
	src/ast/node/program.cpp \
	src/ast/statement/vardef.cpp \
	src/ast/statement/classdef.cpp \
	src/ast/statement/constructor.cpp \
	src/ast/statement/forstmt.cpp \
	src/ast/statement/funcdef.cpp \
	src/ast/statement/ifstmt.cpp \
	src/ast/statement/returnstmt.cpp \
	src/ast/statement/simplestmt.cpp \
	src/ast/statement/superinit.cpp \
	src/ast/statement/varassi.cpp \
	src/ast/support/avalue.cpp \
	src/ast/support/classinfo.cpp \
	src/ast/support/context.cpp \
	src/ast/support/support.cpp \

headers=include/expression.h \
	include/node.h \
	include/statement.h \
	include/support.h \
	temp/parser.hpp

all:parser lex compile

clean:
	rm -rf temp sprc

compile_syslib:src/syslib/sysapi.c
	gcc -c src/syslib/sysapi.c -o lib/sysapi.o -Igc/include -std=c99

parser:src/parser/parser.y
	mkdir temp -p
	bison -d -o temp/parser.cpp $<

lex:src/parser/token.l
	mkdir temp -p
	flex -o temp/token.cpp $<

compile:$(sources) $(headers)
	g++ -o sprc $(sources) -Iinclude -Itemp `llvm-config --cxxflags \
	--ldflags --libs core native all-targets asmparser`
