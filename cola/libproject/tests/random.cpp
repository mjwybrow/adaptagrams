#include <iostream>
#include <cassert>
#include <libproject/project.h>
#include <libproject/util.h>
#include <cmath>

using namespace project;
using namespace std;
/**
 * Random test:
 *   - set of variables V with strictly increasing random starting positions
 *     and random desired positions
 *   - generate random constraints over pairs v_i, v_j where i<j
 *     with gap = r*(x_i - x_j) where 0<=r<=1
 *
 * Notes:
 *   - no splitting required
 *   - all constraints active
 *   - constraint DAG is a simple chain
 */
double getRand(const double range) {
	return (double)range*rand()/(RAND_MAX);
}
typedef map<const Variable*,unsigned> VMap;
void printProblem(Variables &vs, 
		vector<double> &XI, 
		Constraints &cs, 
		VMap &vmap) 
{
	vector<double>::iterator xi=XI.begin();
	for(Variables::iterator i=vs.begin();i!=vs.end();++i,++xi) {
		Variable* v=*i;
		printf("Variable(%f,%f);\n",*xi,v->d);
	}
	for(Constraints::iterator i=cs.begin();i!=cs.end();++i) {
		Constraint* c=*i;
		unsigned l=vmap[c->l], r=vmap[c->r];
		printf("Constraint(vs[%d],vs[%d],%f);\n", l,r,c->g);
	}
}
void test(unsigned n=10,double range=10) {
	Variables vs;
	double rpos=getRand(range/n);
	double epsilon = 1e-10;
	VMap vmap;
	vector<double> XI(n);
	for(unsigned i=0;i<n;i++) {
		Variable* v=new Variable(rpos,getRand(range));
		XI[i]=rpos;
		vmap[v]=i;
		vs.push_back(v);
		rpos+=getRand(range/n);
	}
	Constraints cs;
	for(unsigned i=0;i<n-1;i++) {
		if(getRand(2)>1) {
			unsigned j=i+unsigned(ceil(getRand(n-i-1)));
			double r=getRand(1);
			Variable *lv=vs[i], *rv=vs[j];
			double g=r*(rv->x-lv->x);
			cs.push_back(new Constraint(lv,rv,g));
		}
	}
	// assert initial solution is feasible:
	for(Constraints::iterator i=cs.begin();i!=cs.end();++i) {
		Constraint* c=*i;
		assert(c->r->x - c->l->x >= c->g);
	}
	FeasibleProjection f(vs,cs);
	f.solve();
	// assert final solution is feasible:
	for(Constraints::iterator i=cs.begin();i!=cs.end();++i) {
		Constraint* c=*i;
		double slack = c->l->x+c->g-c->r->x;
		if(slack<0) {
			unsigned l=vmap[c->l], r=vmap[c->r];
			printf("Unsatisfied: Constraint(vs[%d],vs[%d],%f), slack=%f\n", l,r,c->g,slack);
			printProblem(vs,XI,cs,vmap);
		}
		assert(slack+epsilon>=0);
	}
	for_each(vs.begin(),vs.end(),delete_object());
	for_each(cs.begin(),cs.end(),delete_object());
}
int main() {
	srand(time(NULL));
	for(unsigned i=0;i<10000;i++) {
		test(3,3);
		if(!(i%100)) {
			printf("i=%d\n",i);
		}
	}
	return 0;
}
