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

using namespace std;
using namespace cola;

void makeEdge(unsigned u, unsigned v, 
        vector<Edge> &edges, CompoundConstraints &cy) {
    edges.push_back(make_pair(u,v));
    cy.push_back(new SeparationConstraint(u,v,20));
}
vector<Edge> random_tree(unsigned depth, unsigned maxbranch, unsigned &V,
        CompoundConstraints &cx, CompoundConstraints &cy) {
	vector<Edge> edges;
    unsigned lstart=0, lend=1;
    V=0;
	for(unsigned i=0;i<depth;i++) {
        for(unsigned j=lstart;j<lend;j++) {
            //makeEdge(j,++V,edges,cy);
            //makeEdge(j,++V,edges,cy);
            for(unsigned k=0;k<maxbranch;k++) {
                double r=(double)rand()/(double)RAND_MAX;
                if(r < 0.5) {
                    makeEdge(j,++V,edges,cy);
                }
            }
        }
        lstart=lend;
        lend=V+1;
    }
    V++;
    DFS::Graph dfs(V,edges);
    for(unsigned i=1;i<dfs.order.size();i++) {
        cx.push_back(
                new SeparationConstraint(dfs.order[i-1],dfs.order[i],0.5));
    }
    /*
    for(unsigned i=0;i<dfs.leaves.size();i++) {
        for(unsigned j=1;j<dfs.leaves[i].size();j++) {
            cx.push_back(
                    new SeparationConstraint(dfs.leaves[i][j-1],dfs.leaves[i][j],10));
        }
    }
    */
	return edges;
}
int main() {
    unsigned V;
    CompoundConstraints cx,cy;
    //srand(time(nullptr));
    srand(3);
    vector<Edge> es = random_tree(7,4,V,cx,cy);
    double defaultEdgeLength=40;

    cout << "V="<<V<<endl;
    double width=1000;
    double height=1000;
    vector<pair<double,double> > startpos(V);
    for(unsigned i=0;i<V;i++) {
        double x=getRand(width), y=getRand(height);
        startpos[i]=make_pair(x,y);
    }
    const char *testname="trees";
    run_test(startpos,es,defaultEdgeLength,cx,cy,CG,false,testname,"cg");
    run_test(startpos,es,defaultEdgeLength,cx,cy,IP,false,testname,"ip");
    run_test(startpos,es,defaultEdgeLength,cx,cy,SGP,false,testname,"sgp");
    run_test(startpos,es,defaultEdgeLength,cx,cy,UGP,false,testname,"ugp");
    run_test(startpos,es,defaultEdgeLength,cx,cy,IP,true,testname,"cip");
    run_test(startpos,es,defaultEdgeLength,cx,cy,SGP,true,testname,"csgp");
    run_test(startpos,es,defaultEdgeLength,cx,cy,UGP,true,testname,"cugp");
    return 0;
}
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=4:softtabstop=4:textwidth=99 :
