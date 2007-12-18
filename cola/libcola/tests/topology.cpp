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
#include <iomanip>
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
void writeFile(const topology::Nodes& vs, const topology::Edges& es, const char *outputFileName) {
    const unsigned n=vs.size();
    vector<cola::Edge> cedges;

    for(unsigned i=0;i<es.size();i++) {
        cedges.push_back(make_pair(1,2));
    }

    vector<straightener::Route*> routes;
    for(topology::Edges::const_iterator e=es.begin();e!=es.end();++e) {
        routes.push_back((*e)->getRoute());
    }

    vector<string> labels(n);
    for(unsigned i=0;i<n;++i) {
        stringstream ss;
        ss << i;
        labels[i]=ss.str();
    }

    vector<vpsc::Rectangle*> rs;
    for(topology::Nodes::const_iterator i=vs.begin();i!=vs.end();++i) {
	    rs.push_back((*i)->rect);
    }
    OutputFile of(rs,cedges,NULL,outputFileName,true,false);
    of.setLabels(labels);
    of.routes=&routes;
    of.generate();

    for_each(routes.begin(),routes.end(),delete_object());
}
struct Test : TestConvergence {
    Test(const double d,const unsigned i,topology::Nodes& vs, topology::Edges& es) : TestConvergence(d,i), vs(vs), es(es) {}
    bool operator()(const double new_stress, valarray<double> & X, valarray<double> & Y) {
        cout << "stress="<<new_stress<<" iteration="<<iterations<<endl;
        stringstream ss;
        ss << "topology-" << setfill('0') << setw(3) << ++iterations << ".svg";
        writeFile(vs,es,ss.str().c_str());
        if(iterations<100) {
            return false;
        }
        return true;
        return TestConvergence::operator()(new_stress,X,Y);
    }
    double lastStress;
    topology::Nodes& vs;
    topology::Edges& es;
};
int main() {
    //srand(time(NULL));
    srand(3);
    const unsigned V = 4;
    Edge edge_array[] = { Edge(0, 1), Edge(1, 2), Edge(2, 0) };
    const std::size_t E = sizeof(edge_array) / sizeof(Edge);
    vector<Edge> es(E);
    copy(edge_array,edge_array+E,es.begin());
    vector<vpsc::Rectangle*> rs;
    double x,y,size=10;
    x=200,y=200;
    rs.push_back(new vpsc::Rectangle(x,x+size,y,y+size));
    x=250,y=250;
    rs.push_back(new vpsc::Rectangle(x,x+size,y,y+size));
    x=300,y=200;
    rs.push_back(new vpsc::Rectangle(x,x+size,y,y+size));
    x=250,y=225;
    rs.push_back(new vpsc::Rectangle(x,x+size,y,y+size));
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
    writeFile(vs,tes,"topology-000.svg");
    Locks locks;
    // we move the 4th node somewhere outside the triangle.  The triangle should be
    // dragged along!
    const double PI = 2.0*acos(0.0);
    double angle = getRand(2.0*PI);
    double dx=150*cos(angle), dy=150*sin(angle);
    double lx=rs[3]->getCentreX()+dx,
           ly=rs[3]->getCentreY()+dy;
    locks.push_back(Lock(3,lx,ly));
    printf(" Lock: %f,%f\n",lx,ly);
    //locks.push_back(Lock(3,150,150));
    SetDesiredPos preIteration(locks);
    Test test(0.00001,100,vs,tes);
    ConstrainedFDLayout alg(rs,es,NULL,idealLength,NULL,test,&preIteration);

    alg.setTopology(&vs,&tes);
    alg.run(true,true);
    double finalStress=alg.computeStress();
    printf("finalStress=%f\n",finalStress);

    //assert(finalStress<1e-5);
    for(unsigned i=0;i<V;i++) {
        delete rs[i];
    }
}
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=4:softtabstop=4:encoding=utf-8:textwidth=80 :
