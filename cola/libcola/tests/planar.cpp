/*
 * vim: ts=4 sw=4 et tw=0 wm=0
 *
 * libcola - A library providing force-directed network layout using the 
 *           stress-majorization method subject to separation constraints.
 *
 * Copyright (C) 2006-2008  Monash University
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

/** \file topology.cpp
 *
 * tests topology preserving layout.  graph is a triangle.
 * We have a fourth disconnected node starting inside the triangle.
 * We give the disconnected node a desired position outside the triangle
 * and begin layout.  Layout should converge to near zero stress as all
 * edges should be able to reach their desired lengths, and disconnected
 * node should still be inside the triangle at the end.
 */
/*
* Authors:
*   Tim Dwyer <tgdwyer@gmail.com>
*/
#include <iostream>
#include <iomanip>
#include <fstream>

#include <vector>
#include <valarray>
#include <algorithm>
#include <float.h>
#include <libcola/cola.h>
#include <libtopology/topology_graph.h>
#include "graphlayouttest.h"
using namespace std;
using namespace cola;

struct TestCase {
    topology::Nodes vs;
    topology::Edges tes;
    topology::EdgePoints ps;
    vector<vpsc::Rectangle*> rs;
    vector<Edge> es;
    void addNode(double minX, double minY, double w, double h) {
        vpsc::Rectangle* r = new Rectangle(minX, minX+w, minY, minY+h);
        rs.push_back(r);
        topology::Node *v = new topology::Node(vs.size(), r);
        vs.push_back(v);
    }
    void addToPath(unsigned vID, topology::EdgePoint::RectIntersect i) {
        ps.push_back(new topology::EdgePoint(vs[vID],i));
    }
    void addEdge(double l) {
        tes.push_back(new topology::Edge(l, ps));
        es.push_back(make_pair(ps[0]->node->id,ps[ps.size()-1]->node->id));
        ps.clear();
    }
    ~TestCase() {
        for_each(rs.begin(),rs.end(),delete_object());
        for_each(tes.begin(),tes.end(),delete_object());
        for_each(vs.begin(),vs.end(),delete_object());
    }
};
void writeFile(const topology::Nodes& vs, const topology::Edges& es, const char *outputFileName) {
    const unsigned n=vs.size();
    vector<cola::Edge> cedges;

    for(unsigned i=0;i<es.size();i++) {
        cedges.push_back(make_pair(1,2));
    }

    vector<straightener::Route*> routes;
    for(topology::Edges::const_iterator e=es.begin();e!=es.end();++e) {
        routes.push_back((*e)->getRoute());
    }

    vector<string> labels(n);
    for(unsigned i=0;i<n;++i) {
        stringstream ss;
        ss << i;
        labels[i]=ss.str();
    }

    vector<vpsc::Rectangle*> rs;
    for(topology::Nodes::const_iterator i=vs.begin();i!=vs.end();++i) {
	    rs.push_back((*i)->rect);
    }
    OutputFile of(rs,cedges,nullptr,outputFileName,true,false);
    of.setLabels(labels);
    of.routes=&routes;
    of.generate();

    for_each(routes.begin(),routes.end(),delete_object());
}
struct Test : TestConvergence {
    Test(const double d,const unsigned i,topology::Nodes& vs, topology::Edges& es) : TestConvergence(d,i), vs(vs), es(es) {}
    bool operator()(const double new_stress, valarray<double> & X, valarray<double> & Y) {
        cout << "stress="<<new_stress<<" iteration="<<iterations<<endl;
        stringstream ss;
        ss << "planar-" << setfill('0') << setw(3) << ++iterations << ".svg";
        writeFile(vs,es,ss.str().c_str());
        if(iterations<100) {
            return false;
        }
        return true;
        return TestConvergence::operator()(new_stress,X,Y);
    }
    double lastStress;
    topology::Nodes& vs;
    topology::Edges& es;
};
void test() {
    TestCase t;
t.addNode(144.000000,511.000000,8.000000,8.000000);
t.addNode(413.000000,155.000000,8.000000,8.000000);
t.addNode(437.000000,169.000000,8.000000,8.000000);
t.addNode(436.000000,122.000000,8.000000,8.000000);
t.addNode(460.000000,150.000000,8.000000,8.000000);
t.addNode(466.000000,178.000000,8.000000,8.000000);
t.addNode(469.000000,198.000000,8.000000,8.000000);
t.addNode(378.000000,332.000000,8.000000,8.000000);
t.addNode(368.000000,315.000000,8.000000,8.000000);
t.addNode(332.000000,341.000000,8.000000,8.000000);
t.addNode(282.000000,374.000000,8.000000,8.000000);
t.addNode(329.000000,364.000000,8.000000,8.000000);
t.addNode(346.000000,390.000000,8.000000,8.000000);
t.addNode(375.000000,402.000000,8.000000,8.000000);
t.addNode(357.000000,438.000000,8.000000,8.000000);
t.addNode(382.000000,422.000000,8.000000,8.000000);
t.addNode(453.000000,280.000000,8.000000,8.000000);
t.addNode(452.000000,258.000000,8.000000,8.000000);
t.addNode(530.000000,449.000000,8.000000,8.000000);
t.addNode(576.000000,431.000000,8.000000,8.000000);
t.addNode(577.000000,408.000000,8.000000,8.000000);
t.addNode(606.000000,438.000000,8.000000,8.000000);
t.addNode(624.000000,467.000000,8.000000,8.000000);
t.addNode(620.000000,492.000000,8.000000,8.000000);
t.addNode(681.000000,492.000000,8.000000,8.000000);
t.addNode(729.000000,528.000000,8.000000,8.000000);
t.addNode(765.000000,515.000000,8.000000,8.000000);
t.addNode(765.000000,492.000000,8.000000,8.000000);
t.addNode(858.000000,538.000000,8.000000,8.000000);
t.addNode(2.000000,544.000000,8.000000,8.000000);
t.addToPath(28,(topology::EdgePoint::RectIntersect)4);
t.addToPath(27,(topology::EdgePoint::RectIntersect)4);
t.addEdge(70.000000);
t.addToPath(28,(topology::EdgePoint::RectIntersect)4);
t.addToPath(26,(topology::EdgePoint::RectIntersect)4);
t.addEdge(70.000000);
t.addToPath(25,(topology::EdgePoint::RectIntersect)4);
t.addToPath(28,(topology::EdgePoint::RectIntersect)4);
t.addEdge(70.000000);
t.addToPath(25,(topology::EdgePoint::RectIntersect)4);
t.addToPath(29,(topology::EdgePoint::RectIntersect)4);
t.addEdge(70.000000);
t.addToPath(17,(topology::EdgePoint::RectIntersect)4);
t.addToPath(24,(topology::EdgePoint::RectIntersect)4);
t.addEdge(70.000000);
t.addToPath(16,(topology::EdgePoint::RectIntersect)4);
//t.addToPath(20,(topology::EdgePoint::RectIntersect)1);
t.addToPath(24,(topology::EdgePoint::RectIntersect)4);
t.addEdge(70.000000);
t.addToPath(16,(topology::EdgePoint::RectIntersect)4);
t.addToPath(7,(topology::EdgePoint::RectIntersect)4);
t.addEdge(70.000000);
t.addToPath(24,(topology::EdgePoint::RectIntersect)4);
t.addToPath(25,(topology::EdgePoint::RectIntersect)4);
t.addEdge(70.000000);
t.addToPath(22,(topology::EdgePoint::RectIntersect)4);
t.addToPath(24,(topology::EdgePoint::RectIntersect)4);
t.addEdge(70.000000);
t.addToPath(22,(topology::EdgePoint::RectIntersect)4);
t.addToPath(0,(topology::EdgePoint::RectIntersect)4);
t.addEdge(70.000000);
t.addToPath(22,(topology::EdgePoint::RectIntersect)4);
t.addToPath(23,(topology::EdgePoint::RectIntersect)4);
t.addEdge(70.000000);
t.addToPath(22,(topology::EdgePoint::RectIntersect)4);
t.addToPath(18,(topology::EdgePoint::RectIntersect)4);
t.addEdge(70.000000);
t.addToPath(21,(topology::EdgePoint::RectIntersect)4);
t.addToPath(20,(topology::EdgePoint::RectIntersect)4);
t.addEdge(70.000000);
t.addToPath(20,(topology::EdgePoint::RectIntersect)4);
t.addToPath(19,(topology::EdgePoint::RectIntersect)4);
t.addEdge(70.000000);
t.addToPath(18,(topology::EdgePoint::RectIntersect)4);
t.addToPath(20,(topology::EdgePoint::RectIntersect)4);
t.addEdge(70.000000);
t.addToPath(18,(topology::EdgePoint::RectIntersect)4);
t.addToPath(16,(topology::EdgePoint::RectIntersect)4);
t.addEdge(70.000000);
t.addToPath(14,(topology::EdgePoint::RectIntersect)4);
t.addToPath(18,(topology::EdgePoint::RectIntersect)4);
t.addEdge(70.000000);
t.addToPath(13,(topology::EdgePoint::RectIntersect)4);
t.addToPath(14,(topology::EdgePoint::RectIntersect)4);
t.addEdge(70.000000);
t.addToPath(15,(topology::EdgePoint::RectIntersect)4);
t.addToPath(14,(topology::EdgePoint::RectIntersect)4);
t.addEdge(70.000000);
t.addToPath(12,(topology::EdgePoint::RectIntersect)4);
t.addToPath(13,(topology::EdgePoint::RectIntersect)4);
t.addEdge(70.000000);
t.addToPath(12,(topology::EdgePoint::RectIntersect)4);
t.addToPath(10,(topology::EdgePoint::RectIntersect)4);
t.addEdge(70.000000);
t.addToPath(11,(topology::EdgePoint::RectIntersect)4);
t.addToPath(12,(topology::EdgePoint::RectIntersect)4);
t.addEdge(70.000000);
t.addToPath(10,(topology::EdgePoint::RectIntersect)4);
t.addToPath(11,(topology::EdgePoint::RectIntersect)4);
t.addEdge(70.000000);
t.addToPath(9,(topology::EdgePoint::RectIntersect)4);
t.addToPath(13,(topology::EdgePoint::RectIntersect)4);
t.addEdge(70.000000);
t.addToPath(9,(topology::EdgePoint::RectIntersect)4);
t.addToPath(10,(topology::EdgePoint::RectIntersect)4);
t.addEdge(70.000000);
t.addToPath(6,(topology::EdgePoint::RectIntersect)4);
//t.addToPath(8,(topology::EdgePoint::RectIntersect)2);
t.addToPath(9,(topology::EdgePoint::RectIntersect)4);
t.addEdge(70.000000);
t.addToPath(6,(topology::EdgePoint::RectIntersect)4);
t.addToPath(7,(topology::EdgePoint::RectIntersect)4);
t.addEdge(70.000000);
t.addToPath(8,(topology::EdgePoint::RectIntersect)4);
t.addToPath(7,(topology::EdgePoint::RectIntersect)4);
t.addEdge(70.000000);
t.addToPath(6,(topology::EdgePoint::RectIntersect)4);
t.addToPath(25,(topology::EdgePoint::RectIntersect)4);
t.addEdge(70.000000);
t.addToPath(5,(topology::EdgePoint::RectIntersect)4);
t.addToPath(10,(topology::EdgePoint::RectIntersect)4);
t.addEdge(70.000000);
t.addToPath(5,(topology::EdgePoint::RectIntersect)4);
t.addToPath(29,(topology::EdgePoint::RectIntersect)4);
t.addEdge(70.000000);
t.addToPath(29,(topology::EdgePoint::RectIntersect)4);
t.addToPath(1,(topology::EdgePoint::RectIntersect)4);
t.addEdge(70.000000);
t.addToPath(3,(topology::EdgePoint::RectIntersect)4);
t.addToPath(2,(topology::EdgePoint::RectIntersect)4);
t.addEdge(70.000000);
t.addToPath(5,(topology::EdgePoint::RectIntersect)4);
t.addToPath(6,(topology::EdgePoint::RectIntersect)4);
t.addEdge(70.000000);
t.addToPath(4,(topology::EdgePoint::RectIntersect)4);
t.addToPath(5,(topology::EdgePoint::RectIntersect)4);
t.addEdge(70.000000);
t.addToPath(3,(topology::EdgePoint::RectIntersect)4);
t.addToPath(4,(topology::EdgePoint::RectIntersect)4);
t.addEdge(70.000000);
t.addToPath(29,(topology::EdgePoint::RectIntersect)4);
//t.addToPath(1,(topology::EdgePoint::RectIntersect)2);
t.addToPath(3,(topology::EdgePoint::RectIntersect)4);
t.addEdge(70.000000);
    writeFile(t.vs,t.tes,"planar-000.svg");
    Test test(0.00001,100,t.vs,t.tes);
    ConstrainedFDLayout alg(t.rs,t.es,70.0,nullptr,test,nullptr);
    alg.setTopology(&t.vs,&t.tes);
    alg.run(true,true);
    double finalStress=alg.computeStress();
    printf("finalStress=%f\n",finalStress);

    //assert(finalStress<1e-5);
}
int main() {
    test();
    return 0;
}
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=4:softtabstop=4:textwidth=80 :
