/*
 * vim: ts=4 sw=4 et tw=0 wm=0
 *
 * libcola - A library providing force-directed network layout using the 
 *           stress-majorization method subject to separation constraints.
 *
 * Copyright (C) 2006-2008  Monash University
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


/** \file
 * Interface between Inkscape code (SPItem) and graphlayout functions.
 *
 * Authors:
 *   Tim Dwyer <tgdwyer@gmail.com>
 */
#include <iostream>

#include <vector>
#include <algorithm>
#include <float.h>
#include <libcola/cola.h>
#include <libvpsc/exceptions.h>
#include <libvpsc/constraint.h>
#include "graphlayouttest.h"

using namespace std;
using namespace cola;
int main() {

	const unsigned V = 2;
	typedef pair < unsigned, unsigned >Edge;
	Edge edge_array[] = { Edge(0, 1) };
	unsigned E = sizeof(edge_array) / sizeof(Edge);
	vector<Edge> es(edge_array,edge_array+E);
	double width=100;
	double height=100;
	vector<vpsc::Rectangle*> rs;
	for(unsigned i=0;i<V;i++) {
		double x=getRand(width), y=getRand(height);
		rs.push_back(new vpsc::Rectangle(x,x+5,y,y+5));
	}
	ConstrainedFDLayout alg(rs,es,width/2);
	CompoundConstraints ccs;
	AlignmentConstraint ac(vpsc::YDIM, 1);
	ccs.push_back(&ac);
	ac.addShape(0,0);
	ac.addShape(1,0);
	ccs.push_back(new SeparationConstraint(vpsc::YDIM, 0,1,10));
	alg.setConstraints(ccs);
    try {
	    alg.run();
    } catch (vpsc::UnsatisfiableException& e) {
        cerr << "Unsatisfiable" << endl;
		for(vector<vpsc::Constraint*>::iterator i=e.path.begin();
				i!=e.path.end();i++) {
			cout << **i << endl;
		}
		exit(1);
    }
	//assert(fabs(rs[0]->getCentreX()-rs[3]->getCentreX())<0.001);
	cout<<rs[0]->getCentreX()<<","<<rs[1]->getCentreX()<<endl;
	//output_svg(rs,es,"unsatisfiable.svg");
	for(unsigned i=0;i<V;i++) {
		delete rs[i];
	}
    return 0;
}
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=4:softtabstop=4:textwidth=99 :
