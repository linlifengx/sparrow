
class A{
	long a;
	long b;
	long c;
	long d;
	long e;
}

long main(char[][] args){
	printL(GetHeapSize());println();
	for(long i = 0; i < 102400; i++){
		A a = new A();
	}
	printL(GetHeapSize());println();
	GC();
	printL(GetHeapSize());println();
}
