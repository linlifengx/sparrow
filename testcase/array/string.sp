
void printlnS(char[] a){
	printS(a);
	println();
}

void printS(char[] a){
	for(long i = 0; i < a.length; i = i + 1){
		printC(a[i]);
	}
}

void main(){
	char[] a = "Hello world!";
	printS("a's length is ");printL(a.length);println(); // 12
	printlnS(a); // Hello world!
	
	a = "\u66fe\u7ecf\u6709\u4e00\u4efd\u771f\u631a\u7684\u7231\u60c5\u6446\u5728\u6211\u9762\u524d\uff0c";
	printS("a's length is ");printL(a.length);println(); // 16
	printlnS(a); // ceng jing ......
	
	char x = a[8];
	printC(x);println(); // ai
	
	char[] temp = a;
	a = {'I',' ','l','o','v','e',' ','y','o','u','!'};
	printlnS(a); // I love you!
	printB(a == temp);println(); // false
	
	temp = a;
	a = ['i',,'L'];
	printlnS(a); // i Love you!
	printB(a == temp);println(); // true
	
	char[] xxx;
	printB(xxx == null);println(); // true
	xxx = "hello";
	printlnS(xxx); // hello
	
	printL("\u6709\u4e00\u0000\u771F\u0000\u632a".length);println(); // 6
	printlnS("\u66fe\u7ecf\u6709\u4e00\u4EFD\u0000\u771f\u631a\u7684");
}