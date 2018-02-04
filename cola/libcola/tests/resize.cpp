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

/** \file nodedragging.cpp
 *
 * tests interaction with layout and solver.  We have a number of disconnected
 * shapes.  One shape is "dragged" through the others.  Overlaps should be
 * avoided.
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
#include <libcola/cola.h>
#include <libtopology/topology_graph.h>
#include <libproject/project.h>
#include "graphlayouttest.h"
using namespace std;
using namespace cola;

string outFName="resize";

topology::Node* addNode(
        topology::Nodes& vs, vpsc::Rectangle* r) {
    topology::Node *v = new topology::Node(vs.size(), r);
    vs.push_back(v);
    return v;
}
void addToPath(topology::EdgePoints& ps, topology::Node *v, topology::EdgePoint::RectIntersect i) {
    ps.push_back(new topology::EdgePoint(v,i));
}
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

void resize() {
//printf(
//"tests resizing of a shape.  We have a number of disconnected"
//"shapes.  One shape is 'enlarged'.  Overlaps should be"
//"avoided."
//"\n");
    const unsigned V = 9;
    Edge edge_array[] = { Edge(0, 1), Edge(1, 2), Edge(2, 0) };
    const std::size_t E = sizeof(edge_array) / sizeof(Edge);
    vector<Edge> es(edge_array,edge_array+E);
    vector<vpsc::Rectangle*> rs;
    const double w=54, h=34;
    const double
        x[]={406, 444, 474, 406, 441, 375, 408, 373, 339},
        y[]={279, 224, 179, 92, 135, 135, 179, 226, 179};
    const unsigned resizeID=6;
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

    Resizes resize;
    vpsc::Rectangle* r=rs[resizeID];
    resize.push_back(Resize(resizeID,r->getCentreX()-30, r->getCentreY()-30,
            r->width()+60,r->height()+60));
    PreIteration preIteration(resize);
    Test test(0.00001,100,vs,tes);
    ConstrainedFDLayout alg(rs,es,idealLength,nullptr,test,&preIteration);
    alg.setTopology(&vs,&tes);

    alg.run(true,true);

    double finalStress=alg.computeStress();
    printf("finalStress=%f\n",finalStress);

    //assert(finalStress<1e-5);
    for_each(rs.begin(),rs.end(),delete_object());
    for_each(tes.begin(),tes.end(),delete_object());
    for_each(vs.begin(),vs.end(),delete_object());
}
int main() {
	resize();
    return 0;
}
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=4:softtabstop=4:textwidth=80 :

