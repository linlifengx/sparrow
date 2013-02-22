
void printlnL(long a){
	printL(a);println();
}

void printlnD(double a){
	printD(a);println();
}

void printlnC(char a){
	printC(a);println();
}

void printlnB(bool a){
	printB(a);println();
}

class A {
	long a;
	long b;
	
	long add(){
		return a + b; 
	}
}

class B : A {
	long x;
	long y;
}

void main(){
	long a = (double)98;
	char b = (char) a;
	printlnL(a); // 98
	printlnC(b); // b
	printlnL(b); // 98
	printlnC(a); // b
	println();
	
	printlnB(a isa long); // true
	printlnB(a isa char); // false
	printlnB(a isa double); // false
	printlnB(b isa char); // true
	println();
	
	printlnB( (87 + 6) isa double); // false
	printlnB( a isa long && b isa char); // true
	printlnB( null isa long); // false
	printlnB( null isa A); // true
	println();
	
	B x;
	A y;
	printlnB(x isa A); // true
	printlnB(y isa B); // true
	
	x = new B();
	y = x;
	printlnB(x.x isa B); // false
	printlnB(x.add() isa long); // true
	printlnB( (double)x.add() isa long); // false
	println();

	x.a = 99;
	x.x = 100;
	printlnL(((B)y).x); // 100
	printlnL( 2 * (long)2.5 + 3 ); // 7
	printlnB( (B)new A() isa B); // true
	printlnB( (B)new A() == null); // true
	println();
	
	B[] z;
	B w;
	printlnB(z isa B || z isa A); // false
	printlnB(null isa B[]); // true
	printlnB(w isa B[]); // false
}