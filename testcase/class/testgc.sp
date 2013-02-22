
class A{
	long a;
	long b;
	long c;
	long d;
	long e;
}

void main(){
	for(long i = 0; i < 102400; i = i+ 1){
		A a = new A();
	}
	printL(GetHeapSize());println();
}
