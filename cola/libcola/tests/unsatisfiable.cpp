
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
//#include <libvspc/exceptions.h>
#include <libvpsc/constraint.h>
#include "graphlayouttest.h"

using namespace std;
using namespace cola;
int main() {

	const int V = 2;
	typedef pair < unsigned, unsigned >Edge;
	Edge edge_array[] = { Edge(0, 1) };
	unsigned E = sizeof(edge_array) / sizeof(Edge);
	vector<Edge> es(edge_array,edge_array+E);
	double width=100;
	double height=100;
	vector<vpsc::Rectangle*> rs;
	for(unsigned i=0;i<V;i++) {
		double x=getRand(width), y=getRand(height);
		rs.push_back(new vpsc::Rectangle(x,x+5,y,y+5));
	}
	ConstrainedMajorizationLayout alg(rs,es,NULL,width/2);
	AlignmentConstraints acsy;
	AlignmentConstraint ac(1);
	acsy.push_back(&ac);
	alg.setYAlignmentConstraints(&acsy);
	ac.offsets.push_back(make_pair((unsigned)0,(double)0));
	ac.offsets.push_back(make_pair((unsigned)1,(double)0));
	SeparationConstraints cs;
	cs.push_back(new SeparationConstraint(0,1,10));
	alg.setYSeparationConstraints(&cs);
    try {
	    alg.run();
    } catch (vpsc::UnsatisfiableException& e) {
        cerr << "Unsatisfiable" << endl;
		for(vector<vpsc::Constraint*>::iterator i=e.path.begin();
				i!=e.path.end();i++) {
			cout << **i << endl;
		}
		exit(1);
    }
	//assert(fabs(rs[0]->getCentreX()-rs[3]->getCentreX())<0.001);
	cout<<rs[0]->getCentreX()<<","<<rs[1]->getCentreX()<<endl;
	output_svg(rs,es,"unsatisfiable.svg");
	for(unsigned i=0;i<V;i++) {
		delete rs[i];
	}
}
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=4:softtabstop=4:encoding=utf-8:textwidth=99 :
