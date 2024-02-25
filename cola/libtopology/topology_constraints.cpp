/*
 * vim: ts=4 sw=4 et tw=0 wm=0
 *
 * libtopology - Classes used in generating and managing topology constraints.
 *
 * Copyright (C) 2007-2008  Monash University
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * See the file LICENSE.LGPL distributed with the library.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
*/

#include <sstream>
#include <string>
#include "libvpsc/variable.h"
#include "libvpsc/assertions.h"
#include "libcola/cola.h"
#include "libcola/straightener.h"
#include "libtopology/topology_graph.h"
#include "libtopology/topology_constraints.h"
#include "libtopology/topology_log.h"
using namespace std;
namespace topology {
/* 
 * @return the maximum move we can make along the line from initial to
 * final positions without violating this constraint
 */
double TriConstraint::maxSafeAlpha() const {
    const double u1=u->initialPos(scanDim),
                 u2=u->finalPos(),
                 v1=v->initialPos(scanDim),
                 v2=v->finalPos(),
                 w1=w->initialPos(scanDim),
                 w2=w->finalPos(),
                 fSlack=slackAtFinal();
    if(fSlack>=0) {
        FILE_LOG(logDEBUG1)<<"TriConstraint::maxSafeAlpha(): slackAtFinal="<<slackAtFinal();
        return 1;
    }
    double numerator=w1 - g - u1 + p*(u1-v1);
    // There are a number of situations where the following can
    // be 0:
    //   - no movement
    //   - rotation
    double denominator=u2-u1 + p*(u1-u2 + v2-v1) + w1-w2;
    FILE_LOG(logDEBUG1)<<"TriConstraint::maxSafeAlpha(): num="<<numerator<<" den="<<denominator;
    FILE_LOG(logDEBUG1)<<"  u1="<<u1<<" u2="<<u2<<" v1="<<v1<<" v2="<<v2<<" w1="<<w1<<" w2="<<w2;
    if(denominator==0) {
        return 1;
    }
    double msa = numerator/denominator;
    if(msa<0) {
#ifndef NDEBUG
        const double iSlack = slackAtInitial();
#endif
        COLA_ASSERT(iSlack>=fSlack);
        FILE_LOG(logDEBUG1)<<"  tiny negative msa rounded to 0!";
        // we know that fSlack is negative, we do not actually move by
        // negative amounts, but returning the following ensures that
        // the most violated constraint will be processed first
        msa = fSlack;
    }
    return msa;
}
double TriConstraint::slack(const double ux, const double vx, const double wx) const {
    const double lhs = wx;
    const double rhs = ux+p*(vx-ux)+g;
    FILE_LOG(logDEBUG1)<<"  TriConstraint::slack("<<ux<<","<<vx<<","<<wx<<"):leftOf="<<leftOf<<",lhs="<<lhs<<",rhs="<<rhs;
    return leftOf ? rhs - lhs
                  : lhs - rhs;
}
double TriConstraint::slackAtFinal() const {
    return 
        slack(u->finalPos(), 
              v->finalPos(), 
              w->finalPos());
}
double TriConstraint::slackAtInitial () const
{
    return slack(u->initialPos(scanDim), v->initialPos(scanDim),
            w->initialPos(scanDim));
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
/*
 * The bend has become straight, remove bend
 */
void BendConstraint::satisfy()
{
    COLA_ASSERT((scanDim==vpsc::XDIM)||(scanDim==vpsc::YDIM));
    FILE_LOG(logDEBUG) << "BendConstraint::satisfy()... edge id=" <<
            getEdgeID() << " node id=" << bendPoint->node->id;
    // XXX: Note that the call to prune() deletes the bendPoint and the 
    //      BendConstraint (this instance itself), so it is not safe to
    //      access any member variables after that.  Hence the local copy 
    //      of scanDim.  The code really needs to be reworked to be 
    //      written much more cleanly, only this kind of restructuring
    //      has the potential to introduce many bugs, so I will leave it
    //      for now.  -- mjwybrow
    vpsc::Dim dim = scanDim;
    Node* node=bendPoint->node;
    double pos=bendPoint->pos(vpsc::conjugate(scanDim));
    Segment* s=bendPoint->prune(scanDim);
    // create a new StraightConstraint to replace the BendConstraint
    s->createStraightConstraint(dim, node, pos);
    FILE_LOG(logDEBUG)<<"BendConstraint::satisfy()...done.";
}
string BendConstraint::toString() const {
    stringstream s;
    s << "BendConstraint: bendPoint=(" << bendPoint->posX() << "," << 
            bendPoint->posY() << ")";
    return s.str();
}
unsigned BendConstraint::getEdgeID() const {
    COLA_ASSERT(bendPoint->inSegment!=nullptr);
    COLA_ASSERT(bendPoint->outSegment!=nullptr);
    return bendPoint->inSegment->edge->id;
}
/*
 * functor that transfers each StraightConstraint associated with a segment
 * to be replaced, to either of the two new segments.
 * Choice is made depending on which of the segments the scan pos of the
 * constraint intersects with.
 * Note that if the scan pos is at the join of the two new segments then
 * the choice is made based the side of the node associated with the constraint.
 */
struct transferStraightConstraintChoose {
    /*
     * @param target1 the first of two possible target segments for the transfer
     * @param target2 the second of two possible target segments for the
     * transfer
     * @param ignore the constraint which, in being satisfied, caused this
     * transfer and which should therefore not be transfered.
     */
    transferStraightConstraintChoose(Segment* target1, Segment* target2,
            StraightConstraint* ignore)
        : ignore(ignore) 
    {
        vpsc::Dim dim = ignore->scanDim;
        double min1=min(target1->start->pos(vpsc::conjugate(dim)),target1->end->pos(vpsc::conjugate(dim)));
        double max1=max(target1->start->pos(vpsc::conjugate(dim)),target1->end->pos(vpsc::conjugate(dim)));
        double min2=min(target2->start->pos(vpsc::conjugate(dim)),target2->end->pos(vpsc::conjugate(dim)));
        double max2=max(target2->start->pos(vpsc::conjugate(dim)),target2->end->pos(vpsc::conjugate(dim)));
        if(min1<max2) {
            COLA_ASSERT(max1==min2);
            lSeg = target1;
            rSeg = target2;
            lMin = min1;
            mid = max1;
            rMax = max2;
        } else {
            COLA_ASSERT(max2==min1);
            lSeg = target2;
            rSeg = target1;
            lMin = min2;
            mid = max2;
            rMax = max1;
        }
    }
    /*
     * @param c constraint to transfer to target1 or target2
     */
    void operator() (StraightConstraint* c)
    {
        vpsc::Dim dim = ignore->scanDim;
        if(c!=ignore) {
            Segment* dest=rSeg;
            if(c->pos<mid) {
                dest = lSeg;
            } else if(c->pos==mid) {
                if ( (dim==vpsc::XDIM &&
                        (c->ri==EdgePoint::TL ||c->ri==EdgePoint::TR)) 
                     || 
                     (dim==vpsc::YDIM &&
                        (c->ri==EdgePoint::TR ||c->ri==EdgePoint::BR))) 
                {
                    dest=lSeg;
                }
            }
            dest->transferStraightConstraint(c);
        }
    }
    Segment* lSeg, * rSeg;
    double lMin, mid, rMax;
    StraightConstraint* ignore;
};
bool sameCorner(const EdgePoint* a, const EdgePoint* b) {
    COLA_UNUSED(a);
    COLA_UNUSED(b);
    COLA_ASSERT( !(a->node->id==b->node->id
                &&a->rectIntersect==b->rectIntersect));
    return false;
}
bool zigzag(const EdgePoint* a, const Segment* s) {
    COLA_UNUSED(a);
    if(s!=nullptr) {
        COLA_ASSERT(!sameCorner(a,s->end));
    }
    return false;
}
bool zagzig(const EdgePoint* a, const Segment* s) {
    COLA_UNUSED(a);
    if(s!=nullptr) {
        COLA_ASSERT(!sameCorner(a,s->start));
    }
    return false;
}
/*
 * Segment needs to bend
 */
void StraightConstraint::satisfy() {
    FILE_LOG(logDEBUG)<<"StraightConstraint::satisfy():";
    Edge* e = segment->edge;
    EdgePoint* start = segment->start,
             * end = segment->end,
             * bend = new EdgePoint(node,ri);
    FILE_LOG(logDEBUG1)<<"  u=("<<start->node->id<<":"<<start->rectIntersect<<"), v=("<<node->id<<":"<<ri<<"), w=("<<end->node->id<<":"<<end->rectIntersect<<")";
    COLA_ASSERT(!zigzag(bend,end->outSegment));
    COLA_ASSERT(!zagzig(bend,start->inSegment));
    // shouldn't have straight constraints between end segments and the
    // nodes to which they are connected.
    COLA_ASSERT(!segment->connectedToNode(node));
    
    Segment* s1 = new Segment(e,start,bend);
    Segment* s2 = new Segment(e,bend,end);
    if(e->firstSegment==segment) {
        e->firstSegment=s1;
    }
    if(e->lastSegment==segment) {
        e->lastSegment=s2;
    }
    // create BendConstraint to replace this StraightConstraint
    bend->createBendConstraint(scanDim);

    // transfer other StraightConstraint constraints 
    // from s to s1 or s2 depending on which side of p they are on.
    transferStraightConstraintChoose transfer(s1,s2,this);
    segment->forEachStraightConstraint(transfer);
    // BendConstraint constraints associated with segment->end and 
    // segment->start need to be updated
    start->createBendConstraint(scanDim);
    end->createBendConstraint(scanDim);
             
    e->nSegments++;
    delete segment;
}
/*
 * asserts that this TriConstraint is feasible at the initial positions of
 * variables.  Note that we don't apply this check for massive p because this
 * most likely indicates a segment that is near parallel to the scanline and
 * hence will never be violated anyway.
 */
bool TriConstraint::assertFeasible() const {
    COLA_ASSERT(fabs(p)>1e7||slackAtInitial()>-1e-3);
    return true;
}
bool TopologyConstraint::assertFeasible() const {
    COLA_ASSERT(c->assertFeasible());
    return true;
}
string StraightConstraint::
toString() const {
    stringstream s;
    s << "StraightConstraint: node id="<<node->id<<", segment=("<<segment->start->node->id<<":"<<segment->start->rectIntersect<<","<<segment->end->node->id<<":"<<segment->end->rectIntersect<<") pos=" << pos;
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
    for_each(edges.begin(),edges.end(),bind(
               &Edge::getTopologyConstraints,std::placeholders::_1,&ts));
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
    for_each(ts.begin(),ts.end(),mem_fn(&TopologyConstraint::assertFeasible));
    return true;
}
bool TopologyConstraints::solve() {
    FILE_LOG(logDEBUG)<<"TopologyConstraints::solve... dim="<<dim;
    COLA_ASSERT(assertConvexBends(edges));
    COLA_ASSERT(assertNoSegmentRectIntersection(nodes,edges));
    COLA_ASSERT(assertFeasible());
    vector<TopologyConstraint*> ts;
    constraints(ts);
    vpsc::IncSolver s(vs,cs);
    s.solve();
    double minTAlpha=1;
    TopologyConstraint* minT=nullptr;
    //printEdges(edges);
    // find minimum feasible alpha over all topology constraints
    for(vector<TopologyConstraint*>::iterator i=ts.begin();
            i!=ts.end();++i) {
        TopologyConstraint* t=*i;
        FILE_LOG(logDEBUG1)<<"Checking topology constraint:"<<t->toString();
        double tAlpha=t->c->maxSafeAlpha();
        FILE_LOG(logDEBUG1)<<"                              alpha="<<tAlpha;
        if(tAlpha<minTAlpha) {
            minTAlpha=tAlpha;
            minT=t;
        }
    }
#ifndef NDEBUG
    if(minT) {
        FILE_LOG(logDEBUG)<<"                 minT="<<minT->toString();
        FILE_LOG(logDEBUG)<<"                 minTAlpha="<<minTAlpha;
    } else {
        FILE_LOG(logDEBUG)<<" No violated constraints!";
    }
#endif
    if(minTAlpha>0) {
        for(Nodes::iterator i=nodes.begin();i!=nodes.end();++i) {
            Node* v=*i;
            v->rect->moveCentreD(dim,v->posOnLine(dim, minTAlpha));
        }
    }
    COLA_ASSERT(noOverlaps());
    COLA_ASSERT(assertConvexBends(edges));
    // rectangle and edge point positions updated to variables.
    FILE_LOG(logDEBUG)<<" moves done.";
    if(minTAlpha<1 && minT) {
        // now we satisfy the violated topology constraint, i.e. a bend point
        // that has become straight is removed or a segment that needs to bend
        // is split
        minT->satisfy();
    }
    //printEdges(edges);
    COLA_ASSERT(assertFeasible());
    COLA_ASSERT(assertConvexBends(edges));
    COLA_ASSERT(assertNoSegmentRectIntersection(nodes,edges));
    FILE_LOG(logDEBUG)<<"TopologyConstraints::solve... done";
    return minT!=nullptr;
}
/*
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
/*
 * compute the stress:
 * \f[
 *   \sigma = \sum_{e \in E} \left( d_e - \sum_{s \in S(e)} |s| \right)^2
 * \f]
 */
double computeStress(const Edges& es) {
    return sum_over(es.begin(),es.end(),0.0,ComputeStress());
}
} // namespace topology
