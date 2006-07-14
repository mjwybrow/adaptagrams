#include <variable.h>
#include <constraint.h>
#include <solve_VPSC.h>
#include <iostream>
#include <cassert>
#include <math.h>
using namespace std;
using namespace vpsc;
inline bool approxEquals(const double a, const double b) {
	return fabs((double)a-b)<0.0001;
}
void test1() {
	cout << "Test 1..." << endl;
	Variable *a[] = {
		new Variable(0,0,1), new Variable(1,1,1)};
	Constraint *c[] = {
		new Constraint(a[0],a[1],2), new Constraint(a[1],a[0],2)};
	double expected[]={1.5,-0.5};
	unsigned int n = sizeof(a)/sizeof(Variable*);
	unsigned int m = sizeof(c)/sizeof(Constraint*);
	try {
		IncSolver vpsc(n,a,m,c);
		vpsc.solve();
	} catch (char const *msg) {
		cerr << msg << endl;
		exit(1);
	}

	for(int i=0;i<n;i++) {
		assert(approxEquals(a[i]->position(),expected[i]));
	}
	cout << "Test 1... done." << endl;
}
void test2() {
	cout << "Test 2..." << endl;
	Variable *a[] = {
		new Variable(0,8,1),
		new Variable(1,5,1),
		new Variable(2,3,1),
		new Variable(3,1,1)
	};
	Constraint *c[] = {
		new Constraint(a[0],a[3],3),
		new Constraint(a[0],a[1],3),
		new Constraint(a[1],a[3],3),
		new Constraint(a[1],a[2],3),
		new Constraint(a[2],a[3],3),
		new Constraint(a[2],a[3],3)
	};
	//double expected[]={-3.71429,4,1,-0.714286,2.28571,2.28571,7,5.28571,8.28571,11.2857};
	unsigned int n = sizeof(a)/sizeof(Variable*);
	unsigned int m = sizeof(c)/sizeof(Constraint*);
	try {
		IncSolver vpsc(n,a,m,c);
		vpsc.solve();
	} catch (char const *msg) {
		cerr << msg << endl;
		exit(1);
	}

	/*
	for(int i=0;i<n;i++) {
		assert(approxEquals(a[i]->position(),expected[i]));
	}
	*/
	cout << "Test 2... done." << endl;
}
int main() {
	test1();
}
