long main(char[][] args){
	long x = 0;
	for(long i = 1; i < 100; i++){
		x = x + i;
		if(x > 600){
			printL(i);println(); //35
			break;
		}else
			continue;
	}
	printL(x);println(); //630
}
