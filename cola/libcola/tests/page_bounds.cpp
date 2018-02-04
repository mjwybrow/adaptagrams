/*
 * vim: ts=4 sw=4 et tw=0 wm=0
 *
 * libcola - A library providing force-directed network layout using the 
 *           stress-majorization method subject to separation constraints.
 *
 * Copyright (C) 2006-2008  Monash University
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library in the file LICENSE; if not, 
 * write to the Free Software Foundation, Inc., 59 Temple Place, 
 * Suite 330, Boston, MA  02111-1307  USA
 *
*/

/**
 * \file page_bounds.cpp
 *
 * test/example showing the use of page boundary constraints
 */
#include<iostream>
#include<vector>
#include <cmath>
#include <time.h>
#include <valarray>

#include "graphlayouttest.h"

vector<Edge> random_graph(unsigned n) {
	vector<Edge> edges;
	for(unsigned i=1;i<n;i++) {
        edges.push_back(make_pair(i-1,i));
    }
	for(unsigned i=0;i<n;i++) {
		for(unsigned j=i+1;j<n;j++) {
			double r=(double)rand()/(double)RAND_MAX;
			if(r < 1./(double)n) {
				edges.push_back(make_pair(i,j));
			}
		}
	}

	return edges;
}
int main() {
    unsigned V=30;
    CompoundConstraints ccs;
    vector<Edge> es = random_graph(V);
    double defaultEdgeLength=40;
    cola::PageBoundaryConstraints* pbc =
        new cola::PageBoundaryConstraints(0,200,0,200,100);

    double w=2.0, h=2.0;
    for(unsigned i=0;i<V;i++) {
        pbc->addShape(i, w/2, h/2);
    }
    ccs.push_back(pbc);

    cout << "V="<<V<<endl;
    double width=1000;
    double height=1000;
    //srand(time(nullptr));
    vector<pair<double,double> > startpos(V);
    for(unsigned i=0;i<V;i++) {
        double x=getRand(width), y=getRand(height);
        startpos[i]=make_pair(x,y);
    }

    /*void run_test(
        vector<pair<double,double> > const &startpos,
        vector<Edge> const &es, 
        const double defaultEdgeLength,
        CompoundConstraints &cx,
        CompoundConstraints &cy,
        const SolverType s, 
        const bool constrained,
        const char *fname,
        const char *testdesc) {
    */
    
/*
    run_test(startpos,es,defaultEdgeLength,cx,cy,CG,false,"random","cg");
    run_test(startpos,es,defaultEdgeLength,cx,cy,IP,false,"random", "ip");
    run_test(startpos,es,defaultEdgeLength,cx,cy,UGP,false,"random", "ugp");
    run_test(startpos,es,defaultEdgeLength,cx,cy,SGP,false,"random", "sgp");
    run_test(startpos,es,defaultEdgeLength,cx,cy,IP,true,"random", "cip");
    run_test(startpos,es,defaultEdgeLength,cx,cy,SGP,true,"random", "csgp");
    */
    run_test(startpos,es,defaultEdgeLength,ccs,UGP,true,"random", "cugp");
    return 0;
}
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=4:softtabstop=4:textwidth=99 :
