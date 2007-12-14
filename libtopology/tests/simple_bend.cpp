/**
 * Really basic test of topology conserving constraints.  Sets up a graph
 * with three nodes and one edge, initially bent around the disconnected node.
 *
 * \file simple_bend.cpp
 * \author Tim Dwyer
 * \date Dec 2007
 *
 * Runs two iterations of straightening.
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

void simpleBendHorizontal() {
    Nodes vs;
    project::Constraints cs;
    EdgePoints ps;
    Edges es;

    double expectedStress=1.05721;
    double expectedG1[]={0.0143283,0.00271783,-0.0170462};
    double expectedG2[]={0.0,0.004044,-0.004044};
    double expectedH1[]={0.000340898, -0.000138663, -0.000202235, 
                        -0.000138663, 0.000160573, -0.0000219108, 
                        -0.000202235, -0.0000219108, 0.000224146};
    double expectedH2[]={0.000000, 0.000000, 0.000000,
                         0.000000, 0.000094, -0.000094,
                         0.000000, -0.000094, 0.000094};

    printf("test: simpleBendHorizontal()\n");
	const size_t V = 3;

    addNode(vs,571.500000,363.500000,63.000000,43.000000);
    addNode(vs,620.0,469.500000,63.000000,43.000000);
    addNode(vs,541.500000,300.500000,63.000000,43.000000);
    addToPath(ps,vs[1],EdgePoint::CENTRE);
    addToPath(ps,vs[0],EdgePoint::BR);
    addToPath(ps,vs[2],EdgePoint::CENTRE);

    es.push_back(new Edge(100,ps));

    TopologyConstraints t(cola::HORIZONTAL,vs,es,cs);

    // test computeStress
    double stress=t.computeStress();
    assert(fabs(expectedStress-stress)<1e-4);

    writeFile(vs,es,"simpleBendH0.svg");

    valarray<double> g(V);
    cola::SparseMap h(V);
    g=0;
    t.steepestDescent(g,h);
    check(t,g,h,expectedG1,expectedH1);
    writeFile(vs,es,"simpleBendH1.svg");
    g=0;
    h.clear();
    t.steepestDescent(g,h);
    check(t,g,h,expectedG2,expectedH2);
    writeFile(vs,es,"simpleBendH2.svg");

    for_each(vs.begin(),vs.end(),delete_object());
    for_each(es.begin(),es.end(),delete_object());
}
void simpleBendVertical() {
    Nodes vs;
    project::Constraints cs;
    EdgePoints ps;
    Edges es;

    double expectedStress=1.2955;
    printf("test: simpleBendVertical()\n");

    addNode(vs,365,590,63.000000,43.000000);
    addNode(vs,469,620,63.000000,43.000000);
    addNode(vs,300,541,63.000000,43.000000);
	const size_t V = vs.size();
    addToPath(ps,vs[1],EdgePoint::CENTRE);
    addToPath(ps,vs[0],EdgePoint::TL);
    addToPath(ps,vs[2],EdgePoint::CENTRE);

    es.push_back(new Edge(100,ps));

    TopologyConstraints t(cola::VERTICAL,vs,es,cs);

    writeFile(vs,es,"simpleBendV0.svg");

    // test computeStress
    double stress=t.computeStress();
    assert(fabs(expectedStress-stress)<1e-4);

    valarray<double> g(V);
    cola::SparseMap h(V);
    g=0;
    t.steepestDescent(g,h);
    //check(t,g,h,expectedG1,expectedH1);
    writeFile(vs,es,"simpleBendV1.svg");
    g=0;
    h.clear();
    t.steepestDescent(g,h);
    //check(t,g,h,expectedG2,expectedH2);
    writeFile(vs,es,"simpleBendV2.svg");

    for_each(vs.begin(),vs.end(),delete_object());
    for_each(es.begin(),es.end(),delete_object());
}

int main() {
    simpleBendVertical();
    return 0;
}
  
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=4:softtabstop=4:encoding=utf-8:textwidth=80 :
