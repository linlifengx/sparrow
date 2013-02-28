
long main(char[][] args){
	A a = new A();
	printL(a.x);println(); // 1
	printL(a.y);println(); // 2
	setField(a);
	printL(a.x);println(); // 1000
	printL(a.y);println(); // 2000
}

class A{
	long x = 1;
	long y = 2;
}

void setField(A a){
	a.x = 1000;
	a.y = 2000;
}