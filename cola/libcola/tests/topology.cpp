/** \file topology.cpp
 *
 * tests topology preserving layout.  graph is a triangle.
 * We have a fourth disconnected node starting inside the triangle.
 * We give the disconnected node a desired position outside the triangle
 * and begin layout.  Layout should converge to near zero stress as all
 * edges should be able to reach their desired lengths, and disconnected
 * node should still be inside the triangle at the end.
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
#include <fstream>

#include <vector>
#include <valarray>
#include <algorithm>
#include <float.h>
#include "graphlayouttest.h"
#include <libtopology/topology_graph.h>
#include <libproject/project.h>
using namespace std;
using namespace cola;

topology::Node* addNode(
        topology::Nodes& vs, vpsc::Rectangle* r) {
    topology::Node *v = new topology::Node(vs.size(), r);
    vs.push_back(v);
    return v;
}
void addToPath(topology::EdgePoints& ps, topology::Node *v, topology::EdgePoint::RectIntersect i) {
    ps.push_back(new topology::EdgePoint(v,i));
}
struct SetDesiredPos : public PreIteration {
    SetDesiredPos(Locks& locks) : PreIteration(locks) {}
};
int main() {
srand(time(NULL));
    const unsigned V = 4;
    Edge edge_array[] = { Edge(0, 1), Edge(1, 2), Edge(2, 0) };
    const std::size_t E = sizeof(edge_array) / sizeof(Edge);
    vector<Edge> es(E);
    copy(edge_array,edge_array+E,es.begin());
    vector<vpsc::Rectangle*> rs;
    double x,y,size=10;
    x=0,y=0;
    rs.push_back(new vpsc::Rectangle(x,x+size,y,y+size));
    x=50,y=50;
    rs.push_back(new vpsc::Rectangle(x,x+size,y,y+size));
    x=100,y=0;
    rs.push_back(new vpsc::Rectangle(x,x+size,y,y+size));
    x=50,y=25;
    rs.push_back(new vpsc::Rectangle(x,x+size,y,y+size));
    CheckProgress test(0.00001,100);
    double idealLength=60;
    // set up topology graph
    topology::Nodes vs;
    for(vector<Rectangle*>::iterator i = rs.begin(); i!=rs.end();++i) {
        addNode(vs,*i);
    }
    topology::Edges tes;
    for(vector<Edge>::iterator e=es.begin();e!=es.end();++e) {
        topology::EdgePoints ps;
        addToPath(ps,vs[e->first],topology::EdgePoint::CENTRE);
        addToPath(ps,vs[e->second],topology::EdgePoint::CENTRE);
        tes.push_back(new topology::Edge(idealLength, ps));
    }
    Locks locks;
    // we move the 4th node to the right.  The triangle should be
    // dragged along!
    locks.push_back(Lock(3,80,30));
    SetDesiredPos preIteration(locks);
    ConstrainedFDLayout alg(rs,es,NULL,idealLength,NULL,test,&preIteration);

    alg.setTopology(&vs,&tes);
    alg.run();
    double finalStress=alg.computeStress();
    printf("finalStress=%f\n",finalStress);

    // write output
    vector<string> labels(V);
    for(unsigned i=0;i<V;++i) {
        stringstream ss;
        ss << i;
        labels[i]=ss.str();
    }


    OutputFile output(rs,es,NULL,"topology.svg",true);
    output.setLabels(labels);
    vector<straightener::Route*> routes;
    for(vector<topology::Edge*>::const_iterator e=tes.begin();e!=tes.end();++e) {
        routes.push_back((*e)->getRoute());
    }
    output.routes=&routes;
    output.generate();
    assert(finalStress<1e-5);
    for(unsigned i=0;i<V;i++) {
        delete rs[i];
    }
}
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=4:softtabstop=4:encoding=utf-8:textwidth=80 :
