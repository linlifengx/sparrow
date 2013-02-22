
void main(){
	char ln = '\n';
	char a = 'a';
	char b = 'b';
	char c = '\u66fe';
	printC(a);printC(ln); // a
	printC(b);printC(ln); // b
	printC(c);printC(ln); // ceng
	
	char x = a - b;
	printC(x);printC(ln); // -1 is nothing
	printL(x);println(); // -1
	
	x = a + b;
	printC(x);printC(ln); // Ã
	printL(x);println(); // 195

	x = 98;
	printC(x);println(); // b
	
	printB(a == 'a');println(); // true
	printB(a != 'a');println(); // false
	printB(a > 'a');println(); // false
	printB(a >= 'a');println(); // true
	printB(a < 'a');println(); // false
	printB(a <= 'a');println(); // true
	printB(a > b);println(); // false
	printB(a < b);println(); // true
}