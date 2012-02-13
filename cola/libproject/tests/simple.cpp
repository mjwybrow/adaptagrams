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

/**
 * \file simple.cpp
 *
 * Simple self contained test cases that also serve as basic examples of how
 * to use libproject.
 * By simple I mean they don't include any complicated test harness
 * or external functionality other than that provided by libproject.
 */
#include <iostream>
#include <cassert>
#include <cmath>
#include <algorithm>
#include <libproject/project.h>
#include <libproject/util.h>

using namespace project;
using namespace std;
/**
 * three variables, two constraints, splitting required.
 */
void simple1() {
    Variables vs;
    Constraints cs;
    vs.push_back(new Variable(Initial(0.343187),Desired(1.289946)));
    vs.push_back(new Variable(Initial(0.966096),Desired(1.295395)));
    vs.push_back(new Variable(Initial(1.348192),Desired(1.385243)));
    cs.push_back(new Constraint(vs[0],vs[2],0.793396));
    cs.push_back(new Constraint(vs[1],vs[2],0.320881));
    Project f(vs,cs);
    f.solve();
    double expected[]={0.940896,1.295395,1.734292};
    for(unsigned i=0;i<vs.size();i++) {
        printf("vs[%d]=%f\n",i,vs[i]->getPosition());
        assert(fabs(vs[i]->getPosition() - expected[i]) < 1e-5);
    }
    for_each(vs.begin(),vs.end(),delete_object());
    for_each(cs.begin(),cs.end(),delete_object());
}
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
void simple2() {
    unsigned n=100;
    Variables vs;
    for(unsigned i=0;i<n;i++) {
        vs.push_back(new Variable(Initial(i),Desired(0)));
    }
    Constraints cs;
    for(unsigned i=0;i<n-1;i++) {
        cs.push_back(new Constraint(vs[i],vs[i+1],1));
    }
    solve(vs,cs);
    unsigned i=0;
    double firstPos=-(n-1.0)/2.0;
    for(Variables::iterator j=vs.begin();j!=vs.end();i++,j++) {
        Variable *v=*j;
        double expected=firstPos+i;
        //cout << "vs["<<i<<"]="<<v->getPosition() << ", expected="<<expected<<endl;
        assert(expected==v->getPosition());
    }
    for_each(vs.begin(),vs.end(),delete_object());
    for_each(cs.begin(),cs.end(),delete_object());
}
int main() {
    simple1();
    printf("PASS: simple1\n");
    simple2();
    printf("PASS: simple2\n");
    return 0;
}
/*
 * vim: set cindent 
 * vim: ts=4 sw=4 et tw=0 wm=0
 */
