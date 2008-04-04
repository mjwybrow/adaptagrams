#include <sstream>
#include <string>
#include <libvpsc/variable.h>
#include <libcola/cola.h>
#include <libcola/straightener.h>
#include "topology_graph.h"
#include "topology_constraints.h"
#include "topology_log.h"
using namespace std;
namespace topology {
/** 
 * @return the maximum move we can make along the line from initial to
 * final positions without violating this constraint
 */
double TriConstraint::maxSafeAlpha() const {
    double u1=u->initialPos();
    double u2=u->finalPos();
    double v1=v->initialPos();
    double v2=v->finalPos();
    double w1=w->initialPos();
    double w2=w->finalPos();
    double numerator=w1 - g - u1 + p*(u1-v1);
    // There are a number of situations where the following can
    // be 0!
    double denominator=u2-u1 + p*(u1-u2 + v2-v1) + w1-w2;
    if(denominator==0) {
        return 1;
    }
    return numerator/denominator;
}
double TriConstraint::slack(const double ux, const double vx, const double wx) const {
    const double lhs = wx;
    const double rhs = ux+p*(vx-ux)+g;
    return leftOf ? rhs - lhs
                  : lhs - rhs;
}
double TriConstraint::slackAtFinal() const {
    return 
        slack(u->finalPos(), 
              v->finalPos(), 
              w->finalPos());
}
double TriConstraint::slackAtInitial () const {
    return slack(u->initialPos(), v->initialPos(), w->initialPos());
}

ostream& operator<< (ostream& os, const TriConstraint& c) {
    double ux = c.u->finalPos()
         , vx = c.v->finalPos()
         , wx = c.w->finalPos();
    os << "TriConstraint@" << &c 
       << ": u=" << ux 
       <<  " v=" << vx
       <<  " w=" << wx
       <<  " p=" << c.p
       <<  " g=" << c.g
       <<  " left=" << c.leftOf; 
    return os;
}
/**
 * The bend has become straight, remove bend
 */
void BendConstraint::satisfy() {
    FILE_LOG(logDEBUG)<<"BendConstraint::satisfy()... edge id="<<getEdgeID()<<" node id="<<bendPoint->node->id;
    Segment* s1 = bendPoint->inSegment,
           * s2 = bendPoint->outSegment;
    Edge* e = s1->edge;
    EdgePoint* start = s1->start,
             * end = s2->end;
    Segment* s = new Segment(e,start,end);
    if(e->lastSegment==s1 && e->firstSegment==s2) {
        FILE_LOG(logDEBUG)<<"  handling cyclical boundary.";
        e->firstSegment=s;
        e->lastSegment=start->inSegment;
    }
    if(e->firstSegment==s1) {
        e->firstSegment=s;
    }
    if(e->lastSegment==s2) {
        e->lastSegment=s;
    }
    // transfer each StraightConstraint from s1 and s2 to new Segment s.
    Segment::TransferStraightConstraint transfer = 
        bind1st(mem_fun(&Segment::transferStraightConstraint),s);
    s1->forEachStraightConstraint(transfer);
    s2->forEachStraightConstraint(transfer);

    // update the BendConstraints associated with the end EdgePoints of
    // the new segment
    start->createBendConstraint();
    end->createBendConstraint();

    // create a new StraightConstraint to replace the BendConstraint
    s->createStraightConstraint(bendPoint->node, bendPoint->pos(!dim));
             
    e->nSegments--;
    delete bendPoint;
    delete s1;
    delete s2;
    FILE_LOG(logDEBUG)<<"BendConstraint::satisfy()...done.";
}
string BendConstraint::
toString() const {
    stringstream s;
    s << "BendConstraint: bendPoint=(" << bendPoint->posX() << "," << bendPoint->posY() << ")";
    return s.str();
}
/**
 * functor that transfers the constraints from one segment to one of two new
 * segments depending which intersects with pos in the scan axis.
 */
struct transferStraightConstraintChoose {
    /**
     * @param target1 the first of two possible target segments for the transfer
     * @param target2 the second of two possible target segments for the transfer
     * @param pos scan position
     * @param ignore the constraint which, in being satisfied, caused this
     * transfer and which should therefore not be transfered.
     */
    transferStraightConstraintChoose(Segment* target1, Segment* target2,
            double pos, StraightConstraint* ignore)
        : target1(target1), target2(target2)
        , pos(pos), ignore(ignore) {}
    /**
     * @param c constraint to transfer to target1 or target2
     */
    void operator() (StraightConstraint* c) {
        if(c!=ignore) {
            double min1=min(target1->start->pos(!dim),target1->end->pos(!dim));
            double max1=max(target1->start->pos(!dim),target1->end->pos(!dim));
            if(c->pos>=min1 && c->pos<=max1) {
                target1->transferStraightConstraint(c);
            } else {
                assert(c->pos>=min(target2->start->pos(!dim),target2->end->pos(!dim)));
                assert(c->pos<=max(target2->start->pos(!dim),target2->end->pos(!dim)));
                target2->transferStraightConstraint(c);
            }
        }
    }
    Segment* target1, * target2;
    double pos;
    StraightConstraint* ignore;
};
bool sameCorner(const EdgePoint* a, const EdgePoint* b) {
    assert( !(a->node->id==b->node->id
                &&a->rectIntersect==b->rectIntersect));
    return false;
}
bool zigzag(const EdgePoint* a, const Segment* s) {
    if(s!=NULL) {
        assert(!sameCorner(a,s->end));
    }
    return false;
}
bool zagzig(const EdgePoint* a, const Segment* s) {
    if(s!=NULL) {
        assert(!sameCorner(a,s->start));
    }
    return false;
}
/**
 * Segment needs to bend
 */
void StraightConstraint::satisfy() {
    FILE_LOG(logDEBUG)<<"StraightConstraint::satisfy()";
    Edge* e = segment->edge;
    EdgePoint* start = segment->start,
             * end = segment->end,
             * bend = new EdgePoint(node,ri);
    assert(!zigzag(bend,end->outSegment));
    assert(!zagzig(bend,start->inSegment));
    // shouldn't have straight constraints between end segments and the
    // nodes to which they are connected.
    assert(!segment->connectedToNode(node));
    
    Segment* s1 = new Segment(e,start,bend);
    Segment* s2 = new Segment(e,bend,end);
    if(e->firstSegment==segment) {
        e->firstSegment=s1;
    }
    if(e->lastSegment==segment) {
        e->lastSegment=s2;
    }
    // create BendConstraint to replace this StraightConstraint
    bend->createBendConstraint();

    // transfer other StraightConstraint constraints 
    // from s to s1 or s2 depending on which side of p they are on.
    transferStraightConstraintChoose transfer(s1,s2,pos,this);
    segment->forEachStraightConstraint(transfer);
    // BendConstraint constraints associated with segment->end and 
    // segment->start need to be updated
    start->createBendConstraint();
    end->createBendConstraint();
             
    e->nSegments++;
    delete segment;
}
/**
 * asserts that this TriConstraint is feasible at the initial positions of
 * variables.  Note that we don't apply this check for massive p because this
 * most likely indicates a segment that is near parallel to the scanline and
 * hence will never be violated anyway.
 */
bool TriConstraint::assertFeasible() const {
    assert(fabs(p)>1e7||slackAtInitial()>-1e-3);
    return true;
}
bool TopologyConstraint::assertFeasible() const {
    assert(c->assertFeasible());
    return true;
}
string StraightConstraint::
toString() const {
    stringstream s;
    s << "StraightConstraint: pos=" << pos;
    return s.str();
}
struct buildRoute {
    buildRoute(straightener::Route* r, unsigned& n) : r(r), n(n) {}
    void operator() (const Segment* s) {
        EdgePoint* u=s->end;
        r->xs[n]=u->posX();
        r->ys[n++]=u->posY();
    }
    straightener::Route* r;
    unsigned& n;
};
void TopologyConstraints::
constraints(std::vector<TopologyConstraint*>& ts) const {
    for_each(edges.begin(),edges.end(),bind2nd(
                mem_fun(&Edge::getTopologyConstraints),&ts));
}

struct PrintEdgePoint {
    void operator() (EdgePoint* p) {
        printf("t.addToPath(%d,(topology::EdgePoint::RectIntersect)%d);\n",p->node->id,p->rectIntersect);
    }
};
void TopologyConstraints::
printInstance(valarray<double>& g) const {
    printf("double gradient[]={%f",g[0]);
    for(unsigned i=0;i<n;++i) { printf(",%f",g[i]); }
    printf("}\nt.setGradient(gradient)\n");
    for(Nodes::const_iterator i=nodes.begin();i!=nodes.end();++i) {
        const Node* v=*i;
        const vpsc::Rectangle* r=v->rect;
        printf("t.addNode(%f,%f,%f,%f);\n",
                r->getMinX(),r->getMinY(),r->width(),r->height());
    }
    for(Edges::const_iterator e=edges.begin();e!=edges.end();++e) {
        (*e)->forEachEdgePoint(PrintEdgePoint());
        printf("t.addEdge(%f);\n",(*e)->idealLength);
    }
}
bool TopologyConstraints::
assertFeasible() const {
    vector<TopologyConstraint*> ts;
    constraints(ts);
    for_each(ts.begin(),ts.end(),mem_fun(&TopologyConstraint::assertFeasible));
    return true;
}
bool TopologyConstraints::solve() {
    FILE_LOG(logDEBUG)<<"TopologyConstraints::solve... dim="<<dim;
    assert(assertConvexBends(edges));
    assert(assertNoSegmentRectIntersection(nodes,edges));
    assert(assertFeasible());
    vector<TopologyConstraint*> ts;
    constraints(ts);
    vpsc::IncSolver s(vs,cs);
    s.solve();
    double minTAlpha=DBL_MAX;
    TopologyConstraint* minT=NULL;
    //printEdges(edges);
    // find minimum feasible alpha over all topology constraints
    for(vector<TopologyConstraint*>::iterator i=ts.begin();
            i!=ts.end();++i) {
        TopologyConstraint* t=*i;
        double tAlpha=t->c->maxSafeAlpha();
        double slackAtFinal=t->c->slackAtFinal();
        FILE_LOG(logDEBUG1)<<"Checking topology constraint! alpha="<<tAlpha
            <<"  slack at desired="<<slackAtFinal;
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
    //printEdges(edges);
    assert(assertFeasible());
    assert(assertConvexBends(edges));
    assert(assertNoSegmentRectIntersection(nodes,edges));
    FILE_LOG(logDEBUG)<<"TopologyConstraints::steepestDescent... done";
    return interrupted;
}
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
double computeStress(const Edges& es) {
    return sum_over(es.begin(),es.end(),0.0,ComputeStress());
}
} // namespace topology
// vim: cindent ts=4 sw=4 et tw=80 wm=5 
