/**
 * \file static.cpp
 * small test cases that have identified bugs at various times
 */
#include <iostream>
#include <cassert>
#include <libproject/project.h>
#include <libproject/util.h>
#include "testutil.h"

using namespace project;
using namespace std;

/**
 * simple merges, no split
 */
const char* t1(Variables &vs, Constraints &cs) {
vs.push_back(new Variable(0.032341,2.065240));
vs.push_back(new Variable(0.644455,2.642579));
vs.push_back(new Variable(1.456425,2.615846));
cs.push_back(new Constraint(vs[0],vs[2],0.973464));
cs.push_back(new Constraint(vs[1],vs[2],0.367541));
return __ASSERT_FUNCTION;
}

/**
 * two merges and one split required
 */
const char* t2(Variables &vs, Constraints &cs) {
vs.push_back(new Variable(0.343187,1.289946));//soln=0.940896
vs.push_back(new Variable(0.966096,1.295395));//soln=1.295395
vs.push_back(new Variable(1.348192,1.385243));//soln=1.734292
cs.push_back(new Constraint(vs[0],vs[2],0.793396));
cs.push_back(new Constraint(vs[1],vs[2],0.320881));
return __ASSERT_FUNCTION;
}

/**
 * requires two splits
 */
const char* t3(Variables& vs, Constraints &cs) {
vs.push_back(new Variable(0.388295,3.504715));//soln=2.161169
vs.push_back(new Variable(1.061545,4.048542));//soln=4.048542
vs.push_back(new Variable(1.372616,2.417464));//soln=2.417464
vs.push_back(new Variable(1.452004,2.779394));//soln=2.779394
vs.push_back(new Variable(1.891515,1.840865));//soln=3.184411
cs.push_back(new Constraint(vs[0],vs[4],1.023242));
cs.push_back(new Constraint(vs[2],vs[3],0.025524));
cs.push_back(new Constraint(vs[3],vs[4],0.399988));
return __ASSERT_FUNCTION;
}

/**
 * redundant constraints
 */
const char* t4(Variables& vs, Constraints &cs) {
vs.push_back(new Variable(0.875029,2.370362));//soln=1.928746 != rslt=0.875029
vs.push_back(new Variable(1.146413,1.889098));//soln=2.007311 != rslt=1.146413
vs.push_back(new Variable(1.904400,2.402919));//soln=2.726321 != rslt=1.904400
cs.push_back(new Constraint(vs[0],vs[2],0.198559));
cs.push_back(new Constraint(vs[1],vs[2],0.719010));
cs.push_back(new Constraint(vs[0],vs[1],0.078566));
cs.push_back(new Constraint(vs[0],vs[1],0.076233));
return __ASSERT_FUNCTION;
}

int main() {
    //test(t1);
    //test(t2);
    //test(t3);
    test(t4);
    return 0;
}
/*
 * vim: set cindent 
 * vim: ts=4 sw=4 et tw=0 wm=0
 */
