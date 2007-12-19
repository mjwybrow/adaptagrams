#include <sstream>
#include <string>
#include <libproject/project.h>
#include <libcola/cola.h>
#include <libcola/straightener.h>
#include "topology_graph.h"
#include "topology_constraints.h"
#include "topology_log.h"
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
double TriConstraint::slack(const double ux, const double vx, const double wx) const {
    const double lhs = wx;
    const double rhs = ux+p*(vx-ux)+g;
    return leftOf ? rhs - lhs
                  : lhs - rhs;
}
double TriConstraint::slackAtDesired() const {
    return 
        slack(u->getDesiredPosition(), 
              v->getDesiredPosition(), 
              w->getDesiredPosition());
}
double TriConstraint::slack () const {
    return slack(u->getPosition(), v->getPosition(), w->getPosition());
}

ostream& operator<< (ostream& os, const TriConstraint& c) {
    double ux = c.u->relativeDesiredPos()
         , vx = c.v->relativeDesiredPos()
         , wx = c.w->relativeDesiredPos();
    os << "TriConstraint@" << &c 
       << ": u=" << ux 
       <<  " v=" << vx
       <<  " w=" << wx
       <<  " p=" << c.p
       <<  " g=" << c.g
       <<  " left=" << c.leftOf; 
    return os;
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
    FILE_LOG(logDEBUG)<<"BendConstraint::satisfy()";
    assert(fabs(c->slack())<1e-7);
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
    if(start->bendConstraint!=NULL) {
        delete start->bendConstraint;
        start->bendConstraint = new BendConstraint(start);
    }
    if(end->bendConstraint!=NULL) {
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
string BendConstraint::
toString() {
    stringstream s;
    s << "BendConstraint: bendPoint=(" << bendPoint->pos[0] << "," << bendPoint->pos[1] << ")";
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
            double min1=min(target1->start->pos[!dim],target1->end->pos[!dim]);
            double max1=max(target1->start->pos[!dim],target1->end->pos[!dim]);
            if(c->pos>=min1 && c->pos<=max1) {
                target1->straightConstraints.push_back(
                        new StraightConstraint(target1,c->node,c->pos));
            } else {
                assert(c->pos>min(target2->start->pos[!dim],target2->end->pos[!dim]));
                assert(c->pos<max(target2->start->pos[!dim],target2->end->pos[!dim]));
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
    FILE_LOG(logDEBUG)<<"StraightConstraint::satisfy()";
    assert(fabs(c->slack())<1e-7);
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
    if(start->bendConstraint) {
        delete start->bendConstraint;
        start->bendConstraint = new BendConstraint(start);
    }
    if(end->bendConstraint) {
        delete end->bendConstraint;
        end->bendConstraint = new BendConstraint(end);
    }
             
    e->nSegments++;
    delete segment;
}
string StraightConstraint::
toString() {
    stringstream s;
    s << "StraightConstraint: pos=" << pos;
    return s.str();
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
        // bendConstraints may be absent if the segment is horizontal
        if(s->end->bendConstraint!=NULL) {
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

struct PrintEdgePoint {
    void operator() (EdgePoint* p) {
        printf("addToPath(ps,vs[%d],(topology::EdgePoint::RectIntersect)%d);\n",p->node->id,p->rectIntersect);
    }
};
void TopologyConstraints::
printInstance() const {
    for(Nodes::const_iterator i=nodes.begin();i!=nodes.end();++i) {
        const Node* v=*i;
        const vpsc::Rectangle* r=v->rect;
        printf("addNode(vs,%f,%f,%f,%f);\n",
                r->getMinX(),r->getMinY(),r->width(),r->height());
    }
    for(Edges::const_iterator e=edges.begin();e!=edges.end();++e) {
        printf("ps.clear();\n");
        (*e)->forEachEdgePoint(PrintEdgePoint());
        printf("es.push_back(new Edge(%f,ps));\n",(*e)->idealLength);
    }
}
} // namespace topology
// vim: cindent ts=4 sw=4 et tw=80 wm=5 
