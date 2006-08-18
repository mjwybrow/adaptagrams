#include <variable.h>
#include <constraint.h>
#include <solve_VPSC.h>
#include <algorithm>
#include <iostream>
#include <time.h>
#include <math.h>
#include <cassert>
using namespace std;
using namespace vpsc;

inline int getRand(const int range) {
	return (double)range*rand()/(RAND_MAX+1.0);
}
inline bool approxEquals(const double a, const double b) {
	return fabs((double)a-b)<0.0001;
}
typedef vector<Constraint*> CS;

void test1() {
	cout << "Test 1..." << endl;
	Variable *a[] = {
		new Variable(0,2,1),
		new Variable(1,9,1),
		new Variable(2,9,1),
		new Variable(3,9,1),
		new Variable(4,2,1)};
	Constraint *c[] = {
		new Constraint(a[0],a[4],3),
		new Constraint(a[0],a[4],3),
		new Constraint(a[0],a[1],3),
		new Constraint(a[1],a[2],3),
		new Constraint(a[1],a[2],3),
		new Constraint(a[2],a[4],3),
		new Constraint(a[3],a[4],3),
		new Constraint(a[3],a[4],3),
		new Constraint(a[3],a[4],3)};
	unsigned int n = sizeof(a)/sizeof(Variable*);
	unsigned int m = sizeof(c)/sizeof(Constraint*);
	IncSolver vpsc(n,a,m,c);
	vpsc.satisfy();
	cout << "Test 1... done." << endl;
}

// no splits required
void test2() {
	cout << "Test 2..." << endl;
	Variable *a[] = {
		new Variable(0,4,1),
		new Variable(1,6,1),
		new Variable(2,9,1),
		new Variable(3,2,1),
		new Variable(4,5,1)};
	Constraint *c[] = {
		new Constraint(a[0],a[2],3),
		new Constraint(a[0],a[3],3),
		new Constraint(a[1],a[4],3),
		new Constraint(a[2],a[4],3),
		new Constraint(a[2],a[3],3),
		new Constraint(a[3],a[4],3)};
	double expected[]={0.5,6,3.5,6.5,9.5};
	unsigned int n = sizeof(a)/sizeof(Variable*);
	unsigned int m = sizeof(c)/sizeof(Constraint*);
	IncSolver vpsc(n,a,m,c);
	vpsc.solve();
	for(int i=0;i<n;i++) {
		printf("%f,",a[i]->position());
		//assert(approxEquals(a[i]->position(),expected[i]));
	}
	cout << "Test 2... done." << endl;
}

