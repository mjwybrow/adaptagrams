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

// Loads a graph from a text file, generates constraints for each edge requiring them
// to point downwards, and proceeds to produce a layout using libcola.
// The input file format is 2 numeric node labels per line separated by a space,
// each pair representing a directed edge.  Node labels are simply used as array
// offsets so they should start from 0.
// The graph should be connected.
// Default input file is the matrix market 1138_bus graph.
// Running times:
//    no constraints - steepest descent solver: 149 seconds
//    no constraints - mosek: 49.47 seconds
//    no constraints - conjugate gradient solver: 12.2 seconds
//    dir-edge constraints - mosek: 214.2 seconds
//    
//    V=1138
#include<iostream>
#include<iomanip>
#include<fstream>
#include<vector>
#include<valarray>
#include <libvpsc/linesegment.h>
#include "graphlayouttest.h"

using namespace std;
using namespace cola;

int countCrossings(vpsc::Rectangles& rs, vector<cola::Edge>& es) {
    int crossings=0;
    for(unsigned i=0;i<es.size()-1;++i) {
        for(unsigned j=i+1;j<es.size();++j) {
            Edge e1=es[i], e2=es[j];
            Rectangle *e1a=rs[e1.first], 
                      *e1b=rs[e1.second], 
                      *e2a=rs[e2.first], 
                      *e2b=rs[e2.second];
            linesegment::LineSegment s1(
                    linesegment::Vector(e1a->getCentreX(),e1a->getCentreY()),
                    linesegment::Vector(e1b->getCentreX(),e1b->getCentreY()));
            linesegment::LineSegment s2(
                    linesegment::Vector(e2a->getCentreX(),e2a->getCentreY()),
                    linesegment::Vector(e2b->getCentreX(),e2b->getCentreY()));
            linesegment::Vector point;
            if(s1.Intersect(s2,point)==linesegment::LineSegment::INTERSECTING) {
                ++crossings;
            }
        }
    }
    return crossings;
}
int main() {
    const char *fname="data/1138_bus.txt"; //"data/dg_850.txt";
    ifstream f(fname);
    if(!f.is_open()) {
        cout << "Error opening file: " << fname << endl;
        exit(1);
    }
    string startlabel, endlabel;
    unsigned V = 0;
    double defaultEdgeLength=40;
    vector<Edge> es;
    CompoundConstraints cx,cy;
    while(!getline(f,startlabel,' ').eof()) {
        getline(f,endlabel);
        unsigned start = atoi(startlabel.c_str()),
             end = atoi(endlabel.c_str());
        es.push_back(make_pair(start,end));
        cy.push_back(
            new SeparationConstraint(end,start,defaultEdgeLength/3));
        V=max(V,max(start,end));
    }
    V++;
    /*
    DFS::Graph dfs(V,es);
    for(unsigned i=1;i<dfs.order.size();i++) {
        cx.push_back(
                new SeparationConstraint(dfs.order[i-1],dfs.order[i],0));
    }
    */
    cout << "V="<<V<<endl;
    double width=1000;
    double height=1000;
    vector<pair<double,double> > startpos(V);
    //srand(time(nullptr));
    for(unsigned i=0;i<V;i++) {
        double x=getRand(width), y=getRand(height);
        startpos[i]=make_pair(x,y);
    }
    //const char *testname="large_graph";
    //run_test(startpos,es,defaultEdgeLength,cx,cy,CG,false,testname,"cg");
    //run_test(startpos,es,defaultEdgeLength,cx,cy,IP,false,testname,"ip");
    //run_test(startpos,es,defaultEdgeLength,cx,cy,UGP,false,testname,"ugp");
    //run_test(startpos,es,defaultEdgeLength,cx,cy,SGP,false,testname,"sgp");
    //run_test(startpos,es,defaultEdgeLength,cx,cy,IP,true,testname,"cip");
    //run_test(startpos,es,defaultEdgeLength,cx,cy,UGP,true,testname,"cugp");
    //run_test(startpos,es,defaultEdgeLength,cx,cy,SGP,true,testname,"csgp");
	vector<vpsc::Rectangle*> rs;
	for(unsigned i=0;i<V;i++) {
		double x=getRand(width), y=getRand(height);
		rs.push_back(new vpsc::Rectangle(x,x+5,y,y+5));
	}
    cout<<"Initial crossings="<<countCrossings(rs,es)<<endl;
	CheckProgress test(0.0001,200);
    ConstrainedMajorizationLayout alg(rs,es,nullptr,defaultEdgeLength,nullptr,test);
    alg.setYConstraints(&cy);
	alg.run();
    cout<<"Majorization crossings="<<countCrossings(rs,es)<<endl;
	OutputFile output1(rs,es,nullptr,"large_graph-majorization.pdf");
    output1.generate();
	ConstrainedFDLayout alg2(rs,es,defaultEdgeLength,nullptr,test);
    alg2.setYConstraints(&cy);
	alg2.run();
    cout<<"PStress crossings="<<countCrossings(rs,es)<<endl;
	OutputFile output(rs,es,nullptr,"large_graph.pdf");
	output.generate();
	for(unsigned i=0;i<V;i++) {
		delete rs[i];
	}
    return 0;
}
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=4:softtabstop=4:textwidth=99 :
