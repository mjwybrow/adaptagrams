/**
 * \file bend2.cpp
 * \author Tim Dwyer
 * \date Dec 2007
 *
 * Really basic test of topology conserving constraints.  Sets up a graph
 * with three nodes and one edge, initially with two bends around the
 * disconnected node.
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

struct InterruptException {
};
struct AlphaCheck : project::ExternalAlphaCheck {
    AlphaCheck(project::Variables& vs, vector<TopologyConstraint*>& ts) 
        : vs(vs), ts(ts) {}
    void operator()(double alpha) {
        printf("AlphaCheck: %f\n",alpha);
        double minTAlpha=DBL_MAX;
        TopologyConstraint* minT=NULL;
        // find minimum feasible alpha over all topology constraints
        for(vector<TopologyConstraint*>::iterator i=ts.begin();
                i!=ts.end();++i) {
            TopologyConstraint* t=*i;
            double tAlpha=t->c->maxSafeAlpha();
            printf("  TopologyConstraint %p alpha: %f\n",t,tAlpha);
            if(tAlpha>=0 && tAlpha<minTAlpha) {
                minTAlpha=tAlpha;
                minT=t;
                printf("  violated TopologyConstraint at: %f\n",minTAlpha);
            }
        }
        // if minTAlpha<alpha move all by minTAlpha 
        // and throw interrupt exception
        if(minTAlpha<alpha) {
            for_each(vs.begin(),vs.end(),
                    bind2nd(mem_fun(&project::Variable::moveBy),minTAlpha));
            /*
            for(vector<TopologyConstraint*>::iterator i=ts.begin();
                    i!=ts.end();++i) {
                TopologyConstraint* t=*i;
                //t->print();
                printf("  TopologyConstraint %p slack: %f\n",t,t->slack());
            }
            */
            minT->satisfy();
            throw InterruptException();
        }
    }
    project::Variables& vs;
    vector<TopologyConstraint*>& ts;
};
void steepestDescent(
        TopologyConstraints& t,
        valarray<double>& expectedG, 
        valarray<double>& expectedH) {
    unsigned n=vs.size();
    valarray<double> g(n);
    cola::SparseMap h(n);
    t.computeForces(h,g);
    cola::SparseMatrix H(h);
    double stepSize = computeStepSize(H,g,g);
    printf("stepSize=%f\n",stepSize);
    for(unsigned i=0;i<n;++i) {
        vpsc::Rectangle* r=rs[i];
        project::Variable* v=vars[i];
        v->d=r->getCentreX()-g[i]*stepSize;
    }
    vector<TopologyConstraint*> ts;
    t.constraints(ts);
    printf("Have %d topology constraints!\n",(int)ts.size());
    assert(ts.size()==2);
    project::Project p(vars,cs);
    AlphaCheck a(vars,ts);
    p.setExternalAlphaCheck(&a);
    try {
        p.solve();
    } catch(InterruptException& e) {
        printf("finished early!\n");
    }
    for(unsigned i=0;i<n;i++) {
        rs[i]->moveCentreX(vars[i]->x);
        for(Edges::iterator e=es.begin();e!=es.end();++e) {
            (*e)->forEachEdgePoint(mem_fun(&EdgePoint::setPos));
        }
    }
}

void write_file(const char* outputFileName) {
    vector<cola::Edge> cedges;
    cedges.push_back(make_pair(1,2));

    vector<straightener::Route*> routes;
    for(vector<Edge*>::iterator e=es.begin();e!=es.end();++e) {
        routes.push_back((*e)->getRoute());
    }

    vector<string> labels(vs.size());
    for(unsigned i=0;i<vs.size();++i) {
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

void bend2() {
    printf("test: bend2()\n");
	const size_t V = 3;

    addNode(571.5,363.5,63,43);
    addNode(541.5,469.5,63,43);
    addNode(541.5,300.5,63,43);
    addToPath(vs[1],EdgePoint::CENTRE);
    addToPath(vs[0],EdgePoint::TR);
    addToPath(vs[0],EdgePoint::BR);
    addToPath(vs[2],EdgePoint::CENTRE);

    es.push_back(new Edge(100,ps));

    write_file("bend2-0.svg");

    TopologyConstraints t(cola::HORIZONTAL,cs,vs,es);

    // test computeStress
    double stress=t.computeStress();
    printf("Stress=%f\n",stress);
    assert(fabs(expectedStress-stress)<1e-4);

    valarray<double> eg(V);
    copy(expectedG1,expectedG1+V,&eg[0]);
    valarray<double> eh(V*V);
    copy(expectedH1,expectedH1+V*V,&eh[0]);
    steepestDescent(t, eg, eh);
    write_file("bend2-1.svg");
    copy(expectedG2,expectedG2+V,&eg[0]);
    copy(expectedH2,expectedH2+V*V,&eh[0]);
    steepestDescent(t, eg, eh);
    write_file("bend2-2.svg");

    for_each(rs.begin(),rs.end(),delete_object());
    for_each(vs.begin(),vs.end(),delete_object());
    for_each(vars.begin(),vars.end(),delete_object());
    for_each(es.begin(),es.end(),delete_object());
}

int main() {
    bend2();
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
  
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=4:softtabstop=4:encoding=utf-8:textwidth=80 :
