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

Nodes vs;
project::Variables vars;
project::Constraints cs;
vector<vpsc::Rectangle*> rs;
EdgePoints ps;
Edges es;

Node* addNode(double x, double y, double w, double h) {
    vpsc::Rectangle* r = new vpsc::Rectangle(x,x+w,y,y+h);
    project::Variable* var=new project::Variable(r->getCentreX(),-1);
	Node *v = new Node(vs.size(), r, var);
    vars.push_back(var);
    vs.push_back(v);
    rs.push_back(r);
    return v;
}
void addToPath(Node *v, topology::EdgePoint::RectIntersect i) {
    ps.push_back(new EdgePoint(v,i));
}

void writeFile(const TopologyConstraints& t, const char *outputFileName) {
    vector<cola::Edge> cedges;

    for(unsigned i=0;i<t.edges.size();i++) {
        cedges.push_back(make_pair(1,2));
    }

    vector<straightener::Route*> routes;
    for(vector<Edge*>::iterator e=t.edges.begin();e!=t.edges.end();++e) {
        routes.push_back((*e)->getRoute());
    }

    vector<string> labels(t.n);
    for(unsigned i=0;i<t.n;++i) {
        stringstream ss;
        ss << i;
        labels[i]=ss.str();
    }
    OutputFile of(rs,cedges,NULL,outputFileName,true,false);
    of.setLabels(labels);
    of.routes=&routes;
    of.generate();

    for_each(routes.begin(),routes.end(),delete_object());
}

void check(const TopologyConstraints& t, valarray<double>& g, cola::SparseMap& h,
        double* eg, double* eh) {
    for(unsigned i=0;i<t.n;++i) {
        //printf("g[%d]=%f,eg[%d]=%f\n",i,g[i],i,expectedG[i]);
        assert(fabs(g[i]-eg[i])<1e-4);
        for(unsigned j=0;j<t.n;++j) {
            //printf("h[%d,%d]=%f,eh[%d,%d]=%f\n",i,j,h(i,j),i,j,expectedH[i*n+j]);
            //printf("%f,",h(i,j));
            assert(fabs(h(i,j)-eh[i*t.n+j])<1e-4);
        }
        printf("\n");
    }
}
void simpleBendHorizontal() {
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

    addNode(571.500000,363.500000,63.000000,43.000000);
    addNode(620.0,469.500000,63.000000,43.000000);
    addNode(541.500000,300.500000,63.000000,43.000000);
    addToPath(vs[1],EdgePoint::CENTRE);
    addToPath(vs[0],EdgePoint::BR);
    addToPath(vs[2],EdgePoint::CENTRE);

    es.push_back(new Edge(100,ps));

    TopologyConstraints t(cola::HORIZONTAL,vs,es,cs);

    // test computeStress
    double stress=t.computeStress();
    assert(fabs(expectedStress-stress)<1e-4);

    writeFile(t,"simpleBendH0.svg");

    valarray<double> g(V);
    cola::SparseMap h(V);
    g=0;
    t.steepestDescent(g,h);
    check(t,g,h,expectedG1,expectedH1);
    writeFile(t,"simpleBendH1.svg");
    g=0;
    h.clear();
    t.steepestDescent(g,h);
    check(t,g,h,expectedG2,expectedH2);
    writeFile(t,"simpleBendH2.svg");

    for_each(rs.begin(),rs.end(),delete_object());
    for_each(vs.begin(),vs.end(),delete_object());
    for_each(vars.begin(),vars.end(),delete_object());
    for_each(es.begin(),es.end(),delete_object());
}
void simpleBendVertical() {
    double expectedStress=1.2955;
    printf("test: simpleBendVertical()\n");
	const size_t V = 3;

    addNode(365,590,63.000000,43.000000);
    addNode(469,620,63.000000,43.000000);
    addNode(300,541,63.000000,43.000000);
    addToPath(vs[1],EdgePoint::CENTRE);
    addToPath(vs[0],EdgePoint::TL);
    addToPath(vs[2],EdgePoint::CENTRE);

    es.push_back(new Edge(100,ps));

    TopologyConstraints t(cola::VERTICAL,vs,es,cs);

    writeFile(t,"simpleBendV0.svg");

    // test computeStress
    double stress=t.computeStress();
    assert(fabs(expectedStress-stress)<1e-4);

    valarray<double> g(V);
    cola::SparseMap h(V);
    g=0;
    t.steepestDescent(g,h);
    //check(t,g,h,expectedG1,expectedH1);
    writeFile(t,"simpleBendV1.svg");
    g=0;
    h.clear();
    t.steepestDescent(g,h);
    //check(t,g,h,expectedG2,expectedH2);
    writeFile(t,"simpleBendV2.svg");

    for_each(rs.begin(),rs.end(),delete_object());
    for_each(vs.begin(),vs.end(),delete_object());
    for_each(vars.begin(),vars.end(),delete_object());
    for_each(es.begin(),es.end(),delete_object());
}

int main() {
    simpleBendVertical();
    return 0;
}
  
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=4:softtabstop=4:encoding=utf-8:textwidth=80 :
