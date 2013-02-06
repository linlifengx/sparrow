void main(){
	bool x = a() && b(); //print 11111 22222
	printB(x);println(); //print true
	x = a() && !b(); //print 11111 22222
	printB(x);println(); //print false
	x = !a() && b(); //print 11111
	printB(x);println(); //print false
	x = !a() && !b(); //print 11111
	printB(x);println(); //print false

	x = a() || b(); //print 11111
	printB(x);println(); //print true
	x = a() || !b(); //print 11111
	printB(x);println(); //print true
	x = !a() || b(); //print 11111 22222
	printB(x);println(); //print true
	x = !a() || !b(); //print 11111 22222
	printB(x);println(); //print false
}

bool a(){
	printL(11111);
	println();
	return true;
}

bool b(){
	printL(22222);
	println();
	return true;
}
