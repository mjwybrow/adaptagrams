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
Clusters cs;
unsigned iteration=0;

struct CheckProgress : TestConvergence {
	CheckProgress(double d,unsigned i) : TestConvergence(d,i) {}
	bool operator()(double new_stress, valarray<double> const & X, valarray<double> const & Y) {
		cout << "stress="<<new_stress<<endl;
		char fname[50];
		sprintf(fname,"containment2%04d.svg",++iteration);
		output_svg(rs,es,cs,fname,true);
		return TestConvergence::operator()(new_stress,X,Y);
	}
};
/** 
* \brief Determines when to terminate layout of a particular graph based
* on a given relative tolerance. 
*/
int main() {

	const unsigned V = 19;
	const char* labels[]={
		"Hamza Alghamdi",
		"Nawaf Alhazmi",
		"Marwan Al-Shehhi",
		"Mohand Alshehri",
		"Ahmed Alghamdi",
		"Saeed Alghamdi",
		"Salem Alhazmi",
		"Hani Hanjour",
		"Mohamed Atta",
		"Ziad Jarrah",
		"Ahmed Al Haznawi",
		"Fayez Ahmed",
		"Ahmed Alnami",
		"Khalid Al-Mihdhar",
		"Majed Moqed",
		"Abdul Aziz Al-Omari",
		"Waleed Alshehri",
		"Wail Alshehri",
		"Satam Suqami"};

	typedef pair < unsigned, unsigned >Edge;
	Edge edge_array[] = { 
		Edge(0,1), Edge(0,3), Edge(0,4), Edge(0,5), Edge(0,10),
		Edge(1,5), Edge(1,6), Edge(1,7), Edge(1,12), Edge(1,13),
		Edge(2,8), Edge(2,9), Edge(2,11), Edge(2,15),
		Edge(3,11),
		Edge(5,10), Edge(5,12), 
		Edge(7,13), Edge(7,14),
		Edge(8,9), Edge(8,15),
		Edge(9,10), 
		Edge(15,16),
		Edge(16,17), Edge(16,18),
       		Edge(17,18) 
	};
	double g=10;
	SimpleConstraints scy;
	scy.push_back(new SimpleConstraint(0,10,g));
	scy.push_back(new SimpleConstraint(0,3,g));
	scy.push_back(new SimpleConstraint(0,4,g));
	scy.push_back(new SimpleConstraint(0,12,g));
	scy.push_back(new SimpleConstraint(0,5,g));
	scy.push_back(new SimpleConstraint(1,12,g));
	scy.push_back(new SimpleConstraint(1,5,g));
	scy.push_back(new SimpleConstraint(1,13,g));
	scy.push_back(new SimpleConstraint(1,6,g));
	scy.push_back(new SimpleConstraint(1,7,g));
	scy.push_back(new SimpleConstraint(2,8,g));
	scy.push_back(new SimpleConstraint(2,9,g));
	scy.push_back(new SimpleConstraint(2,15,g));
	scy.push_back(new SimpleConstraint(2,11,g));
	scy.push_back(new SimpleConstraint(5,12,g));
	scy.push_back(new SimpleConstraint(7,13,g));
	scy.push_back(new SimpleConstraint(7,14,g));
	scy.push_back(new SimpleConstraint(16,17,g));
	scy.push_back(new SimpleConstraint(16,18,g));
	const std::size_t E = sizeof(edge_array) / sizeof(Edge);
	es.resize(E);
	copy(edge_array,edge_array+E,es.begin());
	double width=100;
	double height=100;
	for(unsigned i=0;i<V;i++) {
		double x=getRand(width), y=getRand(height);
		rs.push_back(new vpsc::Rectangle(x,x+17,y,y+10));
	}

	const unsigned c[]={0,1},d[]={3,11},e[]={8,9,10,15,16},f[]={17,18};
        size_t su=sizeof(unsigned);
	cs.push_back(new Cluster(sizeof(c)/su,c));
	cs.push_back(new Cluster(sizeof(d)/su,d));
	cs.push_back(new Cluster(sizeof(e)/su,e));
	cs.push_back(new Cluster(sizeof(f)/su,f));
	CheckProgress test(0.0001,50);
	ConstrainedMajorizationLayout alg(rs,es,&cs,30,NULL,test);
	//alg.setYSimpleConstraints(&scy);
	//alg.run();
	//alg.setAvoidOverlaps();
	output_svg(rs,es,cs,"containment20000.svg",true);
	alg.run();
	for(unsigned i=0;i<V;i++) {
		delete rs[i];
	}
}
