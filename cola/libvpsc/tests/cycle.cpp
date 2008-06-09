#include <iostream>
#include <cassert>
#include <cmath>
#include <algorithm>
#include <libvpsc/rectangle.h>
#include <libvpsc/variable.h>
#include <libvpsc/constraint.h>
#include <libvpsc/solve_VPSC.h>
using namespace std;
using namespace vpsc;
inline bool approxEquals(const double a, const double b) {
	return fabs((double)a-b)<0.0001;
}
void test1() {
	cout << "Test 1..." << endl;
	vector<Variable*> a;
	a.push_back(new Variable(0,0,1));
	a.push_back(new Variable(1,1,1));
	vector<Constraint*> c;
	c.push_back(new Constraint(a[0],a[1],2));
	c.push_back(new Constraint(a[1],a[0],2));
	double expected[]={1.5,-0.5};
	try {
		IncSolver vpsc(a,c);
		vpsc.solve();
	} catch (UnsatisfiableException& e) {
		cerr << "Unsatisfiable" << endl;
		for(vector<Constraint*>::iterator i=e.path.begin();
				i!=e.path.end();i++) {
			cout << **i << endl;
		}
		exit(1);
	} 
	//catch(...) {
		//cerr << "Unknown error!" << endl;
		//exit(1);
	//}

	for(size_t i=0;i<a.size();i++) {
		assert(approxEquals(a[i]->finalPosition,expected[i]));
	}
	for_each(a.begin(),a.end(),delete_object());
	for_each(c.begin(),c.end(),delete_object());
	cout << "Test 1... done." << endl;
}
void test2() {
	cout << "Test 2..." << endl;
	vector<Variable *> a;
	a.push_back(new Variable(0,8,1));
	a.push_back(new Variable(1,5,1));
	a.push_back(new Variable(2,3,1));
	a.push_back(new Variable(3,1,1));
	vector<Constraint*> c;
	c.push_back(new Constraint(a[0],a[3],3));
	c.push_back(new Constraint(a[0],a[1],3));
	c.push_back(new Constraint(a[1],a[3],3));
	c.push_back(new Constraint(a[1],a[2],3));
	c.push_back(new Constraint(a[2],a[3],3));
	c.push_back(new Constraint(a[2],a[3],3));
	//double expected[]={-3.71429,4,1,-0.714286,2.28571,2.28571,7,5.28571,8.28571,11.2857};
	try {
		IncSolver vpsc(a,c);
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
	for_each(a.begin(),a.end(),delete_object());
	for_each(c.begin(),c.end(),delete_object());
}
int main() {
	test1();
}
