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

#include <map>
#include <vector>
#include <algorithm>
#include <float.h>
#include <iomanip>
#include "graphlayouttest.h"
#include <libcola/cola.h>
#include <libcola/straightener.h>

using namespace cola;
using namespace std;

void addRect(vector<vpsc::Rectangle*>& rs, double x, double y, double w, double h) {
	rs.push_back(new vpsc::Rectangle(x,x+w,y,y+h));
}
void k5() {
	// Define K5 graph
	const unsigned V = 5;
    const char* ls[]={"0","1","2","3","4"};
    vector<const char*> labels(ls,ls+V);
	Edge edge_array[] = { Edge(0, 1), Edge(0, 2), Edge(0, 3), Edge(0, 4),
		Edge(1, 2), Edge(1, 3), Edge(1, 4),
		Edge(2, 3), Edge(2, 4),
		Edge(3, 4)
       	};
	const size_t E = sizeof(edge_array) / sizeof(Edge);
	vector<Edge> es(edge_array,edge_array+E);

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
    TestConvergence test(0.01,100);
	ConstrainedFDLayout alg(rs,es,NULL,200,NULL,test);
	//ConstrainedMajorizationLayout alg(rs,es,NULL,200,NULL,test);
    //alg.setScaling(false);
	alg.setStraightenEdges(&routes);
	alg.run();
	//alg.straighten(routes,HORIZONTAL);
    //OutputFile of1(rs,es,NULL,"straightener-x.svg",true,false);
    //of1.generate();
	//alg.straighten(routes,VERTICAL);
    OutputFile of(rs,es,NULL,"straightener-k5.svg",true,false);
    of.setLabels(&labels);
    of.routes=&routes;
    of.generate();
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
	const unsigned V = 6;
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
	//ConstrainedMajorizationLayout alg(rs,es,NULL,200);
	ConstrainedFDLayout alg(rs,es,NULL,200);
	alg.setStraightenEdges(&routes,1);
	alg.run();
    OutputFile of(rs,es,NULL,"straightener-k6.svg",true,false);
    of.generate();
	for(unsigned i=0;i<V;i++) {
		delete rs[i];
	}
	for(unsigned i=0;i<E;i++) {
		delete routes[i];
	}
}
void severeBend() {
	const unsigned V = 4;
	Edge edge_array[] = { Edge(1,3), Edge(0,3), Edge(1,0), Edge(1,2), Edge(3,2) };
	const size_t E = sizeof(edge_array) / sizeof(Edge);
	vector<Edge> es(edge_array,edge_array+E);

	vector<vpsc::Rectangle*> rs;
	const double avoidBuffer=4;
	const double w=30+2*avoidBuffer, h=22+2*avoidBuffer;
	addRect(rs,123-avoidBuffer,170-avoidBuffer,w,h);
	addRect(rs,176-avoidBuffer,152-avoidBuffer,w,h);
	addRect(rs,136-avoidBuffer,107-avoidBuffer,w,h);
	addRect(rs,215-avoidBuffer,131-avoidBuffer,w,h);

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
	setRoute(routes,1,3,(double[]){
		138.000000,181.000000,
		172.000000,148.000000,
		230.000000,142.000000
	});
	setRoute(routes,3,5,(double[]){
		191.000000,163.000000,
		158.000000,197.000000,
		119.000000,197.000000,
		119.000000,166.000000,
		151.000000,118.000000
	});
	// now straighten the edges
	//ConstrainedMajorizationLayout alg(rs,es,NULL,70);
	ConstrainedFDLayout alg(rs,es,NULL,70);
	alg.setStraightenEdges(&routes,1);
	alg.run();
    OutputFile of(rs,es,NULL,"straightener-severeBend.svg",true,false);
    of.generate();
	for(unsigned i=0;i<V;i++) {
		delete rs[i];
	}
	for(unsigned i=0;i<E;i++) {
		delete routes[i];
	}
}
int main() {
	k5();
	//k6();
	//severeBend();
}
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=4:softtabstop=4:encoding=utf-8:textwidth=99 :
