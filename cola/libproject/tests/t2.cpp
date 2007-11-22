/**
 * \file t2.cpp
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

const char* t2(Variables &vs, Constraints &cs) {
vs.push_back(new Variable(0.343187,1.289946));//soln=0.940896 != rslt=0.343187
vs.push_back(new Variable(0.966096,1.295395));//soln=1.295395 != rslt=0.966096
vs.push_back(new Variable(1.348192,1.385243));//soln=1.734292 != rslt=1.348192
cs.push_back(new Constraint(vs[0],vs[2],0.793396));
cs.push_back(new Constraint(vs[1],vs[2],0.320881));
return __ASSERT_FUNCTION;
}

void test(const char* (*t)(Variables&, Constraints&)) {
    Variables vs;
    Constraints cs;
    const char *testName=t(vs,cs);
    
    // store initial positions
    vector<double> XI(vs.size());
    VMap vmap;
    for(unsigned i=0;i<vs.size();i++) {
        XI[i]=vs[i]->x;
        vmap[vs[i]]=i;
    }

    // assert initial solution is feasible:
    assert(feasible(cs));

    // run standard qpsolver
    vector<double> qpresult;
    qps(vs,cs,qpresult);

    try {
        // run our solver
        FeasibleProjection f(vs,cs);
        f.solve();

        // assert final solution is feasible:
        if(!feasible(cs)) {
            throw "infeasible solution!";
        }
        // assert solution matches solution from standard QP solver
        for(unsigned i=0;i<vs.size();i++) {
            if(!approxEquals(vs[i]->x,qpresult[i])) {
                throw "incorrect solution!";
            }
        }
        printf("PASS: %s\n",testName);
    } catch(const char *e) {
        printProblem(vs,XI,cs,vmap);
        fprintf(stderr,"FAIL: %s\n%s\n",testName,e);
        exit(1);
    }
    for_each(vs.begin(),vs.end(),delete_object());
    for_each(cs.begin(),cs.end(),delete_object());
}

int main() {
    test(t1);
    test(t2);
    return 0;
}
/*
 * vim: set cindent 
 * vim: ts=4 sw=4 et tw=0 wm=0
 */
