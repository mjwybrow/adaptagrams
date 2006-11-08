/** \file
 * Interface between Inkscape code (SPItem) and graphlayout functions.
 */
/*
* Authors:
*   Tim Dwyer <tgdwyer@gmail.com>
*
* Copyright (C) 2005 Authors
*
* Released under GNU GPL.  Read the file 'COPYING' for more information.
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

/** 
* \brief Determines when to terminate layout of a particular graph based
* on a given relative tolerance. 
*/
int main() {

	const unsigned V = 19;
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
	const std::size_t E = sizeof(edge_array) / sizeof(Edge);
	vector<Edge> es(edge_array,edge_array+E);
	double width=100;
	double height=100;
	vector<vpsc::Rectangle*> rs;
	for(unsigned i=0;i<V;i++) {
		double x=getRand(width), y=getRand(height);
		rs.push_back(new vpsc::Rectangle(x,x+17,y,y+10));
	}

	Cluster c,d,e,f;
	c.push_back(0);
	c.push_back(1);
	d.push_back(3);
	d.push_back(11);
	e.push_back(8);
	e.push_back(9);
	e.push_back(10);
	e.push_back(15);
	e.push_back(16);
	f.push_back(17);
	f.push_back(18);
	Clusters cs;
	cs.push_back(&c);
	cs.push_back(&d);
	cs.push_back(&e);
	cs.push_back(&f);
	ConstrainedMajorizationLayout alg(rs,es,&cs,30);
	alg.run();
	alg.setAvoidOverlaps();
	alg.run();
	output_svg(rs,es,cs,"containment2.svg",true);
	for(unsigned i=0;i<V;i++) {
		delete rs[i];
	}
}
