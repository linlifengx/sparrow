long main(char[][] args){
	printL(fib(10));println(); //55
	printL(fib(20));println(); //6765
	printL(fib2(10));println(); //55
	printL(fib2(20));println(); //6765
	printL(fib2(40));println(); //102334155
}

// recursion
long fib(long n){
	if(n < 0)
		return 0;
	if(n == 0)
		return 0;
	else if(n == 1)
		return 1;
	else
		return fib(n-1)+fib(n-2);
}

// loop
[long res] fib2(long n){
	long a0 = 0;
	long a1 = 1;
	res = 0;
	if(n <= 0){
		res = 0;
		return;
	}
	if(n == 1){
		res = 1;
		return;
	}
	for(long i = 2; i <= n; i = i+1){
		res = a0 + a1;
		a0 = a1;
		a1 = res;
	}
}
