/*
 * vim: ts=4 sw=4 et tw=0 wm=0
 *
 * libvpsc - A solver for the problem of Variable Placement with 
 *           Separation Constraints.
 *
 * Copyright (C) 2005-2008  Monash University
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library in the file LICENSE; if not, 
 * write to the Free Software Foundation, Inc., 59 Temple Place, 
 * Suite 330, Boston, MA  02111-1307  USA
 *
*/

#include <libvpsc/rectangle.h>
#include <libvpsc/variable.h>
#include <libvpsc/constraint.h>
#include <libvpsc/solve_VPSC.h>
#include <algorithm>
#include <cstdio>
#include <ctime>
#include <cmath>
#include <cassert>

using namespace std;
using namespace vpsc;

static inline double getRand(const int range) {
	return (double)range*rand()/(RAND_MAX+1.0);
}
inline bool approxEquals(const double a, const double b) {
	return fabs((double)a-b)<0.01;
}
typedef vector<Constraint*> CS;

bool checkResult(unsigned n, Variable *a[], unsigned m, Constraint *c[], double expected[]=nullptr) {
        std::vector<Variable*> aa(a,a+n);
        std::vector<Constraint*> cc(c,c+m);
	IncSolver vpsc(aa,cc);
	vpsc.solve();
#ifdef MOSEK_AVAILABLE
	//printf("Checking with mosek...");
	MosekEnv* menv = mosek_init_sep_ls(n,c,m);
	float *b=new float[n];
	float *x=new float[n];
	for(unsigned i=0;i<n;i++) {
		b[i]=a[i]->desiredPosition;
	}
	mosek_quad_solve_sep(menv,b,x);
	mosek_delete(menv);
#endif
	for(unsigned i=0;i<n;i++) {
		char s=',';
		if(i==n-1) s='\n';
#ifdef MOSEK_AVAILABLE
		//printf("%f(%f)%c",a[i]->finalPosition,x[i],s);
		if(!(approxEquals(a[i]->finalPosition,x[i]))) {
			return false;
		}
		assert(approxEquals(a[i]->finalPosition,x[i]));
#endif
		if(expected) assert(approxEquals(a[i]->finalPosition,expected[i]));
	}
#ifdef MOSEK_AVAILABLE
	delete [] b;
	delete [] x;
#endif
	return true;
}
void dumpMatlabProblem(unsigned n, Variable *a[], unsigned m, Constraint *c[]){
	printf("H=2*eye(%d);\n",n);
	printf("f=-2*[ ");
	for(unsigned i=0;i<n;i++) {
		printf("%f ",a[i]->desiredPosition);
	}
	printf("];\n");
	printf("s=[ ");
	for(unsigned i=0;i<n;i++) {
		printf("%f ",a[i]->scale);
	}
	printf("];\n");
	printf("C=zeros(%d,%d);\n",m,n);
	for(unsigned i=0;i<m;i++) {
		printf("C(%d,[%d %d])=[1 -1];\n",i+1,c[i]->left->id+1,c[i]->right->id+1);
	}
	printf("A=C*diag(s);\n");
	printf("b=[ ");
	for(unsigned i=0;i<m;i++) {
		printf("%f ",-1.*c[i]->gap);
	}
	printf("];\n");
	printf("quadprog(H,f,A,b)\n");
}
void test1() {
	cout << "Test 1..." << endl;
	Variable *a[] = {
		new Variable(0,2,1,1.),
		new Variable(1,9,1,1),
		new Variable(2,9,1,1),
		new Variable(3,9,1,1),
		new Variable(4,2,1,1)};
	Constraint *c[] = {
		new Constraint(a[0],a[4],3),
		new Constraint(a[0],a[1],3),
		new Constraint(a[1],a[2],3),
		new Constraint(a[2],a[4],3),
		new Constraint(a[3],a[4],3)};
	unsigned int n = sizeof(a)/sizeof(Variable*);
	unsigned int m = sizeof(c)/sizeof(Constraint*);
	double expected[]={1.4,4.4,7.4,7.4,10.4};
	checkResult(n,a,m,c,expected);
	cout << "Test 1... done." << endl;
}
void test1a() {
	cout << "Test 1a..." << endl;
	/* matlab:
	 H=2*eye(2)
	 f=[0 0]
	 A=[ 2 -1 ]
	 b=[ -2 ]
	 quadprog(H,f,A,b)
	 */
	Variable *a[] = {
		new Variable(0,0,1,2.),
		new Variable(1,0,1,1)};
	Constraint *c[] = {
		new Constraint(a[0],a[1],2)};
	unsigned int n = sizeof(a)/sizeof(Variable*);
	unsigned int m = sizeof(c)/sizeof(Constraint*);
	double expected[]={-0.8,0.4};
	checkResult(n,a,m,c,expected);
	cout << "Test 1a... done." << endl;
}
void test1b() {
	cout << "Test 1b..." << endl;
	/* matlab:
	 H=2*eye(2)
	 f=[0 0]
	 A=[ 1 -2 ]
	 b=[ -2 ]
	 quadprog(H,f,A,b)
	 */
	Variable *a[] = {
		new Variable(0,0,1,1.),
		new Variable(1,0,1,2)};
	Constraint *c[] = {
		new Constraint(a[0],a[1],2)};
	unsigned int n = sizeof(a)/sizeof(Variable*);
	unsigned int m = sizeof(c)/sizeof(Constraint*);
	double expected[]={-0.4,0.8};
	checkResult(n,a,m,c,expected);
	cout << "Test 1b... done." << endl;
}
void test1c() {
	cout << "Test 1c..." << endl;
	/* matlab:
	 H=2*eye(3)
	 f=-2*[ 1 1 1 ]
	 s=[ 3  2  4 ]
	 C=zeros(2,3)
	 C(1,1:2)=[1 -1]
	 C(2,2:3)=[1 -1]
	 A=C*diag(s)
	 b=[-2 -2 ]
	 quadprog(H,f,A,b)
	 */
	Variable *a[] = {
		new Variable(0,1,1,3),
		new Variable(1,1,1,2),
		new Variable(2,1,1,4)};
	Constraint *c[] = {
		new Constraint(a[0],a[1],2),
		new Constraint(a[1],a[2],2)};
	unsigned int n = sizeof(a)/sizeof(Variable*);
	unsigned int m = sizeof(c)/sizeof(Constraint*);
	double expected[]={0.2623, 1.3934, 1.1967};
	checkResult(n,a,m,c,expected);
	cout << "Test 1c... done." << endl;
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
	checkResult(n,a,m,c,expected);
	cout << "Test 2... done." << endl;
}

