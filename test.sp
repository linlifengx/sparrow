

class A{
	long a = 1;
	long b = 2;
	long c = 3;
	long d = 4;
	long e = 5;
	long f = 6;
	long g = 7;
	long h = 8;
	long i = 9;
	long j = 10;
	long k = 11;
	long l = 12;
	long m = 13;
	long n = 14;
	long o = 15;
	long p = 16;
	long q = 17;

	long sum(){
		return a + b + c + d + e + f + g + h + i + j + k + l + m + n + o + p + q;
	}
}

void main(){
	return;
	for(long i = 0; i < 102400; i = i + 1){
		A a = new A();
	}
}