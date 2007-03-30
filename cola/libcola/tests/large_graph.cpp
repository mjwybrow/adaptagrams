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
#include "graphlayouttest.h"

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
    CompoundConstraints cy;
    while(!getline(f,startlabel,' ').eof()) {
        getline(f,endlabel);
        unsigned start = atoi(startlabel.c_str()),
             end = atoi(endlabel.c_str());
        es.push_back(make_pair(start,end));
        cy.push_back(
            new SeparationConstraint(start,end,defaultEdgeLength/3));
        V=max(V,max(start,end));
    }
    V++;
    cout << "V="<<V<<endl;
    double width=1000;
    double height=1000;
    vector<vpsc::Rectangle*> rs;
    //srand(time(NULL));
    for(unsigned i=0;i<V;i++) {
        double x=getRand(width), y=getRand(height);
        rs.push_back(new vpsc::Rectangle(x,x+1,y,y+1));
    }
    CheckProgress test(0.001,100);
    clock_t starttime=clock();
    ConstrainedMajorizationLayout alg(rs,es,NULL,defaultEdgeLength,NULL,test);
    //cout << "Unconstrained layout" << endl;
    alg.setConstrainedLayout(true);
    alg.run();
    /*
    cout << "Constrained layout" << endl;
    alg.setYConstraints(&cy);
    alg.run();
    */
    double t=double(clock()-starttime)/double(CLOCKS_PER_SEC);
    cout<<"Time="<<t<<endl;
    output_svg(rs,es,"large_graph.svg");
    for(unsigned i=0;i<V;i++) {
        delete rs[i];
    }
}
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=4:softtabstop=4:encoding=utf-8:textwidth=99 :
