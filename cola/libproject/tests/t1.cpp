#include <iostream>
#include <cassert>
#include <libproject/project.h>
#include <libproject/util.h>

using namespace project;
using namespace std;
/**
 * basic not very challenging test case:
 *   - set of variables V starting positions
 *      v_i+1=v_{i+1} 
 *   - generate chain of regularly separated (by 1) constraints:
 *      {C(i,i+1,1) | v_i in V, 0<=i<|V|}
 *   - desired positions guarantee all constraints will be active
 *
 * Notes:
 *   - no splitting required
 *   - all constraints active
 *   - constraint DAG is a simple chain
 */
int main() {
	unsigned n=100;
	Variables vs;
	for(unsigned i=0;i<n;i++) {
		vs.push_back(new Variable(i,0));
	}
	Constraints cs;
	for(unsigned i=0;i<n-1;i++) {
		cs.push_back(new Constraint(vs[i],vs[i+1],1));
	}
	FeasibleProjection f(vs,cs);
	f.solve();
	unsigned i=0;
	double firstPos=-(n-1.0)/2.0;
	for(Variables::iterator j=vs.begin();j!=vs.end();i++,j++) {
		Variable *v=*j;
		double expected=firstPos+i;
		//cout << "vs["<<i<<"]="<<v->x << ", expected="<<expected<<endl;
		assert(expected==v->x);
	}
	for_each(vs.begin(),vs.end(),delete_object());
	for_each(cs.begin(),cs.end(),delete_object());
	return 0;
}
