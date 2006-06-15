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

#include <cola.h>
#include <boost/graph/circle_layout.hpp>
#include <boost/graph/kamada_kawai_spring_layout.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graphviz.hpp>
#include <map>
#include <vector>
#include <algorithm>
#include <float.h>
#include <iomanip>
#include "graphlayouttest.h"

using namespace cola;
using namespace std;

static const double w=40, h=40;
void addRect(vector<Rectangle*>& rs, double x, double y) {
	rs.push_back(new Rectangle(x,x+w,y,y+h));
}
int main() {
	// Define K5 graph
	const int V = 5;
	Edge edge_array[] = { Edge(0, 1), Edge(0, 2), Edge(0, 3), Edge(0, 4),
		Edge(1, 2), Edge(1, 3), Edge(1, 4),
		Edge(2, 3), Edge(2, 4),
		Edge(3, 4)
       	};
	const size_t E = sizeof(edge_array) / sizeof(Edge);
	double eweights[E];
	fill(eweights,eweights+E,1);
	vector<Edge> es(E);
	copy(edge_array,edge_array+E,es.begin());

	vector<Rectangle*> rs;
	addRect(rs,79,148);
	addRect(rs,42,25);
	addRect(rs,139,92);
	addRect(rs,77,248);
	addRect(rs,125,200);

	vector<straightener::Edge*> routes(E);
	for(unsigned i=0;i<E;i++) {
		straightener::Route* r=new straightener::Route(2);
		r->xs[0]=rs[es[i].first]->getCentreX();
		r->ys[0]=rs[es[i].first]->getCentreY();
		r->xs[1]=rs[es[i].second]->getCentreX();
		r->ys[1]=rs[es[i].second]->getCentreY();
		routes[i]=new straightener::Edge(i,es[i].first,es[i].second,r);
	}
	// route edges in a crossing-minimal way
	straightener::Route *r=new straightener::Route(4);
	r->xs[0]=rs[2]->getCentreX();
	r->ys[0]=rs[2]->getCentreY();
	r->xs[1]=rs[4]->getMaxX();
	r->ys[1]=rs[4]->getMinY();
	r->xs[2]=rs[4]->getMaxX();
	r->ys[2]=rs[4]->getMaxY();
	r->xs[3]=rs[3]->getCentreX();
	r->ys[3]=rs[3]->getCentreY();
	routes[7]->setRoute(r);
	r=new straightener::Route(3);
	r->xs[0]=rs[1]->getCentreX();
	r->ys[0]=rs[1]->getCentreY();
	r->xs[1]=rs[0]->getMinX();
	r->ys[1]=rs[0]->getMaxY();
	r->xs[2]=rs[3]->getCentreX();
	r->ys[2]=rs[3]->getCentreY();
	routes[5]->setRoute(r);
	r=new straightener::Route(3);
	r->xs[0]=rs[1]->getCentreX();
	r->ys[0]=rs[1]->getCentreY();
	r->xs[1]=rs[0]->getMaxX();
	r->ys[1]=rs[0]->getMinY();
	r->xs[2]=rs[4]->getCentreX();
	r->ys[2]=rs[4]->getCentreY();
	routes[6]->setRoute(r);
	// now straighten the edges
	ConstrainedMajorizationLayout alg(rs,es,eweights,200);
	alg.straighten(routes,HORIZONTAL);
	output_svg(rs,routes,"straightener-x.svg",true);
	alg.straighten(routes,VERTICAL);
	output_svg(rs,routes,"straightener-xy.svg",true);
	for(unsigned i=0;i<V;i++) {
		delete rs[i];
	}
	for(unsigned i=0;i<E;i++) {
		delete routes[i];
	}
}
