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
    unsigned V=100;
    CompoundConstraints ccs;
    vector<Edge> es = random_graph(V);
    double defaultEdgeLength=40;
    for(unsigned i=0;i<es.size();i++) {
        unsigned start=es[i].first, end=es[i].second;
        ccs.push_back(
            new SeparationConstraint(vpsc::YDIM, start,end,-10));
    }

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
    run_test(startpos,es,defaultEdgeLength,cx,cy,UGP,true,"random", "cugp");
    run_test(startpos,es,defaultEdgeLength,cx,cy,SGP,true,"random", "csgp");
    */
	vector<vpsc::Rectangle*> rs;
	for(unsigned i=0;i<V;i++) {
		double x=getRand(width), y=getRand(height);
		rs.push_back(new vpsc::Rectangle(x,x+5,y,y+5));
	}
	CheckProgress test(0.0001,200);
    ConstrainedMajorizationLayout alg(rs,es,nullptr,defaultEdgeLength,
            StandardEdgeLengths,&test);
    //alg.setYConstraints(&cy);
	alg.run();
	ConstrainedFDLayout alg2(rs,es,defaultEdgeLength,
            StandardEdgeLengths, &test);
    //alg2.setYConstraints(&cy);
	alg2.run();
	OutputFile output(rs,es,nullptr,"random.pdf");
	output.generate();
	for(unsigned i=0;i<V;i++) {
		delete rs[i];
	}
    return 0;
}
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=4:softtabstop=4:textwidth=99 :
