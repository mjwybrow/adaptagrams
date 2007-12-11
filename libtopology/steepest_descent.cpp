/**
 * Methods associated with computing a descent vector for the topology stress function.
 *
 * \file steepest_descent.cpp
 * \author Tim Dwyer
 * \date Dec 2007
 */
#include <vector>
#include <libcola/cola.h>
#include <libvpsc/rectangle.h>
#include <libproject/project.h>
#include "topology_graph.h"
#include "topology_constraints.h"
using namespace std;
namespace topology {

/**
 * compute the stress:
 * \f[
 *   \sigma = \sum_{e \in E} \left( d_e - \sum_{s \in S(e)} |s| \right)^2
 * \f]
 */
double TopologyConstraints::computeStress() const {
    double stress=0;
    for(Edges::const_iterator i=edges.begin();i!=edges.end();++i) {
        Edge* e=*i;
        double d = e->idealLength;
        double weight=1.0/(d*d);
        //printf("pathLength=%f\n",e->pathLength());
        double sqrtf=fabs(d-e->pathLength());
        stress+=weight*sqrtf*sqrtf;
    }
    return stress;
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
            if(tAlpha>=0.0001 && tAlpha<minTAlpha) {
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
void TopologyConstraints::
steepestDescent(valarray<double>& g, cola::SparseMap& h) {
    assert(g.size()==n);
    assert(h.n==n);
    computeForces(g,h);
    for(unsigned i=0;i<n;++i) {
        //printf("g[%d]=%f,eg[%d]=%f\n",i,g[i],i,expectedG[i]);
        //assert(fabs(g[i]-expectedG[i])<1e-4);
        for(unsigned j=0;j<n;++j) {
            //printf("h[%d,%d]=%f,eh[%d,%d]=%f\n",i,j,h(i,j),i,j,expectedH[i*n+j]);
            //printf("%f,\n",h(i,j));
            //assert(fabs(h(i,j)-expectedH[i*n+j])<1e-4);
        }
    }
    cola::SparseMatrix H(h);
    double stepSize = computeStepSize(H,g,g);
    printf("stepSize=%f\n",stepSize);
    project::Variables vars(n);
    for(unsigned i=0;i<n;++i) {
        Node* node=nodes[i];
        vpsc::Rectangle* r=node->rect;
        project::Variable* v=node->var;
        v->d=r->getCentreX()-g[i]*stepSize;
        vars[i]=v;
    }
    vector<TopologyConstraint*> ts;
    constraints(ts);
    project::Project p(vars,cs);
    AlphaCheck a(vars,ts);
    p.setExternalAlphaCheck(&a);
    try {
        p.solve();
    } catch(InterruptException& e) {
        printf("finished early!\n");
    }
    for(unsigned i=0;i<n;i++) {
        nodes[i]->rect->moveCentreX(vars[i]->x);
        for(Edges::iterator e=edges.begin();e!=edges.end();++e) {
            (*e)->forEachEdgePoint(mem_fun(&EdgePoint::setPos));
        }
    }
}
} // namespace topology
// vim: cindent ts=4 sw=4 et tw=0 wm=0
