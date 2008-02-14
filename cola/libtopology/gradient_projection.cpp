/**
 * Methods associated with computing a descent vector for the topology stress
 * function.
 *
 * \file gradient_projection.cpp
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
void TopologyConstraints::
gradientProjection(valarray<double>& g, cola::SparseMap& h) {
    DesiredPositions d;
    return gradientProjection(g,h,d);
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

void TopologyConstraints::
gradientProjection(valarray<double>& g, cola::SparseMap& h, const
        DesiredPositions& d) 
{
    FILE_LOG(logDEBUG)<<"TopologyConstraints::gradientProjection... dim="<<dim;
    assert(g.size()==n);
    assert(h.n==n);
    //printInstance(g);
    computeForces(g,h);
    cola::SparseMatrix H(h);
    double stepSize = computeStepSize(H,g,g);
    FILE_LOG(logDEBUG1)<<"stepSize="<<stepSize;
    for(Nodes::iterator i=nodes.begin();i!=nodes.end();++i) {
        Node* v=*i;
        v->setDesiredPos(v->initialPos()-g[v->id]*stepSize);
    }
    for(DesiredPositions::const_iterator i=d.begin();i!=d.end();++i) {
        Node* v=nodes[i->first];
        v->setDesiredPos(i->second,1e10);
    }
}
bool TopologyConstraints::solve() {
    FILE_LOG(logDEBUG)<<"TopologyConstraints::solve... dim="<<dim;
    assert(assertConvexBends(edges));
    assert(assertNoSegmentRectIntersection(nodes,edges));
    assert(assertFeasible());
    vector<TopologyConstraint*> ts;
    constraints(ts);
    vpsc::Variables vs(nodes.size());
    transform(nodes.begin(),nodes.end(),vs.begin(),mem_fun(&Node::getVar));
    vpsc::IncSolver s(vs,cs);
    s.solve();
    double minTAlpha=DBL_MAX;
    TopologyConstraint* minT=NULL;
    // find minimum feasible alpha over all topology constraints
    for(vector<TopologyConstraint*>::iterator i=ts.begin();
            i!=ts.end();++i) {
        TopologyConstraint* t=*i;
        double tAlpha=t->c->maxSafeAlpha();
        double slackAtFinal=t->c->slackAtFinal();
        FILE_LOG(logDEBUG1)<<"Checking topology constraint! alpha="<<tAlpha
            <<"\n  slack at desired="<<slackAtFinal;
        FILE_LOG(logDEBUG1)<<t->toString();
        if(slackAtFinal<0 && tAlpha<minTAlpha) {
            minTAlpha=tAlpha;
            minT=t;
        }
    }
    bool interrupted=false;
    if(minTAlpha<1) {
        interrupted=true;
        FILE_LOG(logDEBUG1)<<"violated topology constraint! alpha="<<minTAlpha;
    }
    for(Nodes::iterator i=nodes.begin();i!=nodes.end();++i) {
        Node* v=*i;
        v->moveRect(interrupted,minTAlpha);
    }
    assert(noOverlaps());
    assert(assertConvexBends(edges));
    // rectangle and edge point positions updated to variables.
    FILE_LOG(logDEBUG)<<" moves done.";
    if(interrupted) {
        // now we satisfy the violated topology constraint, i.e. a bend point
        // that has become straight is removed or a segment that needs to bend
        // is split
        minT->satisfy();
    }
    assert(assertFeasible());
    assert(assertConvexBends(edges));
    assert(assertNoSegmentRectIntersection(nodes,edges));
    FILE_LOG(logDEBUG)<<"TopologyConstraints::steepestDescent... done";
    return interrupted;
}
} // namespace topology
// vim: cindent ts=4 sw=4 et tw=0 wm=0
