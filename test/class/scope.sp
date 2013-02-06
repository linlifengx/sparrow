
long a = 911;
long b;

class A : B{
	long a;

	A(){
		super(a); // global a 911
		printL(super.a);println(); // super a 911
		printL(this.a);println(); // this a 0
		a = 123;
		printL(this.a);println(); // this a 123
	}
}

class B{
	long a;
	long b;
	
	B(long a){
		printL(a);println(); // arg a 911
		this.a = a;
		printL(this.a);println(); // this a 911
		a = a + 1;
		printL(this.a);println(); // this a 911
		printL(a);println(); // arg a 912
		
		printL(b);println(); // this b 0
		long b = 888;
		printL(b);println(); // local b 888
	}
}

void main(){
	printL(a);println(); // global a 911
	A a = new A();
}