// split required - cycles without change in direction check
void test3() {
	cout << "Test 3..." << endl;
	Variable *a[] = {
		new Variable(0,5,1),
		new Variable(1,6,1),
		new Variable(2,7,1),
		new Variable(3,4,1),
		new Variable(4,3,1)};
	Constraint *c[] = {
		new Constraint(a[0],a[4],3),
		new Constraint(a[1],a[2],3),
		new Constraint(a[2],a[3],3),
		new Constraint(a[2],a[4],3),
		new Constraint(a[3],a[4],3)};
	double expected[]={6.2,0.2,3.2,6.2,9.2};
	unsigned int n = sizeof(a)/sizeof(Variable*);
	unsigned int m = sizeof(c)/sizeof(Constraint*);
	IncSolver vpsc(n,a,m,c);
	vpsc.satisfy();
	for(int i=0;i<n;i++) {
		printf("%f,",a[i]->position());
		//assert(approxEquals(a[i]->position(),expected[i]));
	}
	cout << "Test 3... done." << endl;
}
// split required
void test4() {
	cout << "Test 4..." << endl;
	Variable *a[] = {
		new Variable(0,7,1),
		new Variable(1,1,1),
		new Variable(2,6,1),
		new Variable(3,0,1),
		new Variable(4,2,1)};
	Constraint *c[] = {
		new Constraint(a[0],a[3],3),
		new Constraint(a[0],a[1],3),
		new Constraint(a[1],a[4],3),
		new Constraint(a[2],a[4],3),
		new Constraint(a[2],a[3],3),
		new Constraint(a[3],a[4],3)};
	double expected[]={5,0.5,3.5,6.5,9.5};
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
	cout << "Test 4... done." << endl;
}
void test5() {
	cout << "Test 5..." << endl;
	Variable *a[] = {
		new Variable(0,0,1), new Variable(1,9,1), new
			Variable(2,1,1), new Variable(3,9,1), new
			Variable(4,5,1), new Variable(5,1,1), new
			Variable(6,2,1), new Variable(7,1,1), new
			Variable(8,6,1), new Variable(9,3,1)};
	Constraint *c[] = {
		new Constraint(a[0],a[3],3), new Constraint(a[1],a[8],3),
		new Constraint(a[1],a[6],3), new Constraint(a[2],a[6],3),
		new Constraint(a[3],a[5],3), new Constraint(a[3],a[6],3),
		new Constraint(a[3],a[7],3), new Constraint(a[4],a[8],3),
		new Constraint(a[4],a[7],3), new Constraint(a[5],a[8],3),
		new Constraint(a[5],a[7],3), new Constraint(a[5],a[8],3),
		new Constraint(a[6],a[9],3), new Constraint(a[7],a[8],3),
		new Constraint(a[7],a[9],3), new Constraint(a[8],a[9],3)
	};
	double expected[]={-3.71429,4,1,-0.714286,2.28571,2.28571,7,5.28571,8.28571,11.2857};
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
		printf("%f,",a[i]->position());
		//assert(approxEquals(a[i]->position(),expected[i]));
	}
	cout << "Test 5... done." << endl;
}
void test6() {
	cout << "Test 6..." << endl;
	Variable *a[] = {
		new Variable(0,7,1),
		new Variable(1,0,1),
		new Variable(2,3,1),
		new Variable(3,1,1),
		new Variable(4,4,1)
	};
	Constraint *c[] = {
		new Constraint(a[0],a[3],3),
		new Constraint(a[0],a[2],3),
		new Constraint(a[1],a[4],3),
		new Constraint(a[1],a[4],3),
		new Constraint(a[2],a[3],3),
		new Constraint(a[3],a[4],3)
	};
	double expected[]={-0.75,0,2.25,5.25,8.25};
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
	cout << "Test 6... done." << endl;
}
void test7() {
	cout << "Test 7..." << endl;
	Variable *a[] = {
		new Variable(0,4,1),
		new Variable(1,2,1),
		new Variable(2,3,1),
		new Variable(3,1,1),
		new Variable(4,8,1)
	};
	Constraint *c[] = {
		new Constraint(a[0],a[4],3),
		new Constraint(a[0],a[2],3),
		new Constraint(a[1],a[3],3),
		new Constraint(a[2],a[3],3),
		new Constraint(a[2],a[4],3),
		new Constraint(a[3],a[4],3)
	};
	double expected[]={-0.5,2,2.5,5.5,8.5};
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
	cout << "Test 7... done." << endl;
}
void test8() {
	// This cycles when using the heuristic of merging on the least
	// violated, violated constraint first.
	cout << "Test 8..." << endl;
	Variable *a[] = {
		new Variable(0,3,1),
		new Variable(1,4,1),
		new Variable(2,0,1),
		new Variable(3,5,1),
		new Variable(4,6,1)
	};
	Constraint *c[] = {
		new Constraint(a[0],a[1],3),
		new Constraint(a[0],a[2],3),
		new Constraint(a[0],a[2],3),
		new Constraint(a[1],a[2],3),
		new Constraint(a[1],a[4],3),
		new Constraint(a[2],a[3],3),
		new Constraint(a[2],a[3],3),
		new Constraint(a[3],a[4],3),
		new Constraint(a[3],a[4],3)
	};
	double expected[]={-0.5,2,2.5,5.5,8.5};
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
	cout << "Test 8... done." << endl;
}
void test9() {
	cout << "Test 9..." << endl;
	Variable *a[] = {
		new Variable(0,8,1),
		new Variable(1,2,1),
		new Variable(2,6,1),
		new Variable(3,5,1),
		new Variable(4,3,1)};
	Constraint *c[] = {
		new Constraint(a[0],a[4],3),
		new Constraint(a[0],a[3],3),
		new Constraint(a[1],a[2],3),
		new Constraint(a[1],a[4],3),
		new Constraint(a[2],a[3],3),
		new Constraint(a[2],a[4],3),
		new Constraint(a[3],a[4],3)};
	double expected[]={0.5,6,3.5,6.5,9.5};
	unsigned int n = sizeof(a)/sizeof(Variable*);
	unsigned int m = sizeof(c)/sizeof(Constraint*);
	IncSolver vpsc(n,a,m,c);
	vpsc.solve();
	for(int i=0;i<n;i++) {
		printf("%f,",a[i]->position());
		//assert(approxEquals(a[i]->position(),expected[i]));
	}
	cout << "Test 9... done." << endl;
}

