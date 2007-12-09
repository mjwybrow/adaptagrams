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

double expectedStress=1.05721;
double expectedG1[]={0.0143283,0.00271783,-0.0170462};
double expectedG2[]={0.0,0.004044,-0.004044};
double expectedH1[]={0.000340898, -0.000138663, -0.000202235, 
                    -0.000138663, 0.000160573, -0.0000219108, 
                    -0.000202235, -0.0000219108, 0.000224146};
double expectedH2[]={0.000000, 0.000000, 0.000000,
                     0.000000, 0.000094, -0.000094,
                     0.000000, -0.000094, 0.000094};

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
        valarray<double>& expectedH,
        const char* outputFileName) {
    unsigned n=vs.size();
    valarray<double> g(n);
    cola::SparseMap h(n);
    t.computeForces(h,g);
    for(unsigned i=0;i<n;++i) {
        //printf("g[%d]=%f,eg[%d]=%f\n",i,g[i],i,expectedG[i]);
        assert(fabs(g[i]-expectedG[i])<1e-4);
        for(unsigned j=0;j<n;++j) {
            //printf("h[%d,%d]=%f,eh[%d,%d]=%f\n",i,j,h(i,j),i,j,expectedH[i*n+j]);
            //printf("%f,\n",h(i,j));
            assert(fabs(h(i,j)-expectedH[i*n+j])<1e-4);
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

    vector<cola::Edge> cedges;
    cedges.push_back(make_pair(1,2));

    vector<straightener::Route*> routes;
    for(vector<Edge*>::iterator e=es.begin();e!=es.end();++e) {
        routes.push_back((*e)->getRoute());
    }

    vector<string> labels(n);
    for(unsigned i=0;i<n;++i) {
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

void simpleBend() {
    printf("test: simpleBend()\n");
	const size_t V = 3;

    addNode(571.500000,363.500000,63.000000,43.000000);
    addNode(620.0,469.500000,63.000000,43.000000);
    addNode(541.500000,300.500000,63.000000,43.000000);
    addToPath(vs[1],EdgePoint::CENTRE);
    addToPath(vs[0],EdgePoint::BR);
    addToPath(vs[2],EdgePoint::CENTRE);

    es.push_back(new Edge(100,ps));

    TopologyConstraints t(cola::HORIZONTAL,cs,vs,es);

    // test computeStress
    double stress=t.computeStress();
    assert(fabs(expectedStress-stress)<1e-4);

    valarray<double> eg(V);
    copy(expectedG1,expectedG1+V,&eg[0]);
    valarray<double> eh(V*V);
    copy(expectedH1,expectedH1+V*V,&eh[0]);
    steepestDescent(t, eg, eh, "simpleBend1.svg");
    copy(expectedG2,expectedG2+V,&eg[0]);
    copy(expectedH2,expectedH2+V*V,&eh[0]);
    steepestDescent(t, eg, eh, "simpleBend2.svg");

    for_each(rs.begin(),rs.end(),delete_object());
    for_each(vs.begin(),vs.end(),delete_object());
    for_each(vars.begin(),vars.end(),delete_object());
    for_each(es.begin(),es.end(),delete_object());
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
  
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=4:softtabstop=4:encoding=utf-8:textwidth=80 :
