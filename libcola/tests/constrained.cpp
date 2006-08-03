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

#include <vector>
#include <algorithm>
#include <float.h>
#include <libcola/cola.h>
#include "graphlayouttest.h"

using namespace std;
using namespace cola;
/** 
* \brief Determines when to terminate layout of a particular graph based
* on a given relative tolerance. 
*/
int main() {

	const int V = 4;
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
	ConstrainedMajorizationLayout alg(rs,es,width/2);
	AlignmentConstraints acsx;
	AlignmentConstraint ac(1);
	acsx.push_back(&ac);
	ac.offsets.push_back(make_pair((unsigned)0,(double)0));
	ac.offsets.push_back(make_pair((unsigned)3,(double)0));
	alg.setupConstraints(&acsx,NULL,false);
	alg.run();
	assert(fabs(rs[0]->getCentreX()-rs[3]->getCentreX())<0.001);
	cout<<rs[0]->getCentreX()<<","<<rs[3]->getCentreX()<<endl;
	output_svg(rs,es,"constrained.svg");
	for(unsigned i=0;i<V;i++) {
		delete rs[i];
	}
}
