#include <iostream>
#include <cassert>
#include <libproject/project.h>
#include <libproject/util.h>
#include <cmath>
#include "testutil.h"

using namespace project;
using namespace std;

static const unsigned numVars = 3;
static const unsigned minConstraints = numVars * 1.5;

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
const char* randomProblem(Variables &vs, Constraints &cs) {
    double range=(double)numVars;
    double rpos=getRand(range/numVars);
    VMap vmap;
    vector<double> XI(numVars);
    for(unsigned i=0;i<numVars;i++) {
        Variable* v=new Variable(rpos,getRand(range));
        XI[i]=rpos;
        vmap[v]=i;
        vs.push_back(v);
        rpos+=getRand(range/(double)numVars);
    }
    while(cs.size()<minConstraints) {
        for(unsigned i=0;i<numVars-1;i++) {
            double probability = getRand(2);
            if(probability>1.0) {
                unsigned j=i+unsigned(ceil(getRand(numVars-i-1)));
                double r=getRand(1);
                Variable *lv=vs[i], *rv=vs[j];
                double g=r*(rv->x-lv->x);
                cs.push_back(new Constraint(lv,rv,g));
            }
        }
    }
    //printf("minConstraints=%u,|V|=%u,|C|=%u\n",minConstraints,(unsigned)vs.size(),(unsigned)cs.size());
    return __ASSERT_FUNCTION;
}
int main() {
    srand(time(NULL));
    for(unsigned i=0;i<10000;i++) {
        test(randomProblem,true);
        if(!(i%100)) {
            printf("i=%d\n",i);
        }
    }
    return 0;
}
/*
 * vim: set cindent 
 * vim: ts=4 sw=4 et tw=0 wm=0
 */
