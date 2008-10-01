/*
 * vim: ts=4 sw=4 et tw=0 wm=0
 *
 * libproject - Solves an instance of the Variable Placement with Separation
 *              Constraints problem, that is a projection onto separation 
 *              constraints, whilst always maintaining feasibility.
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

#include <iostream>
#include <cassert>
#include <libproject/project.h>
#include <libproject/util.h>
#include <cmath>
#include "testutil.h"

using namespace project;
using namespace std;

static unsigned numVars = 5;
static unsigned minConstraints = (unsigned)((double)numVars * 1.5);

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
void randomProblem(string& name, Variables &vs, Constraints &cs) {
    name = "randomProblem";
    double range=(double)numVars;
    double rpos=getRand(range/numVars);
    VMap vmap;
    vector<double> XI(numVars);
    for(unsigned i=0;i<numVars;i++) {
        Variable* v=new Variable(
                Initial(rpos),
                Desired(getRand(range)),
                Weight(getRand(100))
        );
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
                double g=r*(rv->getPosition()-lv->getPosition());
                cs.push_back(new Constraint(lv,rv,g));
            }
        }
    }
    //printf("minConstraints=%u,|V|=%u,|C|=%u\n",minConstraints,(unsigned)vs.size(),(unsigned)cs.size());
}
int main() {
    //srand(time(NULL));
    unsigned instances = 10000;
    for(numVars = 3; numVars < 10; numVars++) {
        printf("Running %d instances with %d variables...\n",instances,numVars);
        for(unsigned i=0;i<instances;i++) {
            test(randomProblem,true);
            //if(!(i%100)) {
                //printf("i=%d\n",i);
            //}
        }
    }
    instances = 1000;
    for(numVars = 10; numVars < 20; numVars++) {
        printf("Running %d instances with %d variables...\n",instances,numVars);
        for(unsigned i=0;i<instances;i++) {
            test(randomProblem,true);
            //if(!(i%100)) {
                //printf("i=%d\n",i);
            //}
        }
    }
    return 0;
}
/*
 * vim: set cindent 
 * vim: ts=4 sw=4 et tw=0 wm=0
 */