void test10() {
	cout << "Test 10..." << endl;
	Variable *a[] = {
		new Variable(0,5,1),
		new Variable(1,4,1),
		new Variable(2,0,1),
		new Variable(3,6,1),
		new Variable(4,4,1)};
	Constraint *c[] = {
		new Constraint(a[0],a[1],3),
		new Constraint(a[0],a[2],3),
		new Constraint(a[1],a[4],3),
		new Constraint(a[1],a[2],3),
		new Constraint(a[2],a[4],3),
		new Constraint(a[3],a[4],3)};
	double expected[]={5,0.5,3.5,6.5,9.5};
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
	cout << "Test 10... done." << endl;
}

// n=number vars
// m=max constraints per var
void rand_test(int n, int m) {
	Variable *a[n];
	CS cs;
	for(int i=0;i<n;i++) {
		a[i]=new Variable(i,getRand(10),1);
	}
	for(int i=0;i<n-1;i++) {
		for(int j=0;j<getRand(m)+1;j++) {
			int e = i + getRand(n-1-i)+1;
			cs.push_back(new Constraint(a[i],a[e],3));
		}
	}
	Constraint *acs[cs.size()];
	for(int i=0;i<cs.size();i++) {
		acs[i]=cs[i];
	}
	/*
	cout<<"digraph g {"<<endl;
	for(CS::iterator i(cs.begin());i!=cs.end();i++) {
		Constraint *c=*i;
		cout << c->left->id << "->" << c->right->id << ";" << endl;
	}
	cout<<"}"<<endl;
	*/
	IncSolver vpsc(n,a,cs.size(),acs);
	try {
		vpsc.solve();
	} catch (char const *msg) {
		cout << msg << endl;
		for(int i=0;i<n;i++) {
			cout << "new Variable("<<i<<","<<a[i]->desiredPosition<< ",1)," << endl;
			//cout << "a[i]->Pos="<<a[i]->position() << endl;
		}
		for(CS::iterator i(cs.begin());i!=cs.end();i++) {
			Constraint *c=*i;
			//cout << c->left->id << "->" << c->right->id << ";" << endl;
			cout << "new Constraint(a[" <<  c->left->id << "],a[" <<  c->right->id << "],3)," << endl;
		}
		throw "test failed!";
	}
}
int main() {
	srand(time(NULL));
	test1();
	test2();
	test3();
	test4();
	test5();
	test6();
	test7();
	test8();
	test9();
	test10();
	for(int i=0;i<1000;i++) {
		if(i%100==0) cout << "i=" << i << endl;
		rand_test(100,3);
	}
	/*
	for(int i=0;i<10000;i++) {
		if(i%100==0) cout << "i=" << i << endl;
		rand_test(5,3);
	}
	*/
	return 0;
}
