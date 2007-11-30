/** \file straightener.cpp
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
	alg.setStraightenEdges(&routes,1.);
	alg.run(true,true);
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
void simplest(bool straighten) {
    printf("test: simplest(): two nodes, one edge\n");
	const unsigned V = 2;
    const char * ls[]={"0","1"};
    vector<const char *> labels(ls,ls+V);
	Edge edge_array[] = { Edge(0,1) };
	const size_t E = sizeof(edge_array) / sizeof(Edge);
	vector<Edge> es(edge_array,edge_array+E);

	vector<vpsc::Rectangle*> rs;
	addRect(rs,270,0,60,40);
	addRect(rs,270,100,60,40);

	vector<straightener::Edge*> routes(E);
    straightener::Route* r=new straightener::Route(2); 
    r->xs[0]=rs[0]->getCentreX();
    r->ys[0]=rs[0]->getCentreY();
    r->xs[1]=rs[1]->getCentreX();
    r->ys[1]=rs[1]->getCentreY();
    routes[0]=new straightener::Edge(0,0,1,r);
	// now straighten the edges
	//ConstrainedMajorizationLayout alg(rs,es,NULL,70);
    Lock lock(0,270,0);
    Locks locks;
    locks.push_back(lock);
    PreIteration preIteration(locks);
    TestConvergence test(0.01,1);
	ConstrainedFDLayout alg(rs,es,NULL,70,NULL,test,&preIteration);
    if(straighten) {
	    alg.setStraightenEdges(&routes,1);
    }
	alg.run(true,true);
    OutputFile of(rs,es,NULL,"straightener-simplest.svg",true,false);
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
void severeBend() {
    printf("test: severeBend()\n");
	const unsigned V = 3;
    const char * ls[]={"0","1","2"};
    vector<const char *> labels(ls,ls+V);
	Edge edge_array[] = { Edge(1,2) };
	const size_t E = sizeof(edge_array) / sizeof(Edge);
	vector<Edge> es(edge_array,edge_array+E);

	vector<vpsc::Rectangle*> rs;
	addRect(rs,0,50,600,40);
	addRect(rs,270,0,60,40);
	addRect(rs,270,100,60,40);

	vector<straightener::Edge*> routes(E);
    /*
    straightener::Route* r=new straightener::Route(2); 
    r->xs[0]=rs[0]->getCentreX();
    r->ys[0]=rs[0]->getCentreY();
    r->xs[1]=rs[1]->getCentreX();
    r->ys[1]=rs[1]->getCentreY();
    routes[0]=new straightener::Edge(0,0,1,r);
    */
    straightener::Route* r=new straightener::Route(4); 
    r->xs[0]=rs[1]->getCentreX();
    r->ys[0]=rs[1]->getCentreY();
    r->xs[1]=rs[0]->getMinX();
    r->ys[1]=rs[0]->getMinY();
    r->xs[2]=rs[0]->getMinX();
    r->ys[2]=rs[0]->getMaxY();
    r->xs[3]=rs[2]->getCentreX();
    r->ys[3]=rs[2]->getCentreY();
    routes[0]=new straightener::Edge(0,1,2,r);
	// now straighten the edges
	//ConstrainedMajorizationLayout alg(rs,es,NULL,70);
    TestConvergence test(0.01,1);
	ConstrainedFDLayout alg(rs,es,NULL,70,NULL,test);
	alg.setStraightenEdges(&routes,1);
	alg.run(true,true);
    OutputFile of(rs,es,NULL,"straightener-severeBend.svg",true,false);
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
void anotherBend() {
    printf("test: anotherBend()\n");
	const unsigned V = 3;
    const char * ls[]={"0","1","2"};
    vector<const char *> labels(ls,ls+V);
	Edge edge_array[] = { Edge(1,2) };
	const size_t E = sizeof(edge_array) / sizeof(Edge);
	vector<Edge> es(edge_array,edge_array+E);

	vector<vpsc::Rectangle*> rs;
    addRect(rs,571.500000,363.500000,63.000000,43.000000);
    addRect(rs,592.148117,469.500000,63.000000,43.000000);
    addRect(rs,541.500000,300.500000,63.000000,43.000000);
    double xs[]={623.648,634.6,634.6,573};
    double ys[]={491,407.5,364.5,322};

    const size_t rn=sizeof(xs)/sizeof(double);

	vector<straightener::Edge*> routes(E);
    straightener::Route* r=new straightener::Route(rn); 
    copy(xs,xs+rn,r->xs);
    copy(ys,ys+rn,r->ys);
    routes[0]=new straightener::Edge(0,1,2,r);
	// now straighten the edges
	//ConstrainedMajorizationLayout alg(rs,es,NULL,70);
    TestConvergence test(0.01,1);
	ConstrainedFDLayout alg(rs,es,NULL,205.786,NULL,test);
	alg.setStraightenEdges(&routes,1);
	alg.run(true,false);
    OutputFile of(rs,es,NULL,"straightener-anotherBend.svg",true,false);
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
void smallTrap() {
    printf("test: smallTrap()\n");
	const unsigned V = 4;
    const char * ls[]={"0","1","2","3"};
    vector<const char *> labels(ls,ls+V);
	Edge edge_array[] = { Edge(1,2) };
	const size_t E = sizeof(edge_array) / sizeof(Edge);
	vector<Edge> es(edge_array,edge_array+E);

	vector<vpsc::Rectangle*> rs;
    addRect(rs,363.500000,220.500000,63.000000,43.000000);
    addRect(rs,377.500000,177.500000,63.000000,43.000000);
    addRect(rs,403.500000,278.500000,63.000000,43.000000);
    addRect(rs,340.500000,263.500000,63.000000,43.000000);

	vector<straightener::Edge*> routes(E);
    /*
    straightener::Route* r=new straightener::Route(2); 
    r->xs[0]=rs[0]->getCentreX();
    r->ys[0]=rs[0]->getCentreY();
    r->xs[1]=rs[1]->getCentreX();
    r->ys[1]=rs[1]->getCentreY();
    routes[0]=new straightener::Edge(0,0,1,r);
    */
    straightener::Route* r=new straightener::Route(4); 
    r->xs[0]=rs[1]->getCentreX();
    r->ys[0]=rs[1]->getCentreY();
    r->xs[1]=rs[0]->getMinX();
    r->ys[1]=rs[0]->getMinY();
    r->xs[2]=rs[0]->getMinX();
    r->ys[2]=rs[0]->getMaxY();
    r->xs[3]=rs[2]->getCentreX();
    r->ys[3]=rs[2]->getCentreY();
    routes[0]=new straightener::Edge(0,1,2,r);
	// now straighten the edges
	//ConstrainedMajorizationLayout alg(rs,es,NULL,70);
    TestConvergence test(0.01,1);
	ConstrainedFDLayout alg(rs,es,NULL,70,NULL,test);
	alg.setStraightenEdges(&routes,1);
	alg.run(true,false);
    OutputFile of(rs,es,NULL,"straightener-smallTrap.svg",true,false);
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
int main() {
	//k5();
	//k6();
    //smallTrap();
	//severeBend();
    //simplest(false);
    //simplest(true);
    anotherBend();
}
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=4:softtabstop=4:encoding=utf-8:textwidth=99 :