// split required
void test3() {
	/* matlab:
	 H=2*eye(5)
	 f=-2*[ 5 6 7 4 3 ]
	 s=[ 1 1 1 1 1 ]
	 C=zeros(5,5)
	 C(1,[1 5])=[1 -1]
	 C(2,[2 3])=[1 -1]
	 C(3,[3 4])=[1 -1]
	 C(4,[3 5])=[1 -1]
	 C(5,[4 5])=[1 -1]
	 A=C*diag(s)
	 b=-3*ones(5,1)
	 quadprog(H,f,A,b)
	 */
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
	double expected[]={5,0.5,3.5,6.5,9.5};
	unsigned int n = sizeof(a)/sizeof(Variable*);
	unsigned int m = sizeof(c)/sizeof(Constraint*);
	checkResult(n,a,m,c,expected);
	cout << "Test 3... done." << endl;
}
// split required
void test4() {
	/* matlab:
	 H=2*eye(5)
	 f=-2*[ 7 1 6 0 2 ]
	 s=[ 5 8 3 1 7 ]
	 C=zeros(6,5)
	 C(1,[1 4])=[1 -1]
	 C(2,[1 2])=[1 -1]
	 C(3,[2 5])=[1 -1]
	 C(4,[3 5])=[1 -1]
	 C(5,[3 4])=[1 -1]
	 C(6,[4 5])=[1 -1]
	 A=C*diag(s)
	 b=-3*ones(6,1)
	 quadprog(H,f,A,b)
	 */
	cout << "Test 4..." << endl;
	Variable *a[] = {
		new Variable(0,7,1,1),
		new Variable(1,1,1,1),
		new Variable(2,6,1,1),
		new Variable(3,0,1,1),
		new Variable(4,2,1,1)};
	Constraint *c[] = {
		new Constraint(a[0],a[3],3),
		new Constraint(a[0],a[1],3),
		new Constraint(a[1],a[4],3),
		new Constraint(a[2],a[4],3),
		new Constraint(a[2],a[3],3),
		new Constraint(a[3],a[4],3)};
	double expected[]={0.8,3.8,0.8,3.8,6.8};
	unsigned int n = sizeof(a)/sizeof(Variable*);
	unsigned int m = sizeof(c)/sizeof(Constraint*);
	checkResult(n,a,m,c,expected);
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
	checkResult(n,a,m,c,expected);
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
	checkResult(n,a,m,c,expected);
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
	checkResult(n,a,m,c,expected);
	cout << "Test 7... done." << endl;
}
void test8() {
	/* matlab:
	 H=2*eye(5)
	 f=-2*[ 3 4 0 5 6 ]
	 s=[ 1 1 1 1 1 ]
	 C=zeros(6,5)
	 C(1,[1 2])=[1 -1]
	 C(2,[1 3])=[1 -1]
	 C(3,[2 3])=[1 -1]
	 C(4,[2 5])=[1 -1]
	 C(5,[3 4])=[1 -1]
	 C(6,[4 5])=[1 -1]
	 A=C*diag(s)
	 b=-3*ones(6,1)
	 quadprog(H,f,A,b)
	 */
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
		new Constraint(a[1],a[2],3),
		new Constraint(a[1],a[4],3),
		new Constraint(a[2],a[3],3),
		new Constraint(a[2],a[3],3),
		new Constraint(a[3],a[4],3),
		new Constraint(a[3],a[4],3)
	};
	double expected[]={-2.4,0.6,3.6,6.6,9.6};
	unsigned int n = sizeof(a)/sizeof(Variable*);
	unsigned int m = sizeof(c)/sizeof(Constraint*);
	checkResult(n,a,m,c,expected);
	cout << "Test 8... done." << endl;
}
void test9() {
	/* matlab:
	 H=2*eye(5)
	 f=-2*[ 8 2 6 5 3 ]
	 s=[ 1 1 1 1 1 ]
	 C=zeros(7,5)
	 C(1,[1 5])=[1 -1]
	 C(2,[1 4])=[1 -1]
	 C(3,[2 3])=[1 -1]
	 C(4,[2 5])=[1 -1]
	 C(5,[3 4])=[1 -1]
	 C(6,[3 5])=[1 -1]
	 C(7,[4 5])=[1 -1]
	 A=C*diag(s)
	 b=-3*ones(7,1)
	 quadprog(H,f,A,b)
	 */
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
	double expected[]={3.6,0.6,3.6,6.6,9.6};
	unsigned int n = sizeof(a)/sizeof(Variable*);
	unsigned int m = sizeof(c)/sizeof(Constraint*);
	checkResult(n,a,m,c,expected);
	cout << "Test 9... done." << endl;
}

