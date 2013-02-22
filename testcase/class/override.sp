
long a = 911;
long b = 100;

class A : B{
	long a;

	A(){
		super(xxx());
		printL(super.getA());println(); // 1011
		printL(getA());println(); // 11
		a = a + 123;
	}
	
	long getA(){
		return a + 11;
	}
}

class B{
	long a;
	B(long a){
		this.a = a;
	}
	
	long getA(){
		return a;
	}
	
	long getX(){
		return a;
	}
}

long xxx(){
	return a + b;
}

void main(){
	A a = new A();
	
	printL(a.getA());println(); // 134
	printL(a.getX());println(); // 123
}
