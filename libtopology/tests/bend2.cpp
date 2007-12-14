/**
 * Really basic test of topology conserving constraints.  Sets up a graph
 * with three nodes and one edge, initially with two bends around the
 * disconnected node.
 *
 * Runs two iterations of straightening.
 *
 * \file bend2.cpp
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
{0.0143234 + 0.0201766, -0.0143234, -0.0201766};
double expectedG2[]={0.003837,-0.001923,-0.001914};
double expectedH1[]= {0.000656695, -0.000319065, -0.000337629
                     ,-0.000319065, 0.000221508, 0.0000975574
                     ,-0.000337629, 0.0000975574, 0.000240072};
double expectedH2[]={0.000505,-0.000169,-0.000336,
                     -0.000169,0.000165,0.000004 ,
                     -0.000336,0.000004,0.000333};
void bend2() {
    printf("test: bend2()\n");

    Nodes vs;
    project::Constraints cs;
    EdgePoints ps;
    Edges es;

    addNode(vs,571.5,363.5,63,43);
    addNode(vs,541.5,469.5,63,43);
    addNode(vs,541.5,300.5,63,43);
	const size_t V = vs.size();
    addToPath(ps,vs[1],EdgePoint::CENTRE);
    addToPath(ps,vs[0],EdgePoint::TR);
    addToPath(ps,vs[0],EdgePoint::BR);
    addToPath(ps,vs[2],EdgePoint::CENTRE);

    es.push_back(new Edge(100,ps));

    TopologyConstraints t(cola::HORIZONTAL,vs,es,cs);
    writeFile(vs,es,"bend2-0.svg");

    // test computeStress
    double stress=t.computeStress();
    printf("Stress=%f\n",stress);
    assert(fabs(expectedStress-stress)<1e-4);

    valarray<double> g(V);
    cola::SparseMap h(V);
    g=0;h.clear();
    t.steepestDescent(g,h);
    check(t, g, h, expectedG1, expectedH1);
    writeFile(vs,es,"bend2-1.svg");
    g=0;h.clear();
    t.steepestDescent(g,h);
    check(t, g, h, expectedG2, expectedH2);
    writeFile(vs,es,"bend2-2.svg");

    for(Nodes::iterator i=vs.begin();i!=vs.end();++i) {
        Node* v=*i;
        delete v->rect;
        delete v->var;
        delete v;
    }
    for_each(es.begin(),es.end(),delete_object());
}

int main() {
    bend2();
    return 0;
}
  
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=4:softtabstop=4:encoding=utf-8:textwidth=80 :
