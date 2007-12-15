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
    Nodes vs;

    printf("test: bend2()\n");
addNode(vs,233.500000,112.500000,35.000000,31.000000);
addNode(vs,222.500000,155.500000,35.000000,31.000000);
addNode(vs,276.500000,101.500000,35.000000,31.000000);
addNode(vs,279.500000,310.500000,35.000000,31.000000);
addNode(vs,236.500000,299.500000,35.000000,31.000000);
addNode(vs,224.500000,256.500000,35.000000,31.000000);
addNode(vs,380.500000,308.500000,35.000000,31.000000);
addNode(vs,433.500000,253.500000,35.000000,31.000000);
addNode(vs,423.500000,296.500000,35.000000,31.000000);
addNode(vs,432.500000,152.500000,35.000000,31.000000);
addNode(vs,420.500000,109.500000,35.000000,31.000000);
addNode(vs,377.500000,99.500000,35.000000,31.000000);
addNode(vs,360.500000,235.500000,35.000000,31.000000);
addNode(vs,297.500000,236.500000,35.000000,31.000000);
addNode(vs,359.500000,172.500000,35.000000,31.000000);
addNode(vs,296.500000,173.500000,35.000000,31.000000);
Edges es;
EdgePoints eps;
addToPath(eps,vs[5],EdgePoint::CENTRE);
addToPath(eps,vs[4],EdgePoint::CENTRE);
makeEdge(es,eps);
addToPath(eps,vs[13],EdgePoint::CENTRE);
addToPath(eps,vs[3],EdgePoint::CENTRE);
makeEdge(es,eps);
addToPath(eps,vs[4],EdgePoint::CENTRE);
addToPath(eps,vs[13],EdgePoint::CENTRE);
makeEdge(es,eps);
addToPath(eps,vs[3],EdgePoint::CENTRE);
addToPath(eps,vs[4],EdgePoint::CENTRE);
makeEdge(es,eps);
addToPath(eps,vs[5],EdgePoint::CENTRE);
addToPath(eps,vs[4],EdgePoint::BL);
addToPath(eps,vs[4],EdgePoint::TL);
addToPath(eps,vs[4],EdgePoint::TR);
addToPath(eps,vs[3],EdgePoint::CENTRE);
makeEdge(es,eps);
addToPath(eps,vs[7],EdgePoint::CENTRE);
addToPath(eps,vs[8],EdgePoint::CENTRE);
makeEdge(es,eps);
addToPath(eps,vs[12],EdgePoint::CENTRE);
addToPath(eps,vs[6],EdgePoint::CENTRE);
makeEdge(es,eps);
addToPath(eps,vs[8],EdgePoint::CENTRE);
addToPath(eps,vs[12],EdgePoint::CENTRE);
makeEdge(es,eps);
addToPath(eps,vs[6],EdgePoint::CENTRE);
addToPath(eps,vs[8],EdgePoint::CENTRE);
makeEdge(es,eps);
addToPath(eps,vs[7],EdgePoint::CENTRE);
addToPath(eps,vs[8],EdgePoint::BR);
addToPath(eps,vs[8],EdgePoint::TR);
addToPath(eps,vs[8],EdgePoint::TL);
addToPath(eps,vs[6],EdgePoint::CENTRE);
makeEdge(es,eps);
addToPath(eps,vs[12],EdgePoint::CENTRE);
addToPath(eps,vs[7],EdgePoint::CENTRE);
makeEdge(es,eps);
addToPath(eps,vs[15],EdgePoint::CENTRE);
addToPath(eps,vs[12],EdgePoint::CENTRE);
makeEdge(es,eps);
addToPath(eps,vs[13],EdgePoint::CENTRE);
addToPath(eps,vs[14],EdgePoint::CENTRE);
makeEdge(es,eps);
addToPath(eps,vs[12],EdgePoint::CENTRE);
addToPath(eps,vs[13],EdgePoint::CENTRE);
makeEdge(es,eps);
addToPath(eps,vs[14],EdgePoint::CENTRE);
addToPath(eps,vs[12],EdgePoint::CENTRE);
makeEdge(es,eps);
addToPath(eps,vs[13],EdgePoint::CENTRE);
addToPath(eps,vs[15],EdgePoint::CENTRE);
makeEdge(es,eps);
addToPath(eps,vs[5],EdgePoint::CENTRE);
addToPath(eps,vs[13],EdgePoint::CENTRE);
makeEdge(es,eps);
addToPath(eps,vs[0],EdgePoint::CENTRE);
addToPath(eps,vs[2],EdgePoint::CENTRE);
makeEdge(es,eps);
addToPath(eps,vs[1],EdgePoint::CENTRE);
addToPath(eps,vs[15],EdgePoint::CENTRE);
makeEdge(es,eps);
addToPath(eps,vs[10],EdgePoint::CENTRE);
addToPath(eps,vs[14],EdgePoint::CENTRE);
makeEdge(es,eps);
addToPath(eps,vs[11],EdgePoint::CENTRE);
addToPath(eps,vs[10],EdgePoint::BL);
addToPath(eps,vs[10],EdgePoint::BR);
addToPath(eps,vs[10],EdgePoint::TR);
addToPath(eps,vs[9],EdgePoint::CENTRE);
makeEdge(es,eps);
addToPath(eps,vs[15],EdgePoint::CENTRE);
addToPath(eps,vs[14],EdgePoint::CENTRE);
makeEdge(es,eps);
addToPath(eps,vs[14],EdgePoint::CENTRE);
addToPath(eps,vs[11],EdgePoint::CENTRE);
makeEdge(es,eps);
addToPath(eps,vs[9],EdgePoint::CENTRE);
addToPath(eps,vs[14],EdgePoint::CENTRE);
makeEdge(es,eps);
addToPath(eps,vs[10],EdgePoint::CENTRE);
addToPath(eps,vs[9],EdgePoint::CENTRE);
makeEdge(es,eps);
addToPath(eps,vs[11],EdgePoint::CENTRE);
addToPath(eps,vs[10],EdgePoint::CENTRE);
makeEdge(es,eps);
addToPath(eps,vs[0],EdgePoint::CENTRE);
addToPath(eps,vs[15],EdgePoint::CENTRE);
makeEdge(es,eps);
addToPath(eps,vs[1],EdgePoint::CENTRE);
addToPath(eps,vs[0],EdgePoint::CENTRE);
makeEdge(es,eps);
addToPath(eps,vs[2],EdgePoint::CENTRE);
addToPath(eps,vs[15],EdgePoint::CENTRE);
makeEdge(es,eps);
addToPath(eps,vs[1],EdgePoint::CENTRE);
addToPath(eps,vs[0],EdgePoint::TL);
addToPath(eps,vs[0],EdgePoint::BL);
addToPath(eps,vs[0],EdgePoint::BR);
addToPath(eps,vs[2],EdgePoint::CENTRE);
makeEdge(es,eps);

    const size_t V = vs.size();
    writeFile(vs,es,"kamada-0.svg");

    project::Constraints cs;
    TopologyConstraints t(cola::HORIZONTAL,vs,es,cs);

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
        ss << "kamada-" << i << ".svg";
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
