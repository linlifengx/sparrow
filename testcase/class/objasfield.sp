
double x = 3.14;
double y = 0.618;

A a = new A();

class A{
	B z = new B(x,y);
	long w = getLong();
}

class B{
	double z;
	B(double x,double y){
		z = x * y;
	}
}

long getLong(){
	return 100;
}

long main(char[][] args){
	printD(a.z.z);println(); // 1.940520
	printL(a.w);println(); // 100
}