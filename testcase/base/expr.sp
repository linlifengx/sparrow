long main(char[][] args){
	long a = (30+12)*5+3*4+6/(5-7)+7/3;
	printL(a);println(); //221
	double b = 1/2;
	printD(b);println(); //0
	b = 1.0/2;
	printD(b);println(); //0.5
	
	bool x = 1 > 2;
	printB(x);println(); //false;
	x = b > 0 && a > 0;
	printB(x);println(); //true;
}
