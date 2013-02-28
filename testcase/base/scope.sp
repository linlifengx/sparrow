long a = 100;


long main(char[][] args){
	printL(a);println(); // 100
	long a = 200;
	printL(a);println(); // 200
	
	printL(func1(a));println(); // 300
}

long func1(long a){
	printL(a);println(); // 200
	long a = 300;
	printL(a);println(); // 300
	return a;
}