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
#include <algorithm>
#include <float.h>
#include <iomanip>
#include "graphlayouttest.h"
using namespace std;

using namespace cola;
/** 
* \brief Determines when to terminate layout of a particular graph based
* on a given relative tolerance. 
*/
struct check : TestConvergence {
	check(double d,unsigned i) : TestConvergence(d,i) {}
	bool operator()(double new_stress, double* X, double* Y) {
		cout << "*";
		return TestConvergence::operator()(new_stress,X,Y);
	}
};
int main() {

	const int V = 4;
	typedef pair < unsigned, unsigned >Edge;
	Edge edge_array[] = { Edge(0, 1), Edge(1, 2), Edge(2, 3), Edge(1, 3) };
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
	check test(0.0001,100);
	ConstrainedMajorizationLayout alg(rs,es,eweights,width/2,test);
	alg.run();
	output_svg(rs,es,"unconstrained.svg");
	for(unsigned i=0;i<V;i++) {
		delete rs[i];
	}
}
