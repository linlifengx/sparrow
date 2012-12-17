Sparrow is a C-like language. Its compiler 'sprc' is implemented with llvm(version 3.2svn).

Requirement  
	flex 2.5+
	bison 2.4+
	llvm 3.2+

Compiling  
	make all

Run  
	./sprc <programfile> [options] 
		
		It will run <programfile> just in time with no option.

		Options
		-S	Compile to a LLVM IR file
		-s	Compile to a assembly file
		-c	Compile to a object file
		-o <file>	place the output into <file>


Sparrow是一个类C语法语言，其编译器sprc是基于llvm3.2开发的。
