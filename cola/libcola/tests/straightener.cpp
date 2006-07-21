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
#include <libcola/cola.h>

using namespace cola;
using namespace std;

void addRect(vector<vpsc::Rectangle*>& rs, double x, double y, double w, double h) {
	rs.push_back(new vpsc::Rectangle(x,x+w,y,y+h));
}
void k5() {
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

	const double w=40, h=40;
	vector<vpsc::Rectangle*> rs;
	addRect(rs,79,148,w,h);
	addRect(rs,42,25,w,h);
	addRect(rs,139,92,w,h);
	addRect(rs,77,248,w,h);
	addRect(rs,125,200,w,h);

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
	TestConvergence test(0.0001,1);
	ConstrainedMajorizationLayout alg(rs,es,eweights,200,test);
	alg.setupConstraints(NULL,NULL,false,NULL,NULL,NULL,NULL,NULL,&routes);
	alg.run();
	//alg.straighten(routes,HORIZONTAL);
	//output_svg(rs,routes,"straightener-x.svg",true);
	//alg.straighten(routes,VERTICAL);
	output_svg(rs,routes,"straightener-xy.svg",true);
	for(unsigned i=0;i<V;i++) {
		delete rs[i];
	}
	for(unsigned i=0;i<E;i++) {
		delete routes[i];
	}
}
void setRoute(vector<straightener::Edge*> routes,unsigned i,
		unsigned n, double ps[])  {
	straightener::Route *r=new straightener::Route(n);
	for(unsigned j=0;j<n;j++) {
		r->xs[j]=ps[2*j];
		r->ys[j]=ps[2*j+1];
	}
	routes[i]->setRoute(r);
}
void k6() {
	// Define K6 graph
	const int V = 6;
	Edge edge_array[] = { 
		Edge(0, 1), Edge(0, 2), Edge(0, 3), Edge(0, 4), Edge(0, 5),
		Edge(1, 2), Edge(1, 3), Edge(1, 4), Edge(1, 5),
		Edge(2, 3), Edge(2, 4), Edge(2, 5),
		Edge(3, 4), Edge(3, 5),
		Edge(4, 5)
       	};
	const size_t E = sizeof(edge_array) / sizeof(Edge);
	double eweights[E];
	fill(eweights,eweights+E,1);
	vector<Edge> es(E);
	copy(edge_array,edge_array+E,es.begin());

	const double w=52, h=32;
	vector<vpsc::Rectangle*> rs;
	addRect(rs,427.000000,372.000000,w,h);
	addRect(rs,372.000000,402.000000,w,h);
	addRect(rs,532.000000,415.000000,w,h);
	addRect(rs,302.000000,407.000000,w,h);
	addRect(rs,314.000000,258.000000,w,h);
	addRect(rs,307.000000,543.000000,w,h);

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
	setRoute(routes,2,3,(double[]){
		453.000000,388.000000,
		368.000000,398.000000,
		328.000000,423.000000});
	setRoute(routes,4,3,(double[]){
		453.000000,388.000000,
		429.000000,439.000000,
		333.000000,559.000000});
	setRoute(routes,9,5,(double[]){
		558.000000,431.000000,
		364.000000,580.000000,
		303.000000,580.000000,
		303.000000,539.000000,
		328.000000,423.000000});
	setRoute(routes,10,3,(double[]){
		558.000000,431.000000,
		484.000000,368.000000,
		340.000000,274.000000});
	setRoute(routes,14,4,(double[]){
		340.000000,274.000000,
		298.000000,403.000000,
		298.000000,444.000000,
		333.000000,559.000000});
	// now straighten the edges
	TestConvergence test(0.0001,1);
	ConstrainedMajorizationLayout alg(rs,es,eweights,200,test);
	alg.setupConstraints(NULL,NULL,false,NULL,NULL,NULL,NULL,NULL,&routes);
	alg.run();
	//alg.straighten(routes,HORIZONTAL);
	//output_svg(rs,routes,"straightener-x.svg",true);
	//alg.straighten(routes,VERTICAL);
	output_svg(rs,routes,"k6-straightened.svg",true);
	for(unsigned i=0;i<V;i++) {
		delete rs[i];
	}
	for(unsigned i=0;i<E;i++) {
		delete routes[i];
	}
}
int main() {
	//k5();
	k6();
}
