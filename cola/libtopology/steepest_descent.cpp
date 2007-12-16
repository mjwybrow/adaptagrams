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
#include "log.h"
using namespace std;
namespace topology {

/**
 * compute the stress:
 * \f[
 *   \sigma = \sum_{e \in E} \left( d_e - \sum_{s \in S(e)} |s| \right)^2
 * \f]
 */
double TopologyConstraints::computeStress() const {
    return sum_over(edges.begin(),edges.end(),0.0,ComputeStress());
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
        double minTAlpha=DBL_MAX;
        TopologyConstraint* minT=NULL;
        // find minimum feasible alpha over all topology constraints
        for(vector<TopologyConstraint*>::iterator i=ts.begin();
                i!=ts.end();++i) {
            TopologyConstraint* t=*i;
            double tAlpha=t->c->maxSafeAlpha();
            if(tAlpha>0.00001 && tAlpha<minTAlpha) {
                minTAlpha=tAlpha;
                minT=t;
            }
        }
        // if minTAlpha<alpha move all by minTAlpha 
        // and throw interrupt exception
        if(minTAlpha<alpha) {
            for_each(vs.begin(),vs.end(),
                    bind2nd(mem_fun(&project::Variable::moveBy),minTAlpha));
            minT->satisfy();
            throw InterruptException();
        }
    }
    project::Variables& vs;
    vector<TopologyConstraint*>& ts;
};
void TopologyConstraints::
steepestDescent(valarray<double>& g, cola::SparseMap& h) {
    steepestDescent(g,h,DesiredPositions());
}
void TopologyConstraints::
steepestDescent(valarray<double>& g, cola::SparseMap& h, const DesiredPositions& d=DesiredPositions()) {
    assert(g.size()==n);
    assert(h.n==n);
    computeForces(g,h);
    cola::SparseMatrix H(h);
    double stepSize = computeStepSize(H,g,g);
    project::Variables vars(n);
    for(unsigned i=0;i<n;++i) {
        Node* node=nodes[i];
        vpsc::Rectangle* r=node->rect;
        project::Variable* v=node->var;
        v->setDesiredPosition(project::Desired(r->getCentreD(dim)-g[i]*stepSize));
        vars[i]=v;
    }
    for(DesiredPositions::const_iterator i=d.begin();i!=d.end();++i) {
        project::Variable* v = vars[i->first];
        v->setDesiredPosition(i->second);
        v->setWeight(project::Weight(1e10));
    }
    vector<TopologyConstraint*> ts;
    constraints(ts);
    project::Project p(vars,cs);
    AlphaCheck a(vars,ts);
    p.setExternalAlphaCheck(&a);
    try {
        p.solve();
    } catch(InterruptException& e) {
        FILE_LOG(logDEBUG)<<"finished early!\n";
    } catch(project::CriticalFailure& f) {
        f.print();
        exit(1);
    }
    for(unsigned i=0;i<n;i++) {
        nodes[i]->rect->moveCentreD(dim,vars[i]->getPosition());
        //vpsc::Rectangle* r=nodes[i]->rect;
     //   cout << "Rectangle: " << *r << "C(" << r->getCentreX() << "," << r->getCentreY() << ")" << endl;
        for(Edges::iterator e=edges.begin();e!=edges.end();++e) {
            (*e)->forEachEdgePoint(mem_fun(&EdgePoint::setPos));
        }
    }
}
} // namespace topology
// vim: cindent ts=4 sw=4 et tw=0 wm=0
