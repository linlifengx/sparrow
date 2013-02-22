
double x = 3.14;
double y = 0.618;

class B:A{
	B(){
		super(x,y);
	}
	
	double,double sum(){
		return x + a, y + b;
	}
}

class A{
	long a;
	long b;
	
	A(long a, long b){
		this.a = a;
		this.b = b;
	}
	
	[long a,long b] fields(){
		a = this.a;
		b = this.b;
		return;
	}
}

void main(){
	B a = new B();
	long x,y;
	[x,y] = a.fields();
	printL(x);println(); // 3
	printL(y);println(); // 0
	
	double i,j;
	[i,j] = a.sum();
	printD(i);println(); // 6.14
	printD(j);println(); // 0.618
}