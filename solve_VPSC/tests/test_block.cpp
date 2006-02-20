#include <variable.h>
#include <constraint.h>
#include <block.h>
#include <iostream>
using namespace std;

void test1() {
	cout << "Block test 1..." << endl;
	Variable *a1=new Variable(1,0,1);
	Variable *a2=new Variable(2,0,1);
	Constraint *c=new Constraint(a1,a2,1);
	Block *b1=new Block(a1);
	Block *b2=new Block(a2);
	b1->merge(b2,c);
	cout << "Block: " << *b1 << endl;
	a1->desiredPosition = -1;
	a2->desiredPosition = 2;
	Constraint *m = b1->findMinLMBetween(a1,a2);
	cout << "Min lm constraint: " << *m << endl;
	assert(c==m);
	cout << "  lm=" << c->lm << endl;
	cout << "Block test 1... Success!" << endl;
}
int main() {
	test1();
	return 0;
}
