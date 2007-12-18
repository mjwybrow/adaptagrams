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
#include "topology_log.h"
using namespace std;
namespace topology {

/**
 * a Functor for use in a sum_over a collection of edges
 * which computes the total stress based on the difference in total
 * pathLength for each edge and idealLength.
 * More precisely the stress for a given edge \f$e\f$ is:
 * \f[
 *   \sigma(e) \left( d_e - \sum_{s \in S(e)} |s| \right)^2
 * \f]
 */
struct ComputeStress {
    double operator()(const Edge* e) {
        double d = e->idealLength;
        double weight=1.0/(d*d);
        double dl=d-e->pathLength();
        if(dl>0) return 0;
        double s=weight*dl*dl;
        FILE_LOG(logDEBUG1)<<"e("<<e->firstSegment->start->node->id<<","<<e->lastSegment->end->node->id<<")="<<s<<std::endl;
        return s;
    }
};
/**
 * compute the stress:
 * \f[
 *   \sigma = \sum_{e \in E} \left( d_e - \sum_{s \in S(e)} |s| \right)^2
 * \f]
 */
double compute_stress(const Edges& es) {
    return sum_over(es.begin(),es.end(),0.0,ComputeStress());
}
/**
 * compute the stress:
 * \f[
 *   \sigma = \sum_{e \in E} \left( d_e - \sum_{s \in S(e)} |s| \right)^2
 * \f]
 */
double TopologyConstraints::computeStress() const {
    return compute_stress(edges);
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
ostream& operator<< (ostream& os, BendConstraint& t) {
    os<<"BendConstraint";
    return os;
}
ostream& operator<< (ostream& os, StraightConstraint& t) {
    os<<"StraightConstraint";
    return os;
}
struct TopologyConstraintInterrupt : InterruptException {
    TopologyConstraintInterrupt(TopologyConstraint* t, double alpha) 
        : violated(t), alpha(alpha) {}
    TopologyConstraint* violated;
    double alpha;
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
            t->c->tightening();
            FILE_LOG(logDEBUG1)<<"Checking topology constraint! alpha="<<tAlpha;
            FILE_LOG(logDEBUG1)<<t->toString();
            if(tAlpha>1e-7 && tAlpha<minTAlpha) {
                minTAlpha=tAlpha;
                minT=t;
            } else {
                //assert(!t->c->tightening());
            }
        }
        // if minTAlpha<alpha move all by minTAlpha 
        // and throw interrupt exception
        if(minTAlpha<alpha) {
            FILE_LOG(logDEBUG)<<"Violated topology constraint! alpha="<<minTAlpha;
            FILE_LOG(logDEBUG)<<minT->toString();
            for_each(vs.begin(),vs.end(),
                    bind2nd(mem_fun(&project::Variable::moveBy),minTAlpha));
            throw TopologyConstraintInterrupt(minT,minTAlpha);
        }
    }
    project::Variables& vs;
    vector<TopologyConstraint*>& ts;
};
TopologyConstraint* TopologyConstraints::
steepestDescent(valarray<double>& g, cola::SparseMap& h) {
    return steepestDescent(g,h,DesiredPositions());
}
struct PrintSegmentLength {
    void operator() (Segment* s) {
        printf("segment len=%f\n",s->length());
    }
};
struct PrintPoint {
    void operator() (EdgePoint* p) {
        printf(" node: %p, corner: %d\n",p->node,p->rectIntersect);
    }
};
TopologyConstraint* TopologyConstraints::
steepestDescent(valarray<double>& g, cola::SparseMap& h, const DesiredPositions& d=DesiredPositions()) {
    FILE_LOG(logDEBUG)<<"TopologyConstraints::steepestDescent... dim="<<dim;
    assert(g.size()==n);
    assert(h.n==n);
    printf("double gradient[]={");
    copy(&g[0],&g[0]+n,ostream_iterator<double>(cout,","));
    printf("}\n");
    printInstance();
    computeForces(g,h);
    cola::SparseMatrix H(h);
    double stepSize = computeStepSize(H,g,g);
    FILE_LOG(logDEBUG1)<<"stepSize="<<stepSize;
    project::Variables vars(n);
    for(unsigned i=0;i<n;++i) {
        Node* node=nodes[i];
        vpsc::Rectangle* r=node->rect;
        project::Variable* v=node->var;
        double x=r->getCentreD(dim);
        v->setPosition(project::Initial(x));
        v->setPosition(project::Desired(x-g[i]*stepSize));
        vars[i]=v;
        FILE_LOG(logDEBUG1)<<"v["<<i<<"]:init="<<v->getPosition()<<", desi="<<v->getDesiredPosition();
    }
    for(DesiredPositions::const_iterator i=d.begin();i!=d.end();++i) {
        project::Variable* v = vars[i->first];
        v->setPosition(i->second);
        v->setWeight(project::Weight(1e10));
        printf("desired.push_back(make_pair(%d,%f));\n",i->first,i->second.pos);
        FILE_LOG(logDEBUG1)<<"override desi="<<v->getDesiredPosition();
    }
    vector<TopologyConstraint*> ts;
    constraints(ts);
    project::Project p(vars,cs);
    AlphaCheck a(vars,ts);
    p.setExternalAlphaCheck(&a);
    TopologyConstraint* violated=NULL;
    try {
        p.solve();
        FILE_LOG(logDEBUG)<<"project finished with no violated topology constraints.";
    } catch(TopologyConstraintInterrupt& e) {
        FILE_LOG(logDEBUG)<<"finished early!";
        violated=e.violated;
    } catch(project::CriticalFailure& f) {
        f.print();
        exit(1);
    }
    for(Edges::iterator e=edges.begin();e!=edges.end();++e) {
        assert((*e)->assertConvexBends());
    }
    // at this point, variables have moved by some alpha 0<=alpha<=1 along the line
    // from initial positions to desired positions that is guaranteed not to
    // violate any constraints.
    for(unsigned i=0;i<n;i++) {
        //printf("old pos[%d]=%f,%f\n",i,nodes[i]->rect->getCentreX(),nodes[i]->rect->getCentreY());
        nodes[i]->rect->moveCentreD(dim,vars[i]->getPosition());
        //printf("new pos[%d]=%f,%f\n",i,nodes[i]->rect->getCentreX(),nodes[i]->rect->getCentreY());
    }
    for(Edges::iterator e=edges.begin();e!=edges.end();++e) {
        (*e)->forEachEdgePoint(mem_fun(&EdgePoint::setPos));
    }
    // rectangle and edge point positions updated to variables.
    FILE_LOG(logDEBUG)<<" moves done.";
    if(violated) {
        // now we satisfy the violated topology constraint, i.e. a bend point that
        // has become straight is removed or a segment that needs to bend is split
        violated->satisfy();
    }
    for(Edges::iterator e=edges.begin();e!=edges.end();++e) {
        assert((*e)->assertConvexBends());
    }
    FILE_LOG(logDEBUG)<<"TopologyConstraints::steepestDescent... done";
    return violated;
}
double TopologyConstraints::
reachedDesired(const DesiredPositions& d) {
    double furthest = DBL_MIN;
    for(DesiredPositions::const_iterator i=d.begin();i!=d.end();++i) {
        project::Variable* v = nodes[i->first]->var;
        furthest = max(furthest,fabs(i->second.pos-v->getPosition()));
    }
    FILE_LOG(logDEBUG)<<"max distance to desired="<<furthest;
    return furthest;
}
} // namespace topology
// vim: cindent ts=4 sw=4 et tw=0 wm=0
