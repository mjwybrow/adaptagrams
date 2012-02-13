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

// generates a random graph with a power-law distribution of node degrees
// the algorithm is pretty much the Barabasi-Albert model but with an extra step
// so that I can guarantee the number of nodes in the graph.
// We begin with an initial chain of m nodes.
// Then, we add new nodes, connecting them to existing nodes v with probability
// degree(v)/(sum all degrees)
// We perform an extra step, looking for disconnected nodes, connecting them
// to other nodes again with probability dependent on degree
#include<iostream>
#include<vector>
#include <cmath>
#include <time.h>
#include <valarray>

#include "graphlayouttest.h"

void scale_free(const unsigned n = 50) {
    const unsigned m = 3; // begin with a chain of m nodes
    const double g=0.08; // edge density, every time we add a node we add 1+g*rand(0-1) edges
                  // connecting them to existing nodes with probability based on degree
    unsigned d[n];
    double defaultEdgeLength=100;
    vector<Edge> es;
    CompoundConstraints cx,cy;
    for(unsigned i=0;i<n;i++) {
        d[i]=0;
    }
    unsigned sumdegree=0;
    for(unsigned i=1;i<m;i++) {
        es.push_back(make_pair(i-1,i)); 
        d[i]++;d[i-1]++;
        sumdegree+=2;
    }
    srand(3);
    for(unsigned i=2;i<n;i++) {
        for(unsigned j=0;j<i;j++) {
            double p=(double)d[j]/(double)sumdegree;
            double r=(double)rand()/(double)RAND_MAX;
            if(r*p>g) {
                es.push_back(make_pair(j,i)); 
                d[j]++;d[i]++;
                sumdegree+=2;
            }
        }
    }
    for(unsigned i=0;i<n;i++) {
        if(d[i]==0) {
            double maxP=0;
            unsigned end=0;
            for(unsigned j=0;j<n;j++) {
                if(j==i) continue;
                double p=(double)d[j]/(double)sumdegree;
                double r=(double)rand()/(double)RAND_MAX;
                if(r*p>maxP) {
                    maxP=r*p;
                    end=j;
                }
            }
            if(end>i) {
                es.push_back(make_pair(i,end)); 
            } else {
                es.push_back(make_pair(end,i)); 
            }
            d[end]++;d[i]++;
            sumdegree+=2;
        }
    }
    valarray<double> eweights(es.size());
    sort(d,d+n);
    unsigned degree=0,ctr=0;
    printf("degree distribution:\n");
    for(unsigned i=0;i<n;i++) {
        if(degree!=d[i]) {
            if(degree!=0) printf("%d,%d\n",degree,ctr);
            degree=d[i]; 
            ctr=0;
        }
        ctr++;
    }
    printf("%d,%d\n",degree,ctr);
    for(unsigned i=0;i<es.size();i++) {
        unsigned a=es[i].first, b=es[i].second;
        eweights[i]=1;
        cy.push_back(new SeparationConstraint(a,b,defaultEdgeLength/3));
    }

    cout << "|V|="<<n<<endl;
    double width=1000;
    double height=1000;
    vector<pair<double,double> > startpos(n);
    for(unsigned i=0;i<n;i++) {
        double x=getRand(width), y=getRand(height);
        startpos[i]=make_pair(x,y);
    }
    const char *f="scalefree";
    //run_test(startpos,es,defaultEdgeLength,cx,cy,CG,false,f,"cg");
    //run_test(startpos,es,defaultEdgeLength,cx,cy,IP,false,f,"ip");
    //run_test(startpos,es,defaultEdgeLength,cx,cy,UGP,false,f,"ugp");
    //run_test(startpos,es,defaultEdgeLength,cx,cy,SGP,false,f,"sgp");
    //run_test(startpos,es,defaultEdgeLength,cx,cy,IP,true,f,"cip");
    //run_test(startpos,es,defaultEdgeLength,cx,cy,UGP,true,f,"cugp");
    run_test(startpos,es,defaultEdgeLength,cx,cy,SGP,true,f,"csgp");
}
int main() {
    for(unsigned i=1;i<2;i++) {
        scale_free(i*50);
    }
    return 0;
}
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=4:softtabstop=4:textwidth=99 :
