
class A {
	long i;
	long j;
	long x = -99;
	
	long sum(){
		return i + j + x;
	}
	
	void set(){
		i = 19;
		x = 29;
	}
}

class B : A{
	long i;
	long j;
	
	B(){
		i = 333;
		j = 444;
	}
	
	long sum(){
		return i + j;
	}
}

class C : A{
	
	C(){
		i = -123;
		j = -312;
	}
	
	long sum(){
		return i - j;
	}
}

void main(){
	A a = new B();
	printL(a.i);println(); // 333
	printL(a.j);println(); // 444
	printL(a.x);println(); // -99
	printL(a.sum());println(); // 777
	a.set();
	printL(a.i);println(); // 19
	printL(a.j);println(); // 444
	printL(a.x);println(); // 29
	
	a = new C();
	printL(a.i);println(); // -123
	printL(a.j);println(); // -312
	printL(a.x);println(); // -99
	printL(a.sum());println(); // 189
	a.set();
	printL(a.i);println(); // 19
	printL(a.j);println(); // -312
	printL(a.x);println(); // 29
}