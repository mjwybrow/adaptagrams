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
//    no constraints - conjugate gradient solver: 21.7 seconds
//    dir-edge constraints - conj grad. solver: 155.69 seconds
#include<iostream>
#include<iomanip>
#include<fstream>
#include<vector>
#include<valarray>
#include<libcola/output_svg.h>
#include<graphlayouttest.h>

using namespace std;
using namespace cola;

struct CheckProgress : TestConvergence {
    CheckProgress(const double d,const unsigned i) : TestConvergence(d,i) {}
    bool operator()(const double new_stress, valarray<double> & X, valarray<double> & Y) {
        cout << "stress="<<new_stress<<endl;
        return TestConvergence::operator()(new_stress,X,Y);
    }
};
int main() {
    const char *fname="data/wheel.txt"; //"data/dg_850.txt";
    ifstream f(fname);
    if(!f.is_open()) {
        cout << "Error opening file: " << fname << endl;
        exit(1);
    }
    string startlabel, endlabel;
    unsigned V = 0;
    double defaultEdgeLength=40;
    vector<Edge> es;
    CompoundConstraints cy;
    //CompoundConstraints cx;
    while(!getline(f,startlabel,' ').eof()) {
        getline(f,endlabel);
        unsigned start = atoi(startlabel.c_str()),
             end = atoi(endlabel.c_str());
        es.push_back(make_pair(start,end));
        //cx.push_back(
            //new SeparationConstraint(start,end,defaultEdgeLength/3));
        cy.push_back(
            new SeparationConstraint(start,end,defaultEdgeLength/3));
        V=max(V,max(start,end));
    }
    V++;
    cout << "V="<<V<<endl;
    double width=1000;
    double height=1000;
    vector<vpsc::Rectangle*> rs;
    //srand(time(nullptr));
    for(unsigned i=0;i<V;i++) {
        double x=getRand(width), y=getRand(height);
        rs.push_back(new vpsc::Rectangle(x,x+1,y,y+1));
    }
    CheckProgress test(0.001,100);
    clock_t starttime=clock();
    ConstrainedMajorizationLayout alg(rs,es,nullptr,defaultEdgeLength,nullptr,test);
    bool constrained=false;
    if(!constrained) {
        cout << "Unconstrained layout" << endl;
        alg.setConstrainedLayout(true);
        alg.run();
    } else {
        cout << "Constrained layout" << endl;
        //alg.setXConstraints(&cx);
        alg.setYConstraints(&cy);
        alg.run();
    }
    double t=double(clock()-starttime)/double(CLOCKS_PER_SEC);
    cout<<"Time="<<t<<endl;
    output_svg(rs,es,nullptr,"small_graph.svg",true);
    for(unsigned i=0;i<V;i++) {
        delete rs[i];
    }
    return 0;
}
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=4:softtabstop=4:textwidth=99 :
