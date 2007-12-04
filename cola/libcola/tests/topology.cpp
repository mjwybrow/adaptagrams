#include <vector>
#include <iostream>
#include <libvpsc/rectangle.h>
#include <libcola/topology_constraints.h>
#include <libcola/cola.h>
#include <libcola/output_svg.h>
#include <libproject/project.h>
using namespace std;
using namespace topology;

Nodes vs;
project::Variables vars;
project::Constraints cs;
vector<vpsc::Rectangle*> rs;
EdgePoints ps;
Edges es;
double expectedStress=1.05721;
double expectedG[]={0.0143283,0.00271783,-0.0170462};
double expectedH[][3]={{0.000340898, -0.000138663, -0.000202235}, 
                       {-0.000138663, 0.000160573, -0.0000219108}, 
                       {-0.000202235, -0.0000219108, 0.000224146}};

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
double computeStepSize(
        cola::SparseMatrix const &H, 
        valarray<double> const &g, 
        valarray<double> const &d);

void alphaCheck(double a) {
    printf("Hi from alphaCheck: %f\n",a);
}

void simpleBend() {
    printf("test: simpleBend()\n");
	const unsigned V = 3;
    const char * ls[]={"0","1","2"};
    vector<const char *> labels(ls,ls+V);

    addNode(571.500000,363.500000,63.000000,43.000000);
    addNode(620.0,469.500000,63.000000,43.000000);
    addNode(541.500000,300.500000,63.000000,43.000000);
    addToPath(vs[1],EdgePoint::CENTRE);
    addToPath(vs[0],EdgePoint::BR);
    addToPath(vs[2],EdgePoint::CENTRE);

    Edge *e = new Edge(100,ps);
    es.push_back(e);

    vector<straightener::Route*> routes;
    routes.push_back(e->getRoute());

    TopologyConstraints t(cola::HORIZONTAL,vs,es);

    // this simple case shouldn't have generated any topology constraints
    unsigned ctr=0;
    for(Segments::iterator i=e->segments.begin();i!=e->segments.end();++i) {
        Segment* s=*i;
        ctr+=s->topologyConstraints.size();
    }
    assert(ctr==1);
    printf(" %d topology constraints created!\n",ctr);

    // test computeStress
    double stress=t.computeStress();
    assert(fabs(expectedStress-stress)<1e-4);

    unsigned n=vs.size();
    valarray<double> g(n);
    cola::SparseMap h(n);
    t.computeForces(h,g);
    for(unsigned i=0;i<n;++i) {
        assert(fabs(g[i]-expectedG[i])<1e-4);
        for(unsigned j=0;j<n;++j) {
            assert(fabs(h(i,j)-expectedH[i][j])<1e-4);
        }
    }
    cola::SparseMatrix H(h);
    double stepSize = computeStepSize(H,g,g);
    printf("stepSize=%f\n",stepSize);
    for(unsigned i=0;i<n;++i) {
        vpsc::Rectangle* r=rs[i];
        project::Variable* v=vars[i];
        v->d=r->getCentreX()-g[i]*stepSize;
    }
    project::Project p(vars,cs);
    p.setExternalAlphaCheck(alphaCheck);
    p.solve();
    for(unsigned i=0;i<n;i++) {
        rs[i]->moveCentreX(vars[i]->x);
    }

    vector<cola::Edge> cedges;
    cedges.push_back(make_pair(1,2));

    OutputFile of(rs,cedges,NULL,"simpleBend.svg",true,false);
    of.setLabels(3,ls);
    of.routes=&routes;
    of.generate();

    for_each(ps.begin(),ps.end(),delete_object());
    for_each(rs.begin(),rs.end(),delete_object());
    for_each(vs.begin(),vs.end(),delete_object());
    for_each(vars.begin(),vars.end(),delete_object());
    for_each(es.begin(),es.end(),delete_object());
    for_each(routes.begin(),routes.end(),delete_object());
}

int main() {
    simpleBend();
    return 0;
}
inline double dotProd(valarray<double> x, valarray<double> y) {
    assert(x.size()==y.size());
    double dp=0;
    for(unsigned i=0;i<x.size();i++) {
        dp+=x[i]*y[i]; 
    }
    return dp;
}
double computeStepSize(
        cola::SparseMatrix const &H, 
        valarray<double> const &g, 
        valarray<double> const &d) {
    assert(g.size()==d.size());
    assert(g.size()==H.rowSize());
    // stepsize = g'd / (d' H d)
    double numerator = dotProd(g,d);
    valarray<double> Hd(d.size());
    H.rightMultiply(d,Hd);
    double denominator = dotProd(d,Hd);
    //assert(numerator>=0);
    //assert(denominator>=0);
    if(denominator==0) return 0;
    return numerator/denominator;
}
  
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=4:softtabstop=4:encoding=utf-8:textwidth=99 :
