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

/** \file unconstrained.cpp
 *
 * Unconstrained graph layout test.  Simple graph with 4 nodes and 4 edges,
 * a triangle with a dangle.  Final stress checked.
 *
 *
 * Authors:
 *   Tim Dwyer <tgdwyer@gmail.com>
 */
#include <iostream>
#include <fstream>

#include <vector>
#include <valarray>
#include <algorithm>
#include <float.h>
#include "graphlayouttest.h"
using namespace std;

using namespace cola;
int main() {

	const unsigned V = 4;
	Edge edge_array[] = { Edge(0, 1), Edge(1, 2), Edge(2, 3), Edge(1, 3) };
	const std::size_t E = sizeof(edge_array) / sizeof(Edge);
	vector<Edge> es(E);
	copy(edge_array,edge_array+E,es.begin());
	double width=100;
	double height=100;
	vector<vpsc::Rectangle*> rs;
	for(unsigned i=0;i<V;i++) {
		double x=getRand(width), y=getRand(height);
		rs.push_back(new vpsc::Rectangle(x,x+5,y,y+5));
	}
	CheckProgress test(1e-9,100);
	ConstrainedFDLayout alg(rs,es,width/2,nullptr,&test);
	alg.run();
	double stress = alg.computeStress();
  	assert(stress < 0.0013);
	OutputFile output(rs,es,nullptr,"unconstrained.svg");
	output.generate();
	for(unsigned i=0;i<V;i++) {
		delete rs[i];
	}
    return 0;
}