void test10() {
	cout << "Test 10..." << endl;
	Variable *a[] = {
		new Variable(0,8.56215,1,4.99888),
new Variable(1,1.27641,1,8.06009),
new Variable(2,6.28523,1,1.06585),
new Variable(3,4.09743,1,0.924166),
new Variable(4,0.369025,1,6.12702)};

	Constraint *c[] = {
		new Constraint(a[0],a[2],3),
new Constraint(a[0],a[1],3),
new Constraint(a[0],a[1],3),
new Constraint(a[1],a[3],3),
new Constraint(a[1],a[3],3),
new Constraint(a[1],a[2],3),
new Constraint(a[2],a[4],3),
new Constraint(a[2],a[4],3),
new Constraint(a[3],a[4],3),
new Constraint(a[3],a[4],3)};

	//double expected[]={-1,2,5,5,8};
	unsigned int n = sizeof(a)/sizeof(Variable*);
	unsigned int m = sizeof(c)/sizeof(Constraint*);
        std::vector<Variable*> aa(a,a+n);
        std::vector<Constraint*> cc(c,c+m);
	IncSolver vpsc(aa,cc);
	vpsc.solve();
	assert(checkResult(n,a,m,c,nullptr));
	cout << "Test 10... done." << endl;
}
void test11() {
	cout << "Test 11..." << endl;
	Variable *a[] = {
		new Variable(0,1.31591,1,9.02545),
new Variable(1,1.48155,1,3.68918),
new Variable(2,3.5091,1,2.07033),
new Variable(3,3.47131,1,8.75145),
new Variable(4,0.77374,1,0.967941)};

	Constraint *c[] = {
new Constraint(a[0],a[3],3),
new Constraint(a[0],a[1],3),
new Constraint(a[1],a[4],3),
new Constraint(a[1],a[2],3),
new Constraint(a[2],a[4],3),
new Constraint(a[2],a[4],3),
new Constraint(a[3],a[4],3),
new Constraint(a[3],a[4],3)};

	//double expected[]={-1,2,5,5,8};
	unsigned int n = sizeof(a)/sizeof(Variable*);
	unsigned int m = sizeof(c)/sizeof(Constraint*);
	//dumpMatlabProblem(n,a,m,c);
        std::vector<Variable*> aa(a,a+n);
        std::vector<Constraint*> cc(c,c+m);
	IncSolver vpsc(aa,cc);
	vpsc.solve();
	assert(checkResult(n,a,m,c,nullptr));
	cout << "Test 11... done." << endl;
}
void test12() {
	cout << "Test 12..." << endl;
	Variable *a[] = {
new Variable(0,2.83063,1,6.67901),
new Variable(1,6.81696,1,7.28642),
new Variable(2,9.27616,1,0.918345),
new Variable(3,3.4094,1,3.39673),
new Variable(4,2.92492,1,2.36269)};

	Constraint *c[] = {
new Constraint(a[0],a[3],3),
new Constraint(a[0],a[2],3),
new Constraint(a[0],a[1],3),
new Constraint(a[1],a[4],3),
new Constraint(a[1],a[4],3),
new Constraint(a[1],a[4],3),
new Constraint(a[2],a[4],3),
new Constraint(a[2],a[4],3),
new Constraint(a[3],a[4],3),
new Constraint(a[3],a[4],3),
new Constraint(a[3],a[4],3),
new Constraint(a[3],a[4],3)};

	//double expected[]={-1,2,5,5,8};
	unsigned int n = sizeof(a)/sizeof(Variable*);
	unsigned int m = sizeof(c)/sizeof(Constraint*);
	//dumpMatlabProblem(n,a,m,c);
	assert(checkResult(n,a,m,c,nullptr));
	cout << "Test 12... done." << endl;
}
void test13() {
	cout << "Test 13..." << endl;
	Variable *a[] = {
new Variable(0,0.485024,1,1),
new Variable(1,3.52714,1,1),
new Variable(2,4.01263,1,1),
new Variable(3,4.58524,1,1),
new Variable(4,5.40796,1,1)};

	Constraint *c[] = {
new Constraint(a[0],a[4],3),
new Constraint(a[0],a[4],3),
new Constraint(a[0],a[4],3),
new Constraint(a[0],a[2],3),
new Constraint(a[1],a[3],3),
new Constraint(a[1],a[3],3),
new Constraint(a[1],a[2],3),
new Constraint(a[2],a[4],3),
new Constraint(a[2],a[4],3),
new Constraint(a[2],a[4],3),
new Constraint(a[2],a[3],3),
new Constraint(a[3],a[4],3),
new Constraint(a[3],a[4],3),
new Constraint(a[3],a[4],3)};

	//double expected[]={-1,2,5,5,8};
	unsigned int n = sizeof(a)/sizeof(Variable*);
	unsigned int m = sizeof(c)/sizeof(Constraint*);
	//dumpMatlabProblem(n,a,m,c);
	assert(checkResult(n,a,m,c,nullptr));
	cout << "Test 13... done." << endl;
}

