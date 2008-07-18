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
