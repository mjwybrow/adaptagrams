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

#include <cola.h>
#include <map>
#include <vector>
#include <algorithm>
#include <float.h>
#include <iomanip>
#include "graphlayouttest.h"

using namespace cola;
using namespace std;

/** 
* \brief Determines when to terminate layout of a particular graph based
* on a given relative tolerance. 
*/
int main() {

	const int V = 5;
	typedef pair < unsigned, unsigned >Edge;
	Edge edge_array[] = { Edge(0, 1), Edge(1, 2), Edge(2, 3), Edge(3, 4) };
	const std::size_t E = sizeof(edge_array) / sizeof(Edge);
	double eweights[E];
	fill(eweights,eweights+E,1);
	vector<Edge> es(E);
	copy(edge_array,edge_array+E,es.begin());
	double width=100;
	double height=100;
	vector<Rectangle*> rs;
	for(unsigned i=0;i<V;i++) {
		double x=getRand(width), y=getRand(height);
		rs.push_back(new Rectangle(x,x+5,y,y+5));
	}

	Cluster c;
	c.push_back(0);
	c.push_back(4);
	Clusters cs;
	cs.push_back(&c);
	ConstrainedMajorizationLayout alg(rs,es,eweights,width/2);
	alg.setupConstraints(NULL,NULL,false,NULL,NULL,NULL,NULL,&cs);
	alg.run();
	output_svg(rs,es,"containment.svg");
	for(unsigned i=0;i<V;i++) {
		delete rs[i];
	}
}
