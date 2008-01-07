/** \file nodedragging.cpp
 *
 * tests interaction with layout and solver.  We have a number of disconnected
 * shapes.  One shape is "dragged" through the others.  Overlaps should be
 * avoided.
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
#include <libcola/cola.h>
#include <libtopology/topology_graph.h>
#include <libproject/project.h>
#include "graphlayouttest.h"
using namespace std;
using namespace cola;

string outFName="nodedragging";

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
struct Test : TestConvergence {
    Test(const double d,const unsigned i,topology::Nodes& vs, topology::Edges& es) : TestConvergence(d,i), vs(vs), es(es), iter(1) {}
    bool operator()(const double new_stress, valarray<double> & X, valarray<double> & Y) {
        bool converged = TestConvergence::operator()(new_stress,X,Y);
        if(converged) {
            cout << "stress="<<new_stress<<" iteration="<<iter<<endl;
            stringstream ss;
            ss<<outFName<<"-"<< setfill('0') << setw(3) << iter++ << ".svg";
            writeFile(vs,es,ss.str());
        }
        return converged;
    }
    double lastStress;
    topology::Nodes& vs;
    topology::Edges& es;
    int iter;
};

void nodeDragging() {
//printf(
//"tests interaction with layout and solver.  We have a number of disconnected"
//"shapes.  One shape is 'dragged' through the others.  Overlaps should be"
//"avoided."
//"\n");
    const unsigned V = 10;
    Edge edge_array[] = { Edge(0, 1), Edge(1, 2), Edge(2, 0) };
    const std::size_t E = sizeof(edge_array) / sizeof(Edge);
    vector<Edge> es(edge_array,edge_array+E);
    vector<vpsc::Rectangle*> rs;
    double w=54, h=34;
    double
        x[]={279, 406, 444, 474, 406, 441, 375, 408, 373, 339},
        y[]={273, 279, 224, 179, 92, 135, 135, 179, 226, 179};
    for(unsigned i=0;i<V;++i) {
        rs.push_back(new vpsc::Rectangle(x[i],x[i]+w,y[i],y[i]+h));
    }
    double idealLength=60;
    // set up topology graph
    topology::Nodes vs;
    for(vector<Rectangle*>::iterator i = rs.begin(); i!=rs.end();++i) {
        addNode(vs,*i);
    }
    topology::Edges tes;
    writeFile(vs,tes,outFName+"-000.svg");

    Locks locks;
    locks.push_back(Lock(0,rs[0]->getCentreX(), rs[0]->getCentreY()));
    Lock& lock=locks[0];
    SetDesiredPos preIteration(locks);
    Test test(0.00001,100,vs,tes);
    ConstrainedFDLayout alg(rs,es,idealLength,NULL,test,&preIteration);
    alg.setTopology(&vs,&tes);

    double step=1;
    for(unsigned i=0;i<100;i++) {
        lock.pos[0]=rs[0]->getCentreX()+step;
        lock.pos[1]=rs[0]->getCentreY()-step;
        alg.run(true,true);
    }
    double finalStress=alg.computeStress();
    printf("finalStress=%f\n",finalStress);

    //assert(finalStress<1e-5);
    for_each(rs.begin(),rs.end(),delete_object());
    for_each(tes.begin(),tes.end(),delete_object());
    for_each(vs.begin(),vs.end(),delete_object());
}
int main() {
	nodeDragging();
}
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=4:softtabstop=4:encoding=utf-8:textwidth=80 :

