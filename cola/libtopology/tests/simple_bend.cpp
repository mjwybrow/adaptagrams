/*
 * vim: ts=4 sw=4 et tw=0 wm=0
 *
 * libtopology - Classes used in generating and managing topology constraints.
 *
 * Copyright (C) 2007-2008  Monash University
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
 * Authors:
 *   Tim Dwyer <tgdwyer@gmail.com>
*/

/**
 * Test of topology conserving constraints.  Various configurations of a graph
 * with three nodes and one edge.
 *
 * \file simple_bend.cpp
 * \author Tim Dwyer
 * \date Dec 2007
 */
#include <libvpsc/rectangle.h>
#include <libtopology/topology_constraints.h>
#include <libcola/cola.h>
#include <libcola/output_svg.h>
#include <libvpsc/variable.h>
#include <libvpsc/constraint.h>
#include <vector>
#include <iostream>
#include <sstream>
using namespace std;
using namespace topology;
#include "test.h"
void test1(Nodes& vs, EdgePoints& ps, cola::DesiredPositionsInDim& d, string& name) {
    name=string("test1");
    addNode(vs,400,170,50,30);
    addNode(vs,420,65,50,30);
    addNode(vs,280,220,50,30);
    addToPath(ps,vs[2],EdgePoint::RectIntersect(4));
    addToPath(ps,vs[1],EdgePoint::RectIntersect(4));
    d.push_back(make_pair(0,361.000000));
}
void test2(Nodes& vs, EdgePoints& ps, cola::DesiredPositionsInDim& d, string& name) {
    name=string("test2");
    addNode(vs,0,0,54,34);
    addNode(vs,100,100,54,34);
    addNode(vs,0,50,54,34);
    addToPath(ps,vs[0],EdgePoint::RectIntersect(4));
    addToPath(ps,vs[1],EdgePoint::RectIntersect(4));
    d.push_back(make_pair(2,150));
}
void test3(Nodes& vs, EdgePoints& ps, cola::DesiredPositionsInDim& d, string& name) {
    name=string("test3");
    addNode(vs,0,0,54,34);
    addNode(vs,100,100,54,34);
    addNode(vs,100,50,54,34);
    addToPath(ps,vs[0],EdgePoint::RectIntersect(4));
    addToPath(ps,vs[1],EdgePoint::RectIntersect(4));
    d.push_back(make_pair(2,0));
}
void test4(Nodes& vs, EdgePoints& ps, cola::DesiredPositionsInDim& d, string& name) {
    name=string("test4");
    //double gradient[]={0,3.03652e-19,-3.03652e-19,}
    addNode(vs,455.950000,324.166331,54.000000,34.000000);
    addNode(vs,416.252794,290.166331,54.000000,34.000000);
    addNode(vs,620.342448,342.224389,54.000000,34.000000);
    addToPath(ps,vs[2],(topology::EdgePoint::RectIntersect)4);
    addToPath(ps,vs[0],(topology::EdgePoint::RectIntersect)1);
    addToPath(ps,vs[1],(topology::EdgePoint::RectIntersect)4);
    d.push_back(make_pair(0,339.000000));
}
//
// more challenging tests:
//
//  - zero length edges
//  - edges parallel to scan line
//
// 4 rectangles arranged and touching two above two below.  Additional
// rectangle on top and another below connected with an edge running between
// the other four.  Move the rectangles so that they slide across each other.
//  ---
//  | |
//  ---
//    \                                                                     .
//  ---|---
//  |  |  |
//  ---|---
//   ---|---
//   |  |  |->
//   ---|---
//      \                                                                   .
//     -----
//     |   |
//     -----
void test5(Nodes& vs, EdgePoints& ps, cola::DesiredPositionsInDim& d, string& name) {
    name=string("test5");
    //double gradient[]={0,3.03652e-19,-3.03652e-19,}
    addNode(vs,0,0,10,10);
    addNode(vs,40,50,10,10);
    addNode(vs,10,20,10,10);
    addNode(vs,20,20,10,10);
    addNode(vs,15,30,10,10);
    addNode(vs,25,30,10,10);
    addToPath(ps,vs[0],(topology::EdgePoint::RectIntersect)4);
    addToPath(ps,vs[2],(topology::EdgePoint::RectIntersect)1);
    addToPath(ps,vs[3],(topology::EdgePoint::RectIntersect)3);
    addToPath(ps,vs[4],(topology::EdgePoint::RectIntersect)1);
    addToPath(ps,vs[5],(topology::EdgePoint::RectIntersect)3);
    addToPath(ps,vs[1],(topology::EdgePoint::RectIntersect)4);
    d.push_back(make_pair(2,40));
}
void simple(void test(Nodes&, EdgePoints&, cola::DesiredPositionsInDim&,string&)){
    Nodes nodes;
    EdgePoints ps;
    cola::DesiredPositionsInDim d;
    string name;
    test(nodes,ps,d,name);
    const size_t V = nodes.size();
    Edges es;
    es.push_back(new Edge(0,210,ps));

    writeFile(nodes,es,"simple-"+name+"-0.svg");

    vpsc::Variables vs;
    getVariables(nodes,vs);
    vpsc::Constraints cs;

    { // scope for t, so that t gets destroyed before es
        TopologyConstraints t(vpsc::HORIZONTAL,nodes,es,nullptr,vs,cs);

        // test computeStress
        double stress=computeStress(es);
        printf("Stress=%f\n",stress);
        //assert(fabs(expectedStress-stress)<1e-4);

        valarray<double> g(V);
        valarray<double> x(V);
        for(unsigned i=0;i<V;++i) {
            x[i]=nodes[i]->rect->getCentreX();
        }
        cola::SparseMap h(V);
        for(unsigned i=1;i<10;i++) {
            g=0;
            /*
            h.clear();
            t.computeForces(g,h);
            cola::SparseMatrix H(h);
            x-=computeStepSize(H,g,g)*g;
            */
            setVariableDesiredPositions(vs,d,x);
            t.solve();
            stringstream ss;
            ss << "simple-" << name << "-" << i << ".svg";
            writeFile(nodes,es,ss.str());
        }
    }

    for_each(nodes.begin(),nodes.end(),delete_node());
    for_each(vs.begin(),vs.end(),delete_object());
    for_each(cs.begin(),cs.end(),delete_object());
    for_each(es.begin(),es.end(),delete_object());
}

int main() {
    /*
    simple(test1);
    simple(test2);
    simple(test3);
    simple(test4);
    */
    simple(test5);
    return 0;
}
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=4:softtabstop=4:textwidth=80 :
