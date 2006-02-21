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

/*
 * Constraint tree:
 *    \_/
 *    / \
 */
void test2() {
	cout << "Block test 2..." << endl;
	Variable *a[]={
		new Variable(0,0,1),
		new Variable(1,0,1),
		new Variable(2,1,1),
		new Variable(3,2,1),
		new Variable(4,3,1),
		new Variable(5,3,1)};
	Constraint *c[]={
		new Constraint(a[0],a[2],2),
		new Constraint(a[1],a[2],2),
		new Constraint(a[2],a[3],2),
		new Constraint(a[3],a[4],2),
		new Constraint(a[3],a[5],2)};
	for(int i=0;i<6;i++) {
		new Block(a[i]);
	}
	for(int i=0;i<5;i++) {
		Block *l=c[i]->left->block, *r=c[i]->right->block;
		l->merge(r,c[i]);
	}
	Block *b=a[0]->block;
	cout << "Block: " << *b << endl;
	for(int i=0;i<6;i++) {
			a[i]->desiredPosition = i!=4?-2:5;
	}
	cout << "calc min lm:" << endl;
	Constraint *m = b->findMinLMBetween(a[0],a[4]);
	cout << "Min lm constraint: " << *m << endl;
	assert(m==c[3]);
	cout << "Block test 2... Success!" << endl;
}
int main() {
	test1();
	test2();
	return 0;
}
