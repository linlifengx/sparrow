long main(char[][] args){
	long a,b=x();
	printL(a);println(); //0
	printL(b);println(); //123
	
	[a,b] = x();
	printL(a);println(); //123
	printL(b);println(); //234

	double c,d;
	[a,b,c,d] = y(6,3.14);
	printL(a);println(); //6
	printL(b);println(); //12
	printD(c);println(); //3.14
	printD(d);println(); //6.28

	[,a,,c] = y(1,-0.6);
	printL(a);println(); //2
	printL(b);println(); //12
	printD(c);println(); //-1.2
	printD(d);println(); //6.28
	
}

long,long x(){
	return 123,234;
}

[long a,long b,double c,double d] y(long x,double y){
	a = x;
	b = 2*x;
	c = y;
	d = 2*y;
}
