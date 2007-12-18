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

unsigned case1(Nodes& vs, Edges& es) {
    EdgePoints ps;
    addNode(vs,2.655473,0.000000,10.100000,10.000000);
    addNode(vs,49.957304,50.000000,10.100000,10.000000);
    addNode(vs,97.259136,0.000000,10.100000,10.000000);
    addNode(vs,63.828037,25.000000,10.100000,10.000000);
    ps.clear();
    addToPath(ps,vs[0],(topology::EdgePoint::RectIntersect)4);
    addToPath(ps,vs[1],(topology::EdgePoint::RectIntersect)4);
    es.push_back(new Edge(60.000000,ps));
    ps.clear();
    addToPath(ps,vs[1],(topology::EdgePoint::RectIntersect)4);
    addToPath(ps,vs[3],(topology::EdgePoint::RectIntersect)0);
    addToPath(ps,vs[2],(topology::EdgePoint::RectIntersect)4);
    es.push_back(new Edge(60.000000,ps));
    ps.clear();
    addToPath(ps,vs[2],(topology::EdgePoint::RectIntersect)4);
    addToPath(ps,vs[0],(topology::EdgePoint::RectIntersect)4);
    es.push_back(new Edge(60.000000,ps));
    return vs.size();
}
unsigned case2(Nodes& vs, Edges& es) {
    EdgePoints ps;
    addNode(vs,20.428107,0.000000,10.100000,10.000000);
    addNode(vs,45.716379,50.000000,10.100000,10.000000);
    addNode(vs,79.517409,0.000000,10.100000,10.000000);
    addNode(vs,17.425625,25.000000,10.100000,10.000000);
    ps.clear();
    addToPath(ps,vs[0],(topology::EdgePoint::RectIntersect)4);
    addToPath(ps,vs[3],(topology::EdgePoint::RectIntersect)2);
    addToPath(ps,vs[3],(topology::EdgePoint::RectIntersect)3);
    addToPath(ps,vs[1],(topology::EdgePoint::RectIntersect)4);
    es.push_back(new Edge(60.000000,ps));
    ps.clear();
    addToPath(ps,vs[1],(topology::EdgePoint::RectIntersect)4);
    addToPath(ps,vs[2],(topology::EdgePoint::RectIntersect)4);
    es.push_back(new Edge(60.000000,ps));
    ps.clear();
    addToPath(ps,vs[2],(topology::EdgePoint::RectIntersect)4);
    addToPath(ps,vs[0],(topology::EdgePoint::RectIntersect)4);
    es.push_back(new Edge(60.000000,ps));

    return vs.size();
}
unsigned case3(Nodes& vs, Edges& es) {
    EdgePoints ps;
    addNode(vs,101.846987,160.020991,10.100000,10.000000);
    addNode(vs,117.734372,216.805338,10.100000,10.000000);
    addNode(vs,160.316550,174.380230,10.100000,10.000000);
    addNode(vs,106.896987,179.923284,10.100000,10.000000);
    ps.clear();
    addToPath(ps,vs[0],(topology::EdgePoint::RectIntersect)4);
    addToPath(ps,vs[3],(topology::EdgePoint::RectIntersect)3);
    addToPath(ps,vs[1],(topology::EdgePoint::RectIntersect)4);
    es.push_back(new Edge(60.000000,ps));
    ps.clear();
    addToPath(ps,vs[1],(topology::EdgePoint::RectIntersect)4);
    addToPath(ps,vs[2],(topology::EdgePoint::RectIntersect)4);
    es.push_back(new Edge(60.000000,ps));
    ps.clear();
    addToPath(ps,vs[2],(topology::EdgePoint::RectIntersect)4);
    addToPath(ps,vs[0],(topology::EdgePoint::RectIntersect)4);
    es.push_back(new Edge(60.000000,ps));

    return vs.size();
}

void triangle() {
    printf("test: triangle()\n");
    Nodes vs;
    Edges es;

    const size_t V = case3(vs,es);

    project::Constraints cs;
    TopologyConstraints t(cola::HORIZONTAL,vs,es,cs);
    writeFile(vs,es,"triangle-000.svg");

    // test computeStress
    double stress=t.computeStress();
    printf("Stress=%f\n",stress);
    //assert(fabs(expectedStress-stress)<1e-4);

    double gradient[]={8.34249e-05,-7.59405e-05,-7.48445e-06,0};
    valarray<double> g(V);
    for(unsigned i=0;i<V;i++) {
        g[i]=gradient[i];
    }
    cola::SparseMap h(V);
    DesiredPositions des;
    //des.push_back(make_pair(3,60));
    des.push_back(make_pair(3,111.933269));
    for(unsigned i=1;i<10;i++) {
        g=0;
        h.clear();
        t.steepestDescent(g,h,des);
        stringstream ss;
        ss << "triangle-" << setfill('0') << setw(3) << i << ".svg";
        writeFile(vs,es,ss.str().c_str());
    }

    for(Nodes::iterator i=vs.begin();i!=vs.end();++i) {
        Node* v=*i;
        delete v->rect;
        delete v->var;
        delete v;
    }
    for_each(es.begin(),es.end(),delete_object());
}

int main() {
    triangle();
    return 0;
}
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=4:softtabstop=4:encoding=utf-8:textwidth=80 :
