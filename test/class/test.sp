
class A : B {
	long x;
	long y;
	
	A(){
		super(321,123);
		printL(super.sum());println(); // 444
		printL(this.sum());println(); // 0
		printL(sum());println(); // 0
	}
}

class B{
	long x;
	long y;
	
	B(long x,long y){
		this.x = x;
		this.y = y;
	}
	
	long sum(){
		return x + y;
	}
}

void main(){
	A a = new A();
}