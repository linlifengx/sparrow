
class A {
}

class B : A {
	long x = 110;
	long y = 999;
}

class C : A {
	double i = 3.14;
	double j = 0.123;
	B x = new B();
}

void test(A a){
	if(a isa B){
		printL(((B)a).x + ((B)a).y);println(); // 1109
	}else if(a isa C){
		C c = (C) a;
		printD(c.i + c.j);println(); // 3.263
		printB(c.x.x isa long);println(); //true
	}
}

void main(){
	test(new B());
	test(new C());
	
	printB('a' isa char);println(); // true
	printB(true isa bool);println(); // true
}