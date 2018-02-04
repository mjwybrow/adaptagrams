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
RootCluster root;
unsigned iteration=0;

/** 
* \brief Determines when to terminate layout of a particular graph based
* on a given relative tolerance. 
*/
int main() {

	const unsigned V = 19;
	const char* labels[]={
		"Hamza Alghamdi",
		"Nawaf Alhazmi",
		"Marwan Al-Shehhi",
		"Mohand Alshehri",
		"Ahmed Alghamdi",
		"Saeed Alghamdi",
		"Salem Alhazmi",
		"Hani Hanjour",
		"Mohamed Atta",
		"Ziad Jarrah",
		"Ahmed Al Haznawi",
		"Fayez Ahmed",
		"Ahmed Alnami",
		"Khalid Al-Mihdhar",
		"Majed Moqed",
		"Abdul Aziz Al-Omari",
		"Waleed Alshehri",
		"Wail Alshehri",
		"Satam Suqami"};

	typedef pair < unsigned, unsigned >Edge;
	Edge edge_array[] = { 
		Edge(0,1), Edge(0,3), Edge(0,4), Edge(0,5), Edge(0,10),
		Edge(1,5), Edge(1,6), Edge(1,7), Edge(1,12), Edge(1,13),
		Edge(2,8), Edge(2,9), Edge(2,11), Edge(2,15),
		Edge(3,11),
		Edge(5,10), Edge(5,12), 
		Edge(7,13), Edge(7,14),
		Edge(8,9), Edge(8,15),
		Edge(9,10), 
		Edge(15,16),
		Edge(16,17), Edge(16,18),
       		Edge(17,18) 
	};
	double g=10;
	CompoundConstraints scy;
	scy.push_back(new SeparationConstraint(0,10,g));
	scy.push_back(new SeparationConstraint(0,3,g));
	scy.push_back(new SeparationConstraint(0,4,g));
	scy.push_back(new SeparationConstraint(0,12,g));
	scy.push_back(new SeparationConstraint(0,5,g));
	scy.push_back(new SeparationConstraint(1,12,g));
	scy.push_back(new SeparationConstraint(1,5,g));
	scy.push_back(new SeparationConstraint(1,13,g));
	scy.push_back(new SeparationConstraint(1,6,g));
	scy.push_back(new SeparationConstraint(1,7,g));
	scy.push_back(new SeparationConstraint(2,8,g));
	scy.push_back(new SeparationConstraint(2,9,g));
	scy.push_back(new SeparationConstraint(2,15,g));
	scy.push_back(new SeparationConstraint(2,11,g));
	scy.push_back(new SeparationConstraint(5,12,g));
	scy.push_back(new SeparationConstraint(7,13,g));
	scy.push_back(new SeparationConstraint(7,14,g));
	scy.push_back(new SeparationConstraint(16,17,g));
	scy.push_back(new SeparationConstraint(16,18,g));
	const std::size_t E = sizeof(edge_array) / sizeof(Edge);
	es.resize(E);
	copy(edge_array,edge_array+E,es.begin());
	double width=100;
	double height=100;
	for(unsigned i=0;i<V;i++) {
		double x=getRand(width), y=getRand(height);
		rs.push_back(new vpsc::Rectangle(x,x+17,y,y+10));
	}

	RectangularCluster c;
	c.nodes.push_back(0);
	c.nodes.push_back(1);
	RectangularCluster d;
	d.nodes.push_back(3);
	d.nodes.push_back(11);
	RectangularCluster e;
	e.nodes.push_back(8);
	e.nodes.push_back(9);
	e.nodes.push_back(10);
	e.nodes.push_back(15);
	e.nodes.push_back(16);
	RectangularCluster f;
	f.nodes.push_back(17);
	f.nodes.push_back(18);
	root.clusters.push_back(&c);
	root.clusters.push_back(&d);
	root.clusters.push_back(&e);
	root.clusters.push_back(&f);
    OutputFile of(rs,es,&root,"containment2.svg",true,true);
    //of.setLabels(V,labels);
	CheckProgress test(0.0001,100);
	ConstrainedMajorizationLayout alg(rs,es,&root,30,nullptr,test);
    alg.setScaling(false);
	//alg.setYSeparationConstraints(&scy);
	//alg.run();
	alg.setAvoidOverlaps();
	alg.run();
    of.generate();
	for(unsigned i=0;i<V;i++) {
		delete rs[i];
	}
    return 0;
}
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=4:softtabstop=4:textwidth=99 :
