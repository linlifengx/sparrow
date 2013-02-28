long main(char[][] args){
	long a = 1000;

	//print true
	if(a > 999){
		printB(true);println();
	}else{
		printB(false);println();
	}

	// print nothing
	if(a < 0)
		if(a < 2000)
			printB(true);
	else //match nearest 'if'
		printB(false);
}
