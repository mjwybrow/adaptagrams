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
    unsigned V=1000;
    CompoundConstraints cx,cy;
    vector<Edge> es = random_graph(V);
    double defaultEdgeLength=40;
    for(unsigned i=0;i<es.size();i++) {
        unsigned start=es[i].first, end=es[i].second;
        cy.push_back(
            new SeparationConstraint(start,end,-10));
    }

    cout << "V="<<V<<endl;
    double width=1000;
    double height=1000;
    //srand(time(NULL));
    vector<pair<double,double> > startpos(V);
    for(unsigned i=0;i<V;i++) {
        double x=getRand(width), y=getRand(height);
        startpos[i]=make_pair(x,y);
    }
    run_test(startpos,es,defaultEdgeLength,cx,cy,CG,false,"random_cg.svg");
    run_test(startpos,es,defaultEdgeLength,cx,cy,IP,false,"random_ip.svg");
    run_test(startpos,es,defaultEdgeLength,cx,cy,UGP,false,"random_ugp.svg");
    run_test(startpos,es,defaultEdgeLength,cx,cy,SGP,false,"random_sgp.svg");
    run_test(startpos,es,defaultEdgeLength,cx,cy,IP,true,"random_cip.svg");
    run_test(startpos,es,defaultEdgeLength,cx,cy,UGP,true,"random_cugp.svg");
    run_test(startpos,es,defaultEdgeLength,cx,cy,SGP,true,"random_csgp.svg");
}
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=4:softtabstop=4:encoding=utf-8:textwidth=99 :
