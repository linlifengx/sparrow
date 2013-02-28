
long main(char[][] args){
	char x = 1000;
	long y = 7;
	printB((x%y) isa long);println(); // true
	printL(x % y);println(); // 6
	printL(y % x);println(); // 7
	printL(x % -y);println(); // 6
	printL(-x % y);println(); // -6
	printL(-x % -y);println(); // -6
	
	x = 123*3%5/2*4;
	printL(x);println(); // 8
}
