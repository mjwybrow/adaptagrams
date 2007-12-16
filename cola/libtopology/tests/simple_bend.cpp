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
void test1(Nodes& vs, EdgePoints& ps) {
    addNode(vs,399.000000,171.000000,54.100000,34.000000);
    addNode(vs,420.639762,64.068428,54.100000,34.000000);
    addNode(vs,278.912699,219.031132,54.100000,34.000000);
    addToPath(ps,vs[2],EdgePoint::RectIntersect(4));
    addToPath(ps,vs[1],EdgePoint::RectIntersect(4));
}
void test2(Nodes& vs, EdgePoints& ps) {
}
void simple() {
    Nodes vs;
    EdgePoints ps;
    test1(vs,ps);
    const size_t V = vs.size();
    Edges es;
    es.push_back(new Edge(210,ps));

    writeFile(vs,es,"simple-0.svg");

    project::Constraints cs;
    TopologyConstraints t(cola::HORIZONTAL,vs,es,cs);

    // test computeStress
    double stress=t.computeStress();
    printf("Stress=%f\n",stress);
    //assert(fabs(expectedStress-stress)<1e-4);

    valarray<double> g(V);
    cola::SparseMap h(V);
    DesiredPositions d;
    d.push_back(make_pair(0,project::Desired(361.000000)));
    for(unsigned i=1;i<2;i++) {
        g=0;
        h.clear();
        t.steepestDescent(g,h,d);
        stringstream ss;
        ss << "simple-" << i << ".svg";
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
    simple();
    return 0;
}
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=4:softtabstop=4:encoding=utf-8:textwidth=80 :
