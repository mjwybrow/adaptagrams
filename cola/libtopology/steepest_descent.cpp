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
        //FILE_LOG(logDEBUG1)<<"e("<<e->firstSegment->start->node->id<<","<<e->lastSegment->end->node->id<<")="<<s<<std::endl;
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
bool TopologyConstraints::
steepestDescent(valarray<double>& g, cola::SparseMap& h) {
    DesiredPositions d;
    return steepestDescent(g,h,d);
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

bool assertConvexBends(const Edges& edges) {
    for(Edges::const_iterator e=edges.begin();e!=edges.end();++e) {
        assert((*e)->assertConvexBends());
    }
    return true;
}
bool TopologyConstraints::
steepestDescent(valarray<double>& g, cola::SparseMap& h, const
        DesiredPositions& d) 
{
    FILE_LOG(logDEBUG)<<"TopologyConstraints::steepestDescent... dim="<<dim;
    assert(g.size()==n);
    assert(h.n==n);
    assert(assertConvexBends(edges));
    //printInstance(g);
    bool interrupted=false;
    computeForces(g,h);
    cola::SparseMatrix H(h);
    double stepSize = computeStepSize(H,g,g);
    FILE_LOG(logDEBUG1)<<"stepSize="<<stepSize;
    for(unsigned i=0;i<n;++i) {
        double x=nodes[i]->rect->getCentreD(dim);
        vpsc::Variable* v=vs[i];
        v->desiredPosition=x-g[i]*stepSize;
    }
    for(DesiredPositions::const_iterator i=d.begin();i!=d.end();++i) {
        vpsc::Variable* v = vs[i->first];
        v->desiredPosition=i->second;
        v->weight=1e10;
        FILE_LOG(logDEBUG1)<<"override desi="<<v->desiredPosition;
    }
    vector<TopologyConstraint*> ts;
    constraints(ts);
    vpsc::IncSolver s(vs,cs);
    s.solve();
    for(unsigned i=0;i<n;++i) {
        vpsc::Variable* v=vs[i];
        Node* node=nodes[i];
        node->varPos.initial = node->rect->getCentreD(dim);
        node->varPos.desired = v->finalPosition;
    }
    double minTAlpha=DBL_MAX;
    TopologyConstraint* minT=NULL;
    // find minimum feasible alpha over all topology constraints
    for(vector<TopologyConstraint*>::iterator i=ts.begin();
            i!=ts.end();++i) {
        TopologyConstraint* t=*i;
        double tAlpha=t->c->maxSafeAlpha();
        double slackAtDesired=t->c->slackAtDesired();
        FILE_LOG(logDEBUG1)<<"Checking topology constraint! alpha="<<tAlpha
            <<"\n  slack at desired="<<slackAtDesired;
        FILE_LOG(logDEBUG1)<<t->toString();
        if(slackAtDesired<0 && tAlpha<minTAlpha) {
            minTAlpha=tAlpha;
            minT=t;
        }
    }
    assert(assertConvexBends(edges));
    if(minTAlpha<1) {
        interrupted=true;
        FILE_LOG(logDEBUG1)<<"violated topology constraint! alpha="<<minTAlpha;
    }
    for(Nodes::iterator i=nodes.begin();i!=nodes.end();++i) {
        Node* v=*i;
        double p=interrupted ? v->varPos.posOnLine(minTAlpha)
                             : v->varPos.desired;
        v->rect->moveCentreD(dim,p);
    }
    assert(noOverlaps());
    assert(assertConvexBends(edges));
    for(Edges::iterator e=edges.begin();e!=edges.end();++e) {
        (*e)->forEachEdgePoint(mem_fun(&EdgePoint::setPos),true);
    }
    // rectangle and edge point positions updated to variables.
    FILE_LOG(logDEBUG)<<" moves done.";
    if(interrupted) {
        // now we satisfy the violated topology constraint, i.e. a bend point
        // that has become straight is removed or a segment that needs to bend
        // is split
        minT->satisfy();
    }
    assert(assertConvexBends(edges));
    FILE_LOG(logDEBUG)<<"TopologyConstraints::steepestDescent... done";
    return interrupted;
}
} // namespace topology
// vim: cindent ts=4 sw=4 et tw=0 wm=0
