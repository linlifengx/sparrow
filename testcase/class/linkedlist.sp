
class List {
	Node head;
	Node tail;
	
	void print(){
		for(Node node = head;node != null; ){
			printL(node.v);println();
			node = node.next;
		}
	}
	
	void add(Node node){
		if(head == null) {
			head = node;
			tail = node;
		} else {
			tail.next = node;
			tail = node;
		}
	}
}

class Node {
	long v;
	Node next;
	
	Node(long v){
		this.v = v;
	}
}

long main(char[][] args){
	List list = new List();
	for(long i = 0; i < 100; i++){
		list.add(new Node(i));
	}
	list.print(); // 0 - 99
}