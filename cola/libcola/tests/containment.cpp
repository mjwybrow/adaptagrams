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
RectangularCluster rc, rd;
RootCluster root;
unsigned iteration=0;

/** 
* \brief Determines when to terminate layout of a particular graph based
* on a given relative tolerance. 
*/
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
	ConstrainedMajorizationLayout alg(rs,es,&root,30,NULL,test);
    alg.setScaling(false);
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
}
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=4:softtabstop=4:encoding=utf-8:textwidth=99 :
