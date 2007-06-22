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
	CheckProgress test(0.0001,100);
	ConstrainedMajorizationLayout alg(rs,es,NULL,width/2,NULL,test);
	alg.setConstrainedLayout(true);
	alg.run();
	OutputFile output(rs,es,NULL,"unconstrained.svg");
	output.generate();
	for(unsigned i=0;i<V;i++) {
		delete rs[i];
	}
}
