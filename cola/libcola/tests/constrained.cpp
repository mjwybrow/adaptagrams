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

/** \file constrained.cpp
 *
 * runs constraint layout on a small graph.  nodes 0 and 3 are constrained
 * to a vertical line
 *
 *
 * Authors:
 *   Tim Dwyer <tgdwyer@gmail.com>
 */
#include <iostream>

#include <vector>
#include <algorithm>
#include <float.h>
#include <libcola/cola.h>
#include <libcola/output_svg.h>
inline double getRand(double range) {
	return range*rand()/RAND_MAX;
}

using namespace std;
using namespace cola;
/** 
* \brief Determines when to terminate layout of a particular graph based
* on a given relative tolerance. 
*/
int main() {

	const unsigned V = 4;
	typedef pair < unsigned, unsigned >Edge;
	Edge edge_array[] = { Edge(0, 1), Edge(1, 2), Edge(2, 3), Edge(1, 3) };
	unsigned E = sizeof(edge_array) / sizeof(Edge);
	vector<Edge> es(edge_array,edge_array+E);
	double width=100;
	double height=100;
	vector<vpsc::Rectangle*> rs;
	for(unsigned i=0;i<V;i++) {
		double x=getRand(width), y=getRand(height);
		rs.push_back(new vpsc::Rectangle(x,x+5,y,y+5));
	}
	CompoundConstraints ccs;
	AlignmentConstraint ac(vpsc::XDIM);
	ccs.push_back(&ac);
	ac.addShape(0,0);
	ac.addShape(3,0);
	// apply steepest descent layout
	ConstrainedFDLayout alg2(rs,es,width/2);
	alg2.setConstraints(ccs);
	alg2.run();
	assert(alg2.computeStress()<0.0013);
	// the following pair of nodes should line-up
	assert(fabs(rs[0]->getCentreX()-rs[3]->getCentreX())<0.001);
	// reset rectangles to random positions
	for(unsigned i=0;i<V;i++) {
		double x=getRand(width), y=getRand(height);
		rs[i]->moveCentre(x,y);
	}
	// apply scaled majorization layout
	ConstrainedMajorizationLayout alg(rs,es,nullptr,width/2);
	alg.setConstraints(&ccs);
	alg.setScaling(true);
	alg.run();
	// the following pair of nodes should line-up
	assert(fabs(rs[0]->getCentreX()-rs[3]->getCentreX())<0.001);
	cout<<rs[0]->getCentreX()<<","<<rs[3]->getCentreX()<<endl;
	OutputFile output(rs,es,nullptr,"constrained.svg");
	output.rects=true;
	output.generate();

	for(unsigned i=0;i<V;i++) {
		delete rs[i];
	}
    return 0;
}
