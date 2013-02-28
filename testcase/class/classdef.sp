
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

long main(char[][] args){
	A a = new A(77,88);
	long x,y;
	[x,y] = a.fields();
	printL(x);println(); // 77
	printL(y);println(); // 88
}