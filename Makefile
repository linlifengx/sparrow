sources=src/main.cpp src/AST/ast.cpp src/AST/stmt.cpp src/AST/globalstmt.cpp \
		src/AST/expr.cpp temp/parser.cpp temp/token.cpp
headers=include/ast.hpp temp/parser.hpp

all:parser lex bin

clean:
	rm -rf temp sprc

parser:src/parser/parser.y
	mkdir temp -p
	bison -d -o temp/parser.cpp $<

lex:src/parser/token.l
	mkdir temp -p
	flex -o temp/token.cpp $<

bin:$(sources) $(headers)
	g++ -o sprc $(sources) -Iinclude -Itemp `llvm-config --cxxflags --ldflags --libs core jit native all-targets asmparser`
