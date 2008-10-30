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
#include <vector>
#include <iostream>
#include <iomanip>
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

struct TestCase {
    Nodes nodes;
    Edges es;
    DesiredPositions des;
    valarray<double> g;
    EdgePoints ps;
    vpsc::Constraints cs;
    vpsc::Dim dim;
    unsigned iterations;
    TestCase() : dim(vpsc::HORIZONTAL), iterations(10) { }

    void run() {
        writeFile(nodes,es,"triangle-000.svg");

        if(g.size()!=nodes.size()) {
            g.resize(nodes.size());
            g=0;
        }
        vpsc::Variables vs;
        getVariables(nodes,vs);
        TopologyConstraints t(dim,nodes,es,vs,cs);
        // test computeStress
        double stress=computeStress(es);
        printf("Stress=%f\n",stress);
        //assert(fabs(expectedStress-stress)<1e-4);

        cola::SparseMap h(nodes.size());
        for(unsigned i=1;i<iterations;i++) {
            g=0;
            h.clear();
            t.gradientProjection(g,h,des);
            stringstream ss;
            ss << "triangle-" << setfill('0') << setw(3) << i << ".svg";
            writeFile(nodes,es,ss.str().c_str());
        }
    }
    ~TestCase() {
        for_each(es.begin(),es.end(),delete_object());
        for_each(cs.begin(),cs.end(),delete_object());
        for(Nodes::iterator i=nodes.begin();i!=nodes.end();++i) {
            Node* v=*i;
            delete v->rect;
            delete v->var;
            delete v;
        }
    }
    void setGradient(double* gradient) {
        assert(nodes.size()>0);
        g.resize(nodes.size());
        for(unsigned i=0;i<nodes.size();i++) {
            g[i]=gradient[i];
        }
    }
    Node* addNode(double x, double y, double w, double h) {
        vpsc::Rectangle* r = new vpsc::Rectangle(x,x+w,y,y+h);
        const unsigned id = nodes.size();
        Node *v = new Node(id, r, new vpsc::Variable(id));
        nodes.push_back(v);
        return v;
    }
    void addEdge(double idealLength) {
        assert(ps.size()>=2);
        es.push_back(new Edge(idealLength,ps));
        ps.clear();
    }
    void addDesired(unsigned vid, double pos) {
        des.push_back(make_pair(vid,pos));
    }
    void addToPath(unsigned vid, topology::EdgePoint::RectIntersect i) {
        ps.push_back(new EdgePoint(nodes[vid],i));
    }
};
void case1(TestCase& t) {
    t.addNode(2.655473,0.000000,10.100000,10.000000);
    t.addNode(49.957304,50.000000,10.100000,10.000000);
    t.addNode(97.259136,0.000000,10.100000,10.000000);
    t.addNode(63.828037,25.000000,10.100000,10.000000);
    t.addToPath(0,(topology::EdgePoint::RectIntersect)4);
    t.addToPath(1,(topology::EdgePoint::RectIntersect)4);
    t.addEdge(60);
    t.addToPath(1,(topology::EdgePoint::RectIntersect)4);
    t.addToPath(3,(topology::EdgePoint::RectIntersect)0);
    t.addToPath(2,(topology::EdgePoint::RectIntersect)4);
    t.addEdge(60);
    t.addToPath(2,(topology::EdgePoint::RectIntersect)4);
    t.addToPath(0,(topology::EdgePoint::RectIntersect)4);
    t.addEdge(60);
}
void case2(TestCase& t) {
    t.addNode(20.428107,0.000000,10.100000,10.000000);
    t.addNode(45.716379,50.000000,10.100000,10.000000);
    t.addNode(79.517409,0.000000,10.100000,10.000000);
    t.addNode(17.425625,25.000000,10.100000,10.000000);
    t.addToPath(0,(topology::EdgePoint::RectIntersect)4);
    t.addToPath(3,(topology::EdgePoint::RectIntersect)2);
    t.addToPath(3,(topology::EdgePoint::RectIntersect)3);
    t.addToPath(1,(topology::EdgePoint::RectIntersect)4);
    t.addEdge(60);
    t.addToPath(1,(topology::EdgePoint::RectIntersect)4);
    t.addToPath(2,(topology::EdgePoint::RectIntersect)4);
    t.addEdge(60);
    t.addToPath(2,(topology::EdgePoint::RectIntersect)4);
    t.addToPath(0,(topology::EdgePoint::RectIntersect)4);
    t.addEdge(60);
}
void case3(TestCase& t) {
    t.addNode(101.846987,160.020991,10.100000,10.000000);
    t.addNode(117.734372,216.805338,10.100000,10.000000);
    t.addNode(160.316550,174.380230,10.100000,10.000000);
    t.addNode(106.896987,179.923284,10.100000,10.000000);
    t.addToPath(0,(topology::EdgePoint::RectIntersect)4);
    t.addToPath(3,(topology::EdgePoint::RectIntersect)3);
    t.addToPath(1,(topology::EdgePoint::RectIntersect)4);
    t.addEdge(60);
    t.addToPath(1,(topology::EdgePoint::RectIntersect)4);
    t.addToPath(2,(topology::EdgePoint::RectIntersect)4);
    t.addEdge(60);
    t.addToPath(2,(topology::EdgePoint::RectIntersect)4);
    t.addToPath(0,(topology::EdgePoint::RectIntersect)4);
    t.addEdge(60);
}

