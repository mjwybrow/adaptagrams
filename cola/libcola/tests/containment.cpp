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

/** \file containment.cpp
 *
 * simple test of clustered graph layout
 *
 * Authors:
 *   Tim Dwyer <tgdwyer@gmail.com>
 */
#include <iostream>

#include <map>
#include <vector>
#include <algorithm>
#include <float.h>
#include <iomanip>
#include <libcola/cola.h>
#include <libcola/output_svg.h>
#include "graphlayouttest.h"

using namespace cola;
using namespace std;

vector<vpsc::Rectangle*> rs;
vector<Edge> es;
RectangularCluster rc, rd;
RootCluster root;
unsigned iteration=0;

int main() {

	const unsigned V = 5;
	typedef pair < unsigned, unsigned >Edge;
	Edge edge_array[] = { Edge(0, 1), Edge(1, 2), Edge(2, 3), Edge(3, 4) };
	const std::size_t E = sizeof(edge_array) / sizeof(Edge);
	es.resize(E);
	copy(edge_array,edge_array+E,es.begin());
	double width=100;
	double height=100;
	for(unsigned i=0;i<V;i++) {
		double x=getRand(width), y=getRand(height);
		rs.push_back(new vpsc::Rectangle(x,x+20,y,y+15));
	}

	const unsigned c[]={0,4}, d[]={1,2,3};
	unsigned nc=sizeof(c)/sizeof(unsigned), nd=sizeof(d)/sizeof(unsigned);
	CheckProgress test(0.0001,100);
	ConstrainedMajorizationLayout alg(rs,es,&root,30,nullptr,test);
    alg.setScaling(true);
	rc.nodes.resize(nc);
	copy(c,c+nc,rc.nodes.begin());
	rd.nodes.resize(nd);
	copy(d,d+nd,rd.nodes.begin());
	root.clusters.push_back(&rc);
	root.clusters.push_back(&rd);
	alg.run();
	alg.setAvoidOverlaps();
	alg.run();
    OutputFile of(rs,es,&root,"containment.svg",false,true);
    of.generate();
	for(unsigned i=0;i<V;i++) {
		delete rs[i];
	}
    return 0;
}
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=4:softtabstop=4:textwidth=99 :
