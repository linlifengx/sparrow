#Sparrow
Sparrow is a C-style toy language and implemented with [llvm](http://llvm.rog).
It also is a object-oriented language similar to JAVA. It integrates Boehm GC. Program will be complied to a native executable file.

##Requirement
flex 2.5+  
bison 2.4+  
llvm 3.2  
os linux i386

##Building
    git clone git://github.com/linlifengx/sparrow.git
    cd sparrow
    make

##Usage
    ./sprc [options] file

    Options:
      -S            Compile to ir file
      -s            Compile to asm file
      -c            Compile to object file, but do not link
      -o <file>     Place the output into <file>

##Examples

####fibonacci.sp
	long main(char[][] args){
		printL(fib(10));println(); //55
		printL(fib(20));println(); //6765
		printL(fib2(10));println(); //55
		printL(fib2(20));println(); //6765
		printL(fib2(40));println(); //102334155
	}
	
	// recursion
	long fib(long n){
		if(n < 0)
			return 0;
		if(n == 0)
			return 0;
		else if(n == 1)
			return 1;
		else
			return fib(n-1)+fib(n-2);
	}
	
	// loop
	[long res] fib2(long n){
		long a0 = 0;
		long a1 = 1;
		res = 0;
		if(n <= 0){
			res = 0;
			return;
		}
		if(n == 1){
			res = 1;
			return;
		}
		for(long i = 2; i <= n; i++){
			res = a0 + a1;
			a0 = a1;
			a1 = res;
		}
	}

####linkedlist.sp
	class List {
		Node head;
		Node tail;
		
		void print(){
			for(Node node = head;node != null; ){
				printL(node.v);println();
				node = node.next;
			}
		}
		
		void add(Node node){
			if(head == null) {
				head = node;
				tail = node;
			} else {
				tail.next = node;
				tail = node;
			}
		}
	}
	
	class Node {
		long v;
		Node next;
		
		Node(long v){
			this.v = v;
		}
	}
	
	long main(char[][] args){
		List list = new List();
		for(long i = 0; i < 100; i++){
			list.add(new Node(i));
		}
		list.print(); // 0 - 99
	}