
class A {
	long x;
	long y;
}

class B {
	double x;
}

void main(){
	A a;
	B b;
	printB(a == null);println(); // true
	printB(a == b); println(); // true
	
	a = new A();
	printB(a == null);println(); // false
	printB(a != b); println(); // true
	printL(a.x);println(); // 0
	
	b = new B();
	printB(a == b); println(); // false
}