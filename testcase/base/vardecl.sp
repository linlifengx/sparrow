long b,b1=-0,b2=-100;
double pi=3.14;

long main(char[][] args){
	printL(b);println(); //0
	printL(b1);println(); //0
	printL(b2);println(); //-100
	printL(pi);println(); //3
	printD(pi);println(); //3.14
	printD(b2);println(); //-100.0000
	b = 0.4*3;
	printL(b);println(); //1

	long _a,a=1000,a1_a_12=a;
	printL(_a);println(); //0
	printL(a);println(); //1000
	printL(a1_a_12);println(); //1000
	
	_a = 1;
	a=234,a1_a_12=_a;
	printL(_a);println(); //1
	printL(a);println(); //234
	printL(a1_a_12);println(); //1


	double x=1,y=2;
	printD(x);println(); //1.0000
	printD(y);println(); //2.0000

	x=x*y;
	y=x*y;
	printD(x);println(); //2.0000
	printD(y);println(); //4.0000
}
