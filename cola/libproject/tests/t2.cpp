/**
 * small test cases that have identified bugs at various times
 */
#include <iostream>
#include <cassert>
#include <libproject/project.h>
#include <libproject/util.h>

using namespace project;
using namespace std;
bool test1() {
	Variables vs;
	Constraints cs;
	vs.push_back(new Variable(0.611350,1.619516));
	vs.push_back(new Variable(1.230924,2.380480));
	vs.push_back(new Variable(1.233324,2.453532));
	cs.push_back(new Constraint(vs[0],vs[2],0.159439));
	FeasibleProjection f(vs,cs);
	f.solve();
	double epsilon=1e-10;
	for(Constraints::iterator i=cs.begin();i!=cs.end();++i) {
		Constraint* c=*i;
		double slack = c->l->x+c->g-c->r->x;
		assert(slack+epsilon>=0);
	}
	for_each(vs.begin(),vs.end(),delete_object());
	for_each(cs.begin(),cs.end(),delete_object());
}

int main() {
	test1();
	return 0;
}
