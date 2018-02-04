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

/** \file boundary.cpp
 *
 * a clustered graph using libtopology to preserve the cluster boundary.
 */
/*
* Authors:
*   Tim Dwyer <tgdwyer@gmail.com>
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
using namespace topology;

Node* addNode( Nodes& vs, vpsc::Rectangle* r) {
    Node *v = new Node(vs.size(), r);
    vs.push_back(v);
    return v;
}
void addToPath(EdgePoints& ps, Node *v, EdgePoint::RectIntersect i) {
    ps.push_back(new EdgePoint(v,i));
}
struct SetDesiredPos : public PreIteration {
    SetDesiredPos(Locks& locks) : PreIteration(locks) {}
};
struct Test : TestConvergence {
    Test(const double d,const unsigned i,topology::Nodes& vs, topology::Edges& es,const string fname) : TestConvergence(d,i), vs(vs), es(es), fname(fname) {}
    bool operator()(const double new_stress, valarray<double> & X, valarray<double> & Y) {
        bool converged= TestConvergence::operator()(new_stress,X,Y);
        if(converged) {
            cerr << "stress="<<new_stress<<" iteration="<<iterations<<endl;
            stringstream ss;
            ss << fname << "-" << setfill('0') << setw(3) << iterations << ".svg";
            cerr << "writing file: " << ss.str() << endl;
            writeFile(vs,es,ss.str());
        }
        return converged;
    }
    double lastStress;
    topology::Nodes& vs;
    topology::Edges& es;
    const string fname;
};
void clusterBoundary() {
    vector<cola::Edge> es;
    vector<vpsc::Rectangle*> rs;
    rs.push_back(new vpsc::Rectangle(395,449,155,189));
    rs.push_back(new vpsc::Rectangle(309,363,155,189));
    double idealLength=60;
    // set up topology graph
    topology::Nodes vs;
    for(vector<Rectangle*>::iterator i = rs.begin(); i!=rs.end();++i) {
        addNode(vs,*i);
    }
    Edges tes;
    EdgePoints ps;
    addToPath(ps,vs[1],EdgePoint::BL);
    addToPath(ps,vs[0],EdgePoint::BR);
    addToPath(ps,vs[0],EdgePoint::TR);
    addToPath(ps,vs[1],EdgePoint::TL);
    ps.push_back(ps[0]);
    tes.push_back(new topology::Edge(idealLength, ps));

    writeFile(vs,tes,"boundary-000.svg");

    Locks locks;
    locks.push_back(Lock(0,rs[0]->getCentreX(), rs[0]->getCentreY()+10));
    locks.push_back(Lock(1,rs[1]->getCentreX(), rs[1]->getCentreY()));
    SetDesiredPos preIteration(locks);

    Test test(0.00001,100,vs,tes,"boundary");
    cola::ConstrainedFDLayout alg(rs,es,idealLength,nullptr,test,&preIteration);

    alg.setTopology(&vs,&tes);
    alg.run(true,true);
    double finalStress=alg.computeStress();
    printf("finalStress=%f\n",finalStress);

    //assert(finalStress<1e-5);
    for_each(rs.begin(),rs.end(),cola::delete_object());
    for_each(tes.begin(),tes.end(),cola::delete_object());
    for_each(vs.begin(),vs.end(),cola::delete_object());
}
int main() {
    clusterBoundary();
    return 0;
}
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=4:softtabstop=4:textwidth=80 :
