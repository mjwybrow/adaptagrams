#include <iostream>
#include <iterator>
#include "treeqp.h"
using namespace std;

int main() {
    unsigned n=5;
    Terms ts;
    Constraints cs;
    ts.push_back(new Term(0,1,1,0));
    ts.push_back(new Term(0,2,1,0));
    ts.push_back(new Term(2,3,1,-93));
    ts.push_back(new Term(2,4,1,73));
    cs.push_back(new Constraint(1, 2, 292, false));
    cs.push_back(new Constraint(3, 4, 209, false));

    vector<double> x(n);
    treeQPSolve(ts,cs,x);
    cout << "solution:" << endl;
    copy(x.begin(),x.end(),ostream_iterator<double>(cout,"\n"));
    return 0;
}
/*
 * vim: set cindent 
 * vim: ts=4 sw=4 et tw=0 wm=0
 */
