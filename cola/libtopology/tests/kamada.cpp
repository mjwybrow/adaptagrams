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
*/

/**
 * Test of topology conserving constraints.  Sets up a graph
 * with four nodes and two edges, initially with two bends around the
 * first disconnected node.  After an iteration an edge segment must be split
 * to bend around the second disconnected node.
 *
 * \file split.cpp
 * \author Tim Dwyer
 * \date Dec 2007
 */
#include <libvpsc/rectangle.h>
#include <libtopology/topology_constraints.h>
#include <libcola/cola.h>
#include <libcola/output_svg.h>
#include <libproject/project.h>
#include <vector>
#include <iostream>
#include <sstream>
using namespace std;
using namespace topology;
#include "test.h"

double expectedStress=1.48116;
double expectedG1[]=
{0.00849768, -0.0192465, 0, 0.0107488};

double expectedG2[]={0.003837,-0.001923,-0.001914};
double expectedH1[]= 
{0.000312442, -0.000438933, 0,  0.000126491, 
-0.000438933,  0.000746922, 0, -0.000307989,
           0,            0, 0,            0, 
 0.000126491, -0.000307989, 0,  0.000181498};

double expectedH2[]={0.000505,-0.000169,-0.000336,
                     -0.000169,0.000165,0.000004 ,
                     -0.000336,0.000004,0.000333};

