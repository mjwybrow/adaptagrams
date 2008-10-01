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
 * \file static.cpp
 * small test cases that have identified bugs at various times
 */
#include <iostream>
#include <libproject/project.h>
#include <libproject/util.h>
#include "testutil.h"

using namespace project;
using namespace std;

/**
 * simple merges, no split
 */
void t1(string& name, Variables &vs, Constraints &cs) {
name = "t1";
vs.push_back(new Variable(Initial(0.032341),Desired(2.065240)));
vs.push_back(new Variable(Initial(0.644455),Desired(2.642579)));
vs.push_back(new Variable(Initial(1.456425),Desired(2.615846)));
cs.push_back(new Constraint(vs[0],vs[2],0.973464));
cs.push_back(new Constraint(vs[1],vs[2],0.367541));
}

/**
 * two merges and one split required
 */
void t2(string& name, Variables &vs, Constraints &cs) {
name = "t2";
vs.push_back(new Variable(Initial(0.343187),Desired(1.289946)));//soln=0.940896
vs.push_back(new Variable(Initial(0.966096),Desired(1.295395)));//soln=1.295395
vs.push_back(new Variable(Initial(1.348192),Desired(1.385243)));//soln=1.734292
cs.push_back(new Constraint(vs[0],vs[2],0.793396));
cs.push_back(new Constraint(vs[1],vs[2],0.320881));
}

/**
 * requires two splits
 */
void t3(string& name, Variables& vs, Constraints &cs) {
name = "t3";
vs.push_back(new Variable(Initial(0.388295),Desired(3.504715)));//soln=2.161169
vs.push_back(new Variable(Initial(1.061545),Desired(4.048542)));//soln=4.048542
vs.push_back(new Variable(Initial(1.372616),Desired(2.417464)));//soln=2.417464
vs.push_back(new Variable(Initial(1.452004),Desired(2.779394)));//soln=2.779394
vs.push_back(new Variable(Initial(1.891515),Desired(1.840865)));//soln=3.184411
cs.push_back(new Constraint(vs[0],vs[4],1.023242));
cs.push_back(new Constraint(vs[2],vs[3],0.025524));
cs.push_back(new Constraint(vs[3],vs[4],0.399988));
}

/**
 * redundant constraints
 */
void t4(string& name, Variables& vs, Constraints &cs) {
name = "t4";
vs.push_back(new Variable(Initial(0.875029),Desired(2.370362)));//soln=1.928746 != rslt=0.875029
vs.push_back(new Variable(Initial(1.146413),Desired(1.889098)));//soln=2.007311 != rslt=1.146413
vs.push_back(new Variable(Initial(1.904400),Desired(2.402919)));//soln=2.726321 != rslt=1.904400
cs.push_back(new Constraint(vs[0],vs[2],0.198559));
cs.push_back(new Constraint(vs[1],vs[2],0.719010));
cs.push_back(new Constraint(vs[0],vs[1],0.078566));
cs.push_back(new Constraint(vs[0],vs[1],0.076233));
}
void t5(string& name, Variables& vs, Constraints &cs) {
name = "t5";
vs.push_back(new Variable(0,Initial(190),Desired(190),Weight(1e+10)));
vs.push_back(new Variable(1,Initial(296),Desired(296),Weight(1)));
vs.push_back(new Variable(2,Initial(241),Desired(241),Weight(1)));
vs.push_back(new Variable(3,Initial(196),Desired(196),Weight(1)));
vs.push_back(new Variable(4,Initial(88),Desired(88),Weight(1)));
vs.push_back(new Variable(5,Initial(152),Desired(152),Weight(1)));
vs.push_back(new Variable(6,Initial(122),Desired(122),Weight(1)));
vs.push_back(new Variable(7,Initial(196),Desired(196),Weight(1)));
vs.push_back(new Variable(8,Initial(243),Desired(243),Weight(1)));
vs.push_back(new Variable(9,Initial(156),Desired(156),Weight(1)));
Constraint(vs[6],vs[9],34);
Constraint(vs[9],vs[0],34);
Constraint(vs[4],vs[6],34);
Constraint(vs[6],vs[0],34);
Constraint(vs[4],vs[0],34);
Constraint(vs[0],vs[8],34);
Constraint(vs[4],vs[5],34);
Constraint(vs[8],vs[1],34);
Constraint(vs[7],vs[8],34);
Constraint(vs[5],vs[7],34);
Constraint(vs[7],vs[2],34);
Constraint(vs[5],vs[3],34);
Constraint(vs[3],vs[2],34);
}

int main() {
    test(t1);
    test(t2);
    test(t3);
    test(t4);
    test(t5);
    return 0;
}
/*
 * vim: set cindent 
 * vim: ts=4 sw=4 et tw=0 wm=0
 */
