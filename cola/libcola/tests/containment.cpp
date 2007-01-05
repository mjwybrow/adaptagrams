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

vector<vpsc::Rectangle*> rs;
vector<Edge> es;
RootCluster root;
unsigned iteration=0;

struct CheckProgress : TestConvergence {
	CheckProgress(double d,unsigned i) : TestConvergence(d,i) {}
	bool operator()(double new_stress, valarray<double> & X, valarray<double> & Y) {
		cout << "stress="<<new_stress<<endl;
		char fname[50];
		sprintf(fname,"containment1%04d.svg",++iteration);
		output_svg(rs,es,root,fname,true);
		return TestConvergence::operator()(new_stress,X,Y);
	}
};
/** 
* \brief Determines when to terminate layout of a particular graph based
* on a given relative tolerance. 
*/
int main() {

	const int V = 5;
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
	RectangularCluster rc, rd;
	rc.nodes.resize(nc);
	copy(c,c+nc,rc.nodes.begin());
	rd.nodes.resize(nd);
	copy(d,d+nd,rd.nodes.begin());
	root.clusters.push_back(&rc);
	root.clusters.push_back(&rd);
	CheckProgress test(0.0001,100);
	output_svg(rs,es,root,"containment10000.svg",true);
	ConstrainedMajorizationLayout alg(rs,es,&root,30,NULL,test);
	alg.run();
	alg.setNonOverlappingClusters();
	alg.run();
	for(unsigned i=0;i<V;i++) {
		delete rs[i];
	}
}
