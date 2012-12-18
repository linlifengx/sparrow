#Sparrow
Sparrow is a C-style toy language and implemented with [llvm](http://llvm.rog).

##Requirement
flex 2.5+  
bison 2.4+  
llvm 3.2+

##Compiling
	make all

##Run
	./sprc <programfile> [options]
'sprc' will run \<programfile> just in time with no option.

	options:
	-S           Compile to a LLVM IR file
	-s           Compile to a assembly file
	-c           Compile to a object file
	-o <file>    Place the output into <file>  