void makeEdge(Edges& es,EdgePoints& eps) {
	es.push_back(new Edge(70,eps));
	eps.clear();
}
void split() {
    Nodes nodes;

    printf("test: bend2()\n");
addNode(nodes,233.500000,112.500000,35.000000,31.000000);
addNode(nodes,222.500000,155.500000,35.000000,31.000000);
addNode(nodes,276.500000,101.500000,35.000000,31.000000);
addNode(nodes,279.500000,310.500000,35.000000,31.000000);
addNode(nodes,236.500000,299.500000,35.000000,31.000000);
addNode(nodes,224.500000,256.500000,35.000000,31.000000);
addNode(nodes,380.500000,308.500000,35.000000,31.000000);
addNode(nodes,433.500000,253.500000,35.000000,31.000000);
addNode(nodes,423.500000,296.500000,35.000000,31.000000);
addNode(nodes,432.500000,152.500000,35.000000,31.000000);
addNode(nodes,420.500000,109.500000,35.000000,31.000000);
addNode(nodes,377.500000,99.500000,35.000000,31.000000);
addNode(nodes,360.500000,235.500000,35.000000,31.000000);
addNode(nodes,297.500000,236.500000,35.000000,31.000000);
addNode(nodes,359.500000,172.500000,35.000000,31.000000);
addNode(nodes,296.500000,173.500000,35.000000,31.000000);
Edges es;
EdgePoints eps;
addToPath(eps,nodes[5],EdgePoint::CENTRE);
addToPath(eps,nodes[4],EdgePoint::CENTRE);
makeEdge(es,eps);
addToPath(eps,nodes[13],EdgePoint::CENTRE);
addToPath(eps,nodes[3],EdgePoint::CENTRE);
makeEdge(es,eps);
addToPath(eps,nodes[4],EdgePoint::CENTRE);
addToPath(eps,nodes[13],EdgePoint::CENTRE);
makeEdge(es,eps);
addToPath(eps,nodes[3],EdgePoint::CENTRE);
addToPath(eps,nodes[4],EdgePoint::CENTRE);
makeEdge(es,eps);
addToPath(eps,nodes[5],EdgePoint::CENTRE);
addToPath(eps,nodes[4],EdgePoint::BL);
addToPath(eps,nodes[4],EdgePoint::TL);
addToPath(eps,nodes[4],EdgePoint::TR);
addToPath(eps,nodes[3],EdgePoint::CENTRE);
makeEdge(es,eps);
addToPath(eps,nodes[7],EdgePoint::CENTRE);
addToPath(eps,nodes[8],EdgePoint::CENTRE);
makeEdge(es,eps);
addToPath(eps,nodes[12],EdgePoint::CENTRE);
addToPath(eps,nodes[6],EdgePoint::CENTRE);
makeEdge(es,eps);
addToPath(eps,nodes[8],EdgePoint::CENTRE);
addToPath(eps,nodes[12],EdgePoint::CENTRE);
makeEdge(es,eps);
addToPath(eps,nodes[6],EdgePoint::CENTRE);
addToPath(eps,nodes[8],EdgePoint::CENTRE);
makeEdge(es,eps);
addToPath(eps,nodes[7],EdgePoint::CENTRE);
addToPath(eps,nodes[8],EdgePoint::BR);
addToPath(eps,nodes[8],EdgePoint::TR);
addToPath(eps,nodes[8],EdgePoint::TL);
addToPath(eps,nodes[6],EdgePoint::CENTRE);
makeEdge(es,eps);
addToPath(eps,nodes[12],EdgePoint::CENTRE);
addToPath(eps,nodes[7],EdgePoint::CENTRE);
makeEdge(es,eps);
addToPath(eps,nodes[15],EdgePoint::CENTRE);
addToPath(eps,nodes[12],EdgePoint::CENTRE);
makeEdge(es,eps);
addToPath(eps,nodes[13],EdgePoint::CENTRE);
addToPath(eps,nodes[14],EdgePoint::CENTRE);
makeEdge(es,eps);
addToPath(eps,nodes[12],EdgePoint::CENTRE);
addToPath(eps,nodes[13],EdgePoint::CENTRE);
makeEdge(es,eps);
addToPath(eps,nodes[14],EdgePoint::CENTRE);
addToPath(eps,nodes[12],EdgePoint::CENTRE);
makeEdge(es,eps);
addToPath(eps,nodes[13],EdgePoint::CENTRE);
addToPath(eps,nodes[15],EdgePoint::CENTRE);
makeEdge(es,eps);
addToPath(eps,nodes[5],EdgePoint::CENTRE);
addToPath(eps,nodes[13],EdgePoint::CENTRE);
makeEdge(es,eps);
addToPath(eps,nodes[0],EdgePoint::CENTRE);
addToPath(eps,nodes[2],EdgePoint::CENTRE);
makeEdge(es,eps);
addToPath(eps,nodes[1],EdgePoint::CENTRE);
addToPath(eps,nodes[15],EdgePoint::CENTRE);
makeEdge(es,eps);
addToPath(eps,nodes[10],EdgePoint::CENTRE);
addToPath(eps,nodes[14],EdgePoint::CENTRE);
makeEdge(es,eps);
addToPath(eps,nodes[11],EdgePoint::CENTRE);
addToPath(eps,nodes[10],EdgePoint::BL);
addToPath(eps,nodes[10],EdgePoint::BR);
addToPath(eps,nodes[10],EdgePoint::TR);
addToPath(eps,nodes[9],EdgePoint::CENTRE);
makeEdge(es,eps);
addToPath(eps,nodes[15],EdgePoint::CENTRE);
addToPath(eps,nodes[14],EdgePoint::CENTRE);
makeEdge(es,eps);
addToPath(eps,nodes[14],EdgePoint::CENTRE);
addToPath(eps,nodes[11],EdgePoint::CENTRE);
makeEdge(es,eps);
addToPath(eps,nodes[9],EdgePoint::CENTRE);
addToPath(eps,nodes[14],EdgePoint::CENTRE);
makeEdge(es,eps);
addToPath(eps,nodes[10],EdgePoint::CENTRE);
addToPath(eps,nodes[9],EdgePoint::CENTRE);
makeEdge(es,eps);
addToPath(eps,nodes[11],EdgePoint::CENTRE);
addToPath(eps,nodes[10],EdgePoint::CENTRE);
makeEdge(es,eps);
addToPath(eps,nodes[0],EdgePoint::CENTRE);
addToPath(eps,nodes[15],EdgePoint::CENTRE);
makeEdge(es,eps);
addToPath(eps,nodes[1],EdgePoint::CENTRE);
addToPath(eps,nodes[0],EdgePoint::CENTRE);
makeEdge(es,eps);
addToPath(eps,nodes[2],EdgePoint::CENTRE);
addToPath(eps,nodes[15],EdgePoint::CENTRE);
makeEdge(es,eps);
addToPath(eps,nodes[1],EdgePoint::CENTRE);
addToPath(eps,nodes[0],EdgePoint::TL);
addToPath(eps,nodes[0],EdgePoint::BL);
addToPath(eps,nodes[0],EdgePoint::BR);
addToPath(eps,nodes[2],EdgePoint::CENTRE);
makeEdge(es,eps);

    const size_t V = nodes.size();
    writeFile(nodes,es,"kamada-0.svg");

    vpsc::Variables vs;
    vpsc::Constraints cs;
    getVariables(nodes,vs);
    TopologyConstraints t(vpsc::HORIZONTAL,nodes,es,vs,cs);

    // test computeStress
    double stress=t.computeStress();
    printf("Stress=%f\n",stress);
    //assert(fabs(expectedStress-stress)<1e-4);

    valarray<double> g(V);
    cola::SparseMap h(V);
    for(unsigned i=1;i<5;i++) {
        g=0;
        h.clear();
        t.gradientProjection(g,h);
        stringstream ss;
        ss << "kamada-" << i << ".svg";
        writeFile(nodes,es,ss.str().c_str());
    }

    for_each(cs.begin(),cs.end(),delete_object());
    for_each(nodes.begin(),nodes.end(),delete_node());
    for_each(es.begin(),es.end(),delete_object());
}

int main() {
    split();
    return 0;
}
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=4:softtabstop=4:textwidth=80 :
