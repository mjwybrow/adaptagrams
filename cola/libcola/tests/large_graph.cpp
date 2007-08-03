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
#include "graphlayouttest.h"

using namespace std;
using namespace cola;

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
            new SeparationConstraint(start,end,defaultEdgeLength/3));
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
    //srand(time(NULL));
    for(unsigned i=0;i<V;i++) {
        double x=getRand(width), y=getRand(height);
        startpos[i]=make_pair(x,y);
    }
    const char *testname="large_graph";
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
	CheckProgress test(0.0001,20);
    ConstrainedMajorizationLayout alg(rs,es,NULL,defaultEdgeLength,NULL,test);
    //alg.setYConstraints(&cy);
	alg.run();
	ConstrainedFDLayout alg2(rs,es,NULL,defaultEdgeLength,NULL,test);
    //alg2.setYConstraints(&cy);
	alg2.run();
	OutputFile output(rs,es,NULL,"large_graph.svg");
	output.generate();
	for(unsigned i=0;i<V;i++) {
		delete rs[i];
	}
}
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=4:softtabstop=4:encoding=utf-8:textwidth=99 :
