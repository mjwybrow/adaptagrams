#include <libproject/project.h>
#include <libcola/cola.h>
#include <libcola/straightener.h>
#include "topology_graph.h"
#include "topology_constraints.h"
using namespace std;
namespace topology {
/** 
 * @return the maximum move we can make along the line from initial to
 * desired positions without violating this constraint
 */
double TriConstraint::maxSafeAlpha() const {
    double u1=u->relativeInitialPos();
    double u2=u->relativeDesiredPos();
    double v1=v->relativeInitialPos();
    double v2=v->relativeDesiredPos();
    double w1=w->relativeInitialPos();
    double w2=w->relativeDesiredPos();
    double numerator=w1 - g - u1 + p*(u1-v1);
    // There are a number of situations where the following can
    // be 0!
    double denominator=u2-u1 + p*(u1-u2 + v2-v1) + w1-w2;
    //assert(denominator!=0);
    if(denominator==0) {
        return 1;
    }
    return numerator/denominator;
}
double TriConstraint::slack () const {
    double rhs = u->x+p*(v->x-u->x)+g;
    double lhs = w->x;
    return leftOf ? rhs - lhs : lhs - rhs;
}
void TriConstraint::print() const {
    printf("TopologyConstraint@%p\n",this);
    printf("  u=%f\n  v=%f\n  w=%f\n  p=%f\n  g=%f\n  left=%d\n",
            u->x,v->x,w->x,p,g,leftOf);
}
struct transferStraightConstraint {
    transferStraightConstraint(Segment* target)
        : target(target) {}
    void operator() (StraightConstraint* s) {
        target->straightConstraints.push_back(
                new StraightConstraint(target,s->node,s->pos));
    }
    Segment* target;
};
/**
 * The bend has become straight, remove bend
 */
void BendConstraint::satisfy() {
    Segment* s1 = bendPoint->inSegment,
           * s2 = bendPoint->outSegment;
    Edge* e = s1->edge;
    EdgePoint* start = s1->start,
             * end = s2->end;
    Segment* s = new Segment(e,start,end);
    if(e->firstSegment==s1) {
        e->firstSegment=s;
    }
    if(e->lastSegment==s2) {
        e->lastSegment=s;
    }
    // transfer each StraightConstraint from s1 and s2 to newSegment.
    transferStraightConstraint transfer(s);
    for_each(s1->straightConstraints.begin(),
             s1->straightConstraints.end(),
             transfer);
    for_each(s2->straightConstraints.begin(),
             s2->straightConstraints.end(),
             transfer);
    // update each BendConstraint involving bendPoint
    if(!start->isReal()) {
        delete start->bendConstraint;
        start->bendConstraint = new BendConstraint(start);
    }
    if(!end->isReal()) {
        delete end->bendConstraint;
        end->bendConstraint = new BendConstraint(end);
    }
    // create a new StraightConstraint to replace the BendConstraint
    s->straightConstraints.push_back(new StraightConstraint(
                s, bendPoint->node, bendPoint->pos[!dim]));
             
    e->nSegments--;
    delete bendPoint;
    delete s1;
    delete s2;
}
struct transferStraightConstraintChoose {
    transferStraightConstraintChoose(Segment* target1, Segment* target2,
            double pos, StraightConstraint* ignore)
        : target1(target1), target2(target2)
        , pos(pos), ignore(ignore) {}
    void operator() (StraightConstraint* c) {
        if(c!=ignore) {
            if(target1->start->pos[!dim] > target2->end->pos[!dim]
               && pos > target1->end->pos[!dim]) {
                target1->straightConstraints.push_back(
                        new StraightConstraint(target1,c->node,c->pos));
            } else {
                target2->straightConstraints.push_back(
                        new StraightConstraint(target2,c->node,c->pos));
            }
        }
    }
    Segment* target1, * target2;
    double pos;
    StraightConstraint* ignore;
};
/**
 * Segment needs to bend
 */
void StraightConstraint::satisfy() {
    Edge* e = segment->edge;
    EdgePoint* start = segment->start,
             * end = segment->end,
             * bend = new EdgePoint(node,ri);
    Segment* s1 = new Segment(e,start,bend);
    Segment* s2 = new Segment(e,bend,end);
    if(e->firstSegment==segment) {
        e->firstSegment=s1;
    }
    if(e->lastSegment==segment) {
        e->lastSegment=s2;
    }
    // create BendConstraint to replace this StraightConstraint
    bend->bendConstraint = new BendConstraint(bend);

    // transfer other StraightConstraint constraints 
    // from s to s1 or s2 depending on which side of p they are on.
    transferStraightConstraintChoose transfer(s1,s2,pos,this);
    for_each(segment->straightConstraints.begin(),
             segment->straightConstraints.end(),
             transfer);
    // BendConstraint constraints associated with segment->end and 
    // segment->start need to be updated
    if(!start->isReal()) {
        delete start->bendConstraint;
        start->bendConstraint = new BendConstraint(start);
    }
    if(!end->isReal()) {
        delete end->bendConstraint;
        end->bendConstraint = new BendConstraint(end);
    }
             
    e->nSegments++;
    delete segment;
}
struct buildRoute {
    buildRoute(straightener::Route* r, unsigned& n) : r(r), n(n) {}
    void operator() (const Segment* s) {
        EdgePoint* u=s->end;
        r->xs[n]=u->pos[0];
        r->ys[n++]=u->pos[1];
    }
    straightener::Route* r;
    unsigned& n;
};
struct getTopologyConstraints {
    getTopologyConstraints(vector<TopologyConstraint*>& ts) : ts(ts) {}
    void operator() (const Segment* s) {
        for(vector<StraightConstraint*>::const_iterator 
            t=s->straightConstraints.begin();
            t!=s->straightConstraints.end();++t) {
            assert(*t!=NULL);
            ts.push_back(*t);
        }
        if(!s->end->isReal()) {
            assert(s->end->bendConstraint!=NULL);
            ts.push_back(s->end->bendConstraint);
        }
    }
    vector<TopologyConstraint*>& ts;
};
void TopologyConstraints::
constraints(std::vector<TopologyConstraint*> & ts) const {
    for(Edges::const_iterator e=edges.begin();e!=edges.end();++e) {
        (*e)->forEachSegmentConst(getTopologyConstraints(ts));
    }
}
} // namespace topology
// vim: cindent ts=4 sw=4 et tw=0 wm=0
