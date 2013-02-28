void puts(char[] str){
	if(str == null){
		return;
	}
	for(long i = 0; i < str.length; i ++){
		printC(str[i]);
	}
	println();
}

long main(char[][] args){
	for(long i = 0; i < args.length; i + +){
		puts(args[i]);
	}
	
	return 77;
}