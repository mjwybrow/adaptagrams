#include <iostream>
#include <iterator>
#include "treeqp.h"
using namespace std;

int main() {
    unsigned n=5;
    Terms ts;
    ts.push_back(Term(0,1,1,0));
    ts.push_back(Term(0,2,1,0));
    ts.push_back(Term(2,3,1,-93));
    ts.push_back(Term(2,4,1,73));
    EqualityConstraints ecs;
    ecs.push_back(EqualityConstraint(2,3,0));
    ecs.push_back(EqualityConstraint(0,1,2,292));
    InequalityConstraints ics;
    ics.push_back(InequalityConstraint(1, 2, 292));
    ics.push_back(InequalityConstraint(3, 4, 209));

    vector<double> x(n);
    treeQPSolve(ts,ecs,ics,x);
    cout << "solution:" << endl;
    copy(x.begin(),x.end(),ostream_iterator<double>(cout,"\n"));

    return 0;
}
/*
 * vim: set cindent 
 * vim: ts=4 sw=4 et tw=0 wm=0
 */
