
long main(char[][] args){
	long[][] a = { {1,2,3},
				   {3,2,1},
				   {9,8} };
	printMatix(a);println(); // 1 2 3  3 2 1  9 8
	printL(a[1][1]);println();println(); // 2
	a[1][1] = 100;
	printMatix(a);println(); // 1 2 3  3 100 1  9 8
}

void printMatix(long[][] a){
	if(a != null){
		for(long i = 0; i < a.length; i++){
			printArray(a[i]);
		}
	}
}

void printArray(long[] a){
	if(a != null){
		for(long i  = 0; i < a.length; i++){
			printL(a[i]);printC(' ');
		}
	}
	println();
}