void case4(TestCase& t) {
    t.addNode(205.081973,200.000000,10.100000,10.000000);
    t.addNode(250.000000,250.000000,10.100000,10.000000);
    t.addNode(294.918027,200.000000,10.100000,10.000000);
    t.addNode(289.868027,225.000000,10.100000,10.000000);
    t.addToPath(0,(topology::EdgePoint::RectIntersect)4);
    t.addToPath(1,(topology::EdgePoint::RectIntersect)4);
    t.addEdge(60);
    t.addToPath(1,(topology::EdgePoint::RectIntersect)4);
    t.addToPath(3,(topology::EdgePoint::RectIntersect)0);
    t.addToPath(3,(topology::EdgePoint::RectIntersect)1);
    t.addToPath(2,(topology::EdgePoint::RectIntersect)4);
    t.addEdge(60);
    t.addToPath(2,(topology::EdgePoint::RectIntersect)4);
    t.addToPath(0,(topology::EdgePoint::RectIntersect)4);
    t.addEdge(60);

    t.addDesired(3,400.000000);
}
void case5(TestCase& t) {
    t.addNode(335.658737,287.843360,10.100000,10.000000);
    t.addNode(338.8,347.780782,10.100000,10.000000);
    t.addNode(388.857209,315.489405,10.100000,10.000000);
    t.addNode(348.9,337.789705,10.100000,10.000000);
    t.addToPath(0,(topology::EdgePoint::RectIntersect)4);
    t.addToPath(1,(topology::EdgePoint::RectIntersect)4);
    t.addEdge(60);
    t.addToPath(1,(topology::EdgePoint::RectIntersect)4);
    t.addToPath(3,(topology::EdgePoint::RectIntersect)0);
    t.addToPath(2,(topology::EdgePoint::RectIntersect)4);
    t.addEdge(60);
    t.addToPath(2,(topology::EdgePoint::RectIntersect)4);
    t.addToPath(0,(topology::EdgePoint::RectIntersect)4);
    t.addEdge(60);
    t.addDesired(3,353.886210);

    double gradient[]={1.15455e-05,0.000114288,-0.000125833,0};
    t.setGradient(gradient);
}

void triangle(void tcase(TestCase&)) {
    printf("test: triangle()\n");
    TestCase t;
    tcase(t);
    t.run();
}

int main() {
    //triangle(case1);
    //triangle(case2);
    //triangle(case3);
    //triangle(case4);
    triangle(case5);
    return 0;
}
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=4:softtabstop=4:encoding=utf-8:textwidth=80 :
