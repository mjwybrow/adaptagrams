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

void split() {
    Nodes vs;
    Edges es;

    printf("test: bend2()\n");

    addNode(vs,100,100,40,20);
    addNode(vs,100,130,40,20);
    addNode(vs,70,160,40,20);
    addNode(vs,180,190,40,20);
    EdgePoints p1;
    addToPath(p1,vs[0],EdgePoint::CENTRE);
    addToPath(p1,vs[1],EdgePoint::BL);
    addToPath(p1,vs[1],EdgePoint::TL);
    addToPath(p1,vs[3],EdgePoint::CENTRE);
    es.push_back(new Edge(100,p1));

    EdgePoints p2;
    addToPath(p2,vs[2],EdgePoint::CENTRE);
    addToPath(p2,vs[3],EdgePoint::CENTRE);
    es.push_back(new Edge(50,p2));

    const size_t V = vs.size();

    project::Constraints cs;
    TopologyConstraints t(cola::HORIZONTAL,vs,es,cs);
    writeFile(vs,es,"split-0.svg");

    // test computeStress
    double stress=t.computeStress();
    printf("Stress=%f\n",stress);
    //assert(fabs(expectedStress-stress)<1e-4);

    valarray<double> g(V);
    cola::SparseMap h(V);
    for(unsigned i=1;i<5;i++) {
        g=0;
        h.clear();
        t.steepestDescent(g,h);
        stringstream ss;
        ss << "split-" << i << ".svg";
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
    split();
    return 0;
}
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=4:softtabstop=4:encoding=utf-8:textwidth=80 :