// n=number vars
// m=max constraints per var
void rand_test(unsigned n, unsigned m) {
	Variable **a=new Variable*[n];
	CS cs;
	for(unsigned i=0;i<n;i++) {
		a[i]=new Variable(i,getRand(10),1,1);//getRand(10));
	}
	for(unsigned i=0;i<n-1;i++) {
		for(int j=0;j<getRand(m)+1;j++) {
			int e = static_cast<int>(i + getRand(n-1-i)+1);
			cs.push_back(new Constraint(a[i],a[e],3));
		}
	}
	Constraint **acs=new Constraint*[cs.size()];
	for(unsigned i=0;i<cs.size();i++) {
		acs[i]=cs[i];
	}
	try {
		if(!checkResult(n,a,cs.size(),acs)) {
			throw "Check failed!";
		}
	} catch (char const *msg) {
		cout << msg << endl;
		cout<<"digraph g {"<<endl;
		for(CS::iterator i(cs.begin());i!=cs.end();i++) {
			Constraint *c=*i;
			cout << c->left->id << "->" << c->right->id << ";" << endl;
		}
		cout<<"}"<<endl;
		for(unsigned i=0;i<n;i++) {
			if(i!=0) cout << "," << endl;
			cout << "new Variable("<<i<<","<<a[i]->desiredPosition<< ",1,"
				<< a[i]->scale <<")";
			//cout << "a[i]->Pos="<<a[i]->position() << endl;
		}
		cout << "};" << endl;
		for(CS::iterator i(cs.begin());i!=cs.end();i++) {
			if(i!=cs.begin()) cout << "," << endl;
			Constraint *c=*i;
			//cout << c->left->id << "->" << c->right->id << ";" << endl;
			cout << "new Constraint(a[" <<  c->left->id << "],a[" <<  c->right->id << "],3)";

		}
		cout << "};" << endl;
		throw "test failed!";
	}
	/*
	for(unsigned i=0;i<n;i++) {
		a[i]->desiredPosition=getRand(10);
	}
	vpsc.solve();
	try {
		if(!checkResult(n,a,m,acs)) {
			throw "2nd Check failed!";
		}
	} catch (char const *msg) {
		cout << msg << endl;
		for(unsigned i=0;i<n;i++) {
			if(i!=0) cout << "," << endl;
			cout << "new Variable("<<i<<","<<a[i]->desiredPosition<< ",1)";
			//cout << "a[i]->Pos="<<a[i]->position() << endl;
		}
		cout << "};" << endl;
		for(CS::iterator i(cs.begin());i!=cs.end();i++) {
			if(i!=cs.begin()) cout << "," << endl;
			Constraint *c=*i;
			//cout << c->left->id << "->" << c->right->id << ";" << endl;
			cout << "new Constraint(a[" <<  c->left->id << "],a[" <<  c->right->id << "],3)";

		}
		cout << "};" << endl;
		throw "test failed!";
	}
	*/
	for_each(a,a+n,delete_object());
	delete [] a;
	for_each(cs.begin(),cs.end(),delete_object());
	delete [] acs;
}
int main() {
	srand(time(nullptr));
	test10();
	test2();
	test3();
	test4();
	test5();
	test6();
	test7();
	test8();
	test9();
	test10();
	test11();
	test12();
	test13();
	for(int i=0;i<1000;i++) {
		if(i%100==0) cout << "i=" << i << endl;
		rand_test(100,3);
	}
	for(int i=0;i<10000;i++) {
		if(i%100==0) cout << "i=" << i << endl;
		rand_test(5,3);
	}
	return 0;
}
