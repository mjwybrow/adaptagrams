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
void test1(Nodes& vs, EdgePoints& ps, DesiredPositions& d, string& name) {
    name=string("test1");
    addNode(vs,400,170,50,30);
    addNode(vs,420,65,50,30);
    addNode(vs,280,220,50,30);
    addToPath(ps,vs[2],EdgePoint::RectIntersect(4));
    addToPath(ps,vs[1],EdgePoint::RectIntersect(4));
    d.push_back(make_pair(0,361.000000));
}
void test2(Nodes& vs, EdgePoints& ps, DesiredPositions& d, string& name) {
    name=string("test2");
    addNode(vs,0,0,54,34);
    addNode(vs,100,100,54,34);
    addNode(vs,0,50,54,34);
    addToPath(ps,vs[0],EdgePoint::RectIntersect(4));
    addToPath(ps,vs[1],EdgePoint::RectIntersect(4));
    d.push_back(make_pair(2,150));
}
void test3(Nodes& vs, EdgePoints& ps, DesiredPositions& d, string& name) {
    name=string("test3");
    addNode(vs,0,0,54,34);
    addNode(vs,100,100,54,34);
    addNode(vs,100,50,54,34);
    addToPath(ps,vs[0],EdgePoint::RectIntersect(4));
    addToPath(ps,vs[1],EdgePoint::RectIntersect(4));
    d.push_back(make_pair(2,0));
}
void test4(Nodes& vs, EdgePoints& ps, DesiredPositions& d, string& name) {
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
void simple(void test(Nodes&, EdgePoints&, DesiredPositions&,string&)){
    Nodes nodes;
    EdgePoints ps;
    DesiredPositions d;
    string name;
    test(nodes,ps,d,name);
    const size_t V = nodes.size();
    Edges es;
    es.push_back(new Edge(210,ps));

    writeFile(nodes,es,"simple-"+name+"-0.svg");

    vpsc::Variables vs;
    vpsc::Constraints cs;
    for(unsigned i=0;i<V;++i) {
        vs.push_back(new vpsc::Variable(i));
    }

    { // scope for t, so that t gets destroyed before es
        TopologyConstraints t(cola::HORIZONTAL,nodes,es,vs,cs);

        // test computeStress
        double stress=t.computeStress();
        printf("Stress=%f\n",stress);
        //assert(fabs(expectedStress-stress)<1e-4);

        double gradient[]={0,3.03652e-19,-3.03652e-19};
        valarray<double> g(V);
        for(unsigned i=0;i<V;i++) {
            g[i]=gradient[i];
        }
        cola::SparseMap h(V);
        for(unsigned i=1;i<2;i++) {
            //g=0;
            h.clear();
            t.gradientProjection(g,h,d);
            stringstream ss;
            ss << "simple-" << name << "-" << i << ".svg";
            writeFile(nodes,es,ss.str());
        }
    }

    for(Nodes::iterator i=nodes.begin();i!=nodes.end();++i) {
        Node* v=*i;
        delete v->rect;
        delete v;
    }
    for_each(es.begin(),es.end(),delete_object());
    for_each(cs.begin(),cs.end(),delete_object());
    for_each(vs.begin(),vs.end(),delete_object());
}

int main() {
    simple(test1);
    simple(test2);
    simple(test3);
    simple(test4);
    return 0;
}
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=4:softtabstop=4:encoding=utf-8:textwidth=80 :
