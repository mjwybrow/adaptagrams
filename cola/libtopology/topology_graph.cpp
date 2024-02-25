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

// sstream needs ::strcpy_s under MinGW so include cstring.
#include <cstring>

#include <sstream>
#include <stdexcept>

#include "libvpsc/assertions.h"
#include "libvpsc/rectangle.h"
#include "libcola/cola.h"
#include "libcola/straightener.h"

#include "libtopology/topology_log.h"
#include "libtopology/topology_graph.h"
#include "libtopology/topology_constraints.h"

using namespace std;


namespace topology {

void setNodeVariables(Nodes& ns, std::vector<vpsc::Variable*>& vs) {
    COLA_ASSERT(ns.size()<=vs.size());
    std::vector<vpsc::Variable*>::iterator v=vs.begin();
    for(Nodes::iterator i=ns.begin();i!=ns.end();++i,++v) {
        (*i)->var=(*v);
    }
}

static const double POSITION_LIMIT = 1000000;

Node::Node(unsigned id, vpsc::Rectangle* r, vpsc::Variable* v)
    : id(id), rect(r), var(v)
{ 
    COLA_ASSERT(initialPos(vpsc::XDIM) >- POSITION_LIMIT);
    COLA_ASSERT(initialPos(vpsc::XDIM) < POSITION_LIMIT);

    COLA_ASSERT(initialPos(vpsc::YDIM) >- POSITION_LIMIT);
    COLA_ASSERT(initialPos(vpsc::YDIM) < POSITION_LIMIT);
}
void Node::setDesiredPos(double d, double weight) {
    var->desiredPosition=d;
    var->weight=weight;
}
double Node::initialPos(vpsc::Dim scanDim) const {
    return rect->getCentreD(scanDim);
}
double Node::finalPos() const {
    return var->finalPosition;
}
double Node::posOnLine(vpsc::Dim scanDim, double alpha) const {
    double i=initialPos(scanDim);
    double d=finalPos()-i;
    return i+alpha*d; 
}
void EdgePoint::deleteBendConstraint() {
    if(bendConstraint) {
        delete bendConstraint;
        bendConstraint=nullptr;
    }
}
EdgePoint::~EdgePoint() {
    deleteBendConstraint();
}
Segment* EdgePoint::prune(vpsc::Dim scanDim) {
    Edge* e = inSegment->edge;
    EdgePoint* start = inSegment->start,
             * end = outSegment->end;
    Segment* s = new Segment(e,start,end);
    if(e->lastSegment==inSegment && e->firstSegment==outSegment) {
        FILE_LOG(logDEBUG)<<"  handling cyclical boundary.";
        e->firstSegment=s;
        e->lastSegment=start->inSegment;
    }
    if(e->firstSegment==inSegment) {
        e->firstSegment=s;
    }
    if(e->lastSegment==outSegment) {
        e->lastSegment=s;
    }
    // transfer each StraightConstraint from inSegment and outSegment
    // to new Segment s.
    Segment::TransferStraightConstraint transfer = 
        std::bind(&Segment::transferStraightConstraint,s,std::placeholders::_1);
    inSegment->forEachStraightConstraint(transfer);
    outSegment->forEachStraightConstraint(transfer);

    // update the BendConstraints associated with the end EdgePoints of
    // the new segment
    start->createBendConstraint(scanDim);
    end->createBendConstraint(scanDim);

    e->nSegments--;
    delete inSegment;
    delete outSegment;
    delete this;
    return s;
}
bool EdgePoint::createBendConstraint(vpsc::Dim scanDim) {
    // edges shouldn't double back!
    COLA_ASSERT(assertConvexBend());
    // we replace any existing bend constraint
    if(bendConstraint) {
        delete bendConstraint;
        bendConstraint=nullptr;
    }
    // don't generate BendConstraints for Edge end points
    if(isEnd()) {
        return false;
    }
    // don't try to generate a BendConstraint if both incident segments
    // are parallel to the scan dimension
    if(inSegment->length(vpsc::conjugate(scanDim)) == 0 &&
       outSegment->length(vpsc::conjugate(scanDim)) == 0) {
        return false;
    }
    bendConstraint = new BendConstraint(this, scanDim);
    return true;
}
void EdgePoint::getBendConstraint(vector<TopologyConstraint*>* ts) {
    if(bendConstraint) {
        ts->push_back(bendConstraint);
    }
}
double EdgePoint::pos(vpsc::Dim dim) const {
    double p;
    vpsc::Rectangle* r=node->rect;
    switch(rectIntersect) {
        case TL:
            p=dim==vpsc::XDIM?
                r->getMinX():r->getMaxY();
            break;
        case TR:
            p=r->getMaxD(dim);
            break;
        case BL:
            p=r->getMinD(dim);
            break;
        case BR:
            p=dim==vpsc::XDIM?
                r->getMaxX():r->getMinY();
            break;
        default:
            p=r->getCentreD(dim);
    }
    return p;
}
double EdgePoint::offset(vpsc::Dim dim) const
{
    if(rectIntersect==CENTRE) {
        return 0;
    }
    double o = node->rect->length(dim)/2.0;
    if ((dim==vpsc::XDIM && (rectIntersect == TL || rectIntersect == BL)) ||
        (dim==vpsc::YDIM && (rectIntersect == BL || rectIntersect == BR)))
        {
            return -o;
        }
        return o;
    }
    /*
     * @return true if the EdgePoint is the end of an edge.
     * Note that cluster boundary edges are cycles, and therefore have no
     * ends.
     */
    bool EdgePoint::isEnd() const {
        if(outSegment==nullptr || inSegment==nullptr) {
            return true;
        }
        return false;
    }
inline void normalise(double& x, double& y) {
    double l=sqrt(x*x+y*y);
    x/=l;
    y/=l;
}
bool EdgePoint::assertConvexBend() const {
    const double eps=1e-7;
    if(inSegment && outSegment 
       && inSegment->length()>eps && outSegment->length()>eps) 
    {
        EdgePoint* u=inSegment->start;
        EdgePoint* w=outSegment->end;
        // cp>0: left turn, cp<0: right turn
        double cp = crossProduct(u->posX(),u->posY(),posX(),posY(),w->posX(),w->posY());
        double dx = w->posX() - u->posX(), dy = w->posY() - u->posY();
        try {
            if( rectIntersect==CENTRE ) {
                throw runtime_error("Bend point is CENTRE connected!");
            }
            if( u->node->id==node->id && u->rectIntersect==rectIntersect ) {
                throw runtime_error("Consecutive end points the same!");
            }
            if(w->node->id==node->id&&w->rectIntersect==rectIntersect) {
                throw runtime_error("Consecutive end points the same!");
            }
            if(u->node->id==w->node->id&&u->rectIntersect==w->rectIntersect) {
                throw runtime_error("Two points on same edge the same!");
            }
            // monotonicity:
            if(!( (u->posX()<=posX()+eps && posX()<=w->posX()+eps) ||
                  (u->posX()>=posX()-eps && posX()>=w->posX()-eps))) 
            {
                throw runtime_error("3 consecutive points not monotonically increasing in X!\n");
            }
            if(!( (u->posY()<=posY()+eps && posY()<=w->posY()+eps) ||
                  (u->posY()>=posY()-eps && posY()>=w->posY()-eps)) )
            {
                throw runtime_error("3 consecutive points not monotonically increasing in Y!\n");
            }
            // ensure tight turn
            if(fabs(dx)>eps && fabs(dy)>eps) {
                switch(rectIntersect) {
                    case TR:
                        if(dx>0) { // ux<wx
                            if(dy>0) { // uy<wy
                                throw runtime_error("turn not tight: C1");
                            } else if(cp>eps) {
                                throw runtime_error("turn not tight: C2");
                            }
                        } else { // ux>=wx
                            if(dy<0) {
                                throw runtime_error("turn not tight: C3");
                            } else if(cp<-eps) {
                                throw runtime_error("turn not tight: C4");
                            }
                        }
                        break;
                    case BR:
                        if(dx>0) { // ux<wx
                            if(dy<0) {
                                throw runtime_error("turn not tight: C5");
                            } else if(cp<-eps) {
                                throw runtime_error("turn not tight: C6");
                            }
                        } else { // ux>=wx
                            if(dy>0) {
                                throw runtime_error("turn not tight: C7");
                            } else if(cp>eps) {
                                throw runtime_error("turn not tight: C8");
                            }
                        }
                        break;
                    case BL:
                        if(dx>0) { //ux<wx
                            if(dy>0) {
                                throw runtime_error("turn not tight: C9");
                            } else if(cp<-eps) {
                                throw runtime_error("turn not tight: C10");
                            }
                        } else { //ux>=wx
                            if(dy<0) {
                                throw runtime_error("turn not tight: C11");
                            } else if(cp>eps) {
                                throw runtime_error("turn not tight: C12");
                            }
                        }
                        break;
                    case TL:
                        if(dx>0) { // ux<wx
                            if(dy<0) { // uy>wy
                                throw runtime_error("turn not tight: C13");
                            } else if(cp>eps) {
                                throw runtime_error("turn not tight: C14");
                            }
                        } else { // ux>=wx
                            if(dy>0) {
                                throw runtime_error("turn not tight: C15");
                            } else if(cp<-eps) {
                                throw runtime_error("turn not tight: C16");
                            }
                        }
                        break;
                    default:
                        // a bend point must be associated with one of the
                        // corners of a rectangle!
                        COLA_ASSERT(false);
                }
            }
        } catch(runtime_error & e) {
            printf("  convexity bend point test failed: %s, dx=%f, dy=%f, cp=%f:\n",e.what(),dx,dy,cp);
            printf("    (nid=%d,ri=%d):u={%f,%f}\n",
                    u->node->id,u->rectIntersect,u->posX(),u->posY());
            printf("    (nid=%d,ri=%d):v={%f,%f}\n",
                    node->id,rectIntersect,posX(),posY());
            printf("    (nid=%d,ri=%d):w={%f,%f}\n",
                    w->node->id,w->rectIntersect,w->posX(),w->posY());
            printf("    turn cross product=%e\n",cp);
            cout<<"Show[Graphics[{"<<endl;
            cout<<*u->node->rect<<","<<*node->rect<<","<<*w->node->rect<<","<<endl;
            cout<<inSegment->toString()<<","<<outSegment->toString()<<endl;
            cout<<"}]]"<<endl;
            COLA_ASSERT(false);
        }
    } 
    return true;
}
bool Segment::connectedToNode(const Node* v) const {
    if (start->rectIntersect == EdgePoint::CENTRE && start->node->id == v->id) {
        return true;
    }
    if (end->rectIntersect == EdgePoint::CENTRE && end->node->id == v->id) {
        return true;
    }
    return false;
}
void Segment::
getStraightConstraints(vector<TopologyConstraint*>* ts) const {
    size_t n = ts->size();
    ts->resize(n+straightConstraints.size());
    copy(straightConstraints.begin(),straightConstraints.end(),
            ts->begin()+n);
}
void Segment::deleteStraightConstraints() {
    forEachStraightConstraint(delete_object());
    straightConstraints.clear();
}
/*
 * clean up topologyConstraints
 */
Segment::~Segment() {
    deleteStraightConstraints();
}
double Segment::length(vpsc::Dim dim) const {
    return fabs(end->pos(dim)-start->pos(dim));
}
void Segment::assertNonZeroLength() const {
    if(length()==0) {
        printf("segment length=%f\n",length());
    }
    //COLA_ASSERT(length()>0);
}
double Segment::length() const {
    double dx = end->posX() - start->posX();
    double dy = end->posY() - start->posY();
    return sqrt(dx*dx + dy*dy);
}

struct copyEdgePointsToRoute {
    copyEdgePointsToRoute(straightener::Route* r) : x(r->xs), y(r->ys) {}
    void operator() (const EdgePoint* p) {
        *x++=p->posX();
        *y++=p->posY();
    }
    double *x, *y;
};
straightener::Route* Edge::getRoute() const {
    straightener::Route* r = new straightener::Route(nSegments+1);
    forEachEdgePoint(copyEdgePointsToRoute(r));
    return r;
}
struct accumulateLength {
    accumulateLength(double& a) : a(a) {}
    void operator()(const Segment* s) {
        a+=s->length();
    }
    double& a;
};
double Edge::pathLength() const {
    double totalLength = 0;
    forEachSegment(accumulateLength(totalLength));
    return totalLength;
}
bool Edge::assertConvexBends() const {
    forEachEdgePoint(mem_fn(&EdgePoint::assertConvexBend),true);
    return true;
}
struct PointToString {
    PointToString(stringstream& ss) : ss(ss) {}
    void operator()(const EdgePoint* p) {
        ss << *p->node->rect << "," <<endl;
    }
    stringstream& ss;
};
struct SegmentToString {
    SegmentToString(stringstream& ss) : ss(ss) {}
    void operator()(const Segment* s) {
        ss << s->toString() <<",";
    }
    stringstream& ss;
};
string Segment::toString() const {
    stringstream ss;
    ss << "Hue[0.77]," << "Line[{{"<<start->posX()<<","<<start->posY()<<"},{"
        << end->posX()<<","<<end->posY()<<"}}]";
    return ss.str();
}
string Edge::toString() const {
    stringstream ss;
    ss << "Show[Graphics[{";
    forEach(PointToString(ss),SegmentToString(ss));
    ss << "}]]" << endl;
    return ss.str();
}
struct buildPath {
    buildPath(ConstEdgePoints& vs) : vs(vs) {}
    void operator()(const EdgePoint* p) {
        vs.push_back(p);
    }
    ConstEdgePoints& vs;
};
/*
 * Get a list of all the EdgePoints along the Edge path.  Note that for
 * cycles the start/end point will be at the start and end of the list,
 * i.e. it will appear in the list twice.
 */
void Edge::getPath(ConstEdgePoints& vs) const {
    forEachEdgePoint(buildPath(vs));
}
bool assertConvexBends(const Edges& es) {
    for_each(es.begin(),es.end(),mem_fn(&Edge::assertConvexBends));
    return true;
}
#ifndef NDEBUG
struct NoIntersection {
    NoIntersection(const Nodes& vs) : vs(vs) {}
    void operator()(const Segment* s) {
        for(Nodes::const_iterator v=vs.begin();v!=vs.end();++v) {
            if(s->start->node->id==(*v)->id || s->end->node->id==(*v)->id) {
                continue;
            }
            if(s->start->node==s->end->node) {
                COLA_ASSERT((s->start->rectIntersect==EdgePoint::BL && 
                                s->end->rectIntersect==EdgePoint::BR) ||
                            (s->start->rectIntersect==EdgePoint::BR &&
                                s->end->rectIntersect==EdgePoint::BL) ||
                            (s->start->rectIntersect==EdgePoint::BL &&
                                s->end->rectIntersect==EdgePoint::TL) ||
                            (s->start->rectIntersect==EdgePoint::TL &&
                                s->end->rectIntersect==EdgePoint::BL) ||
                            (s->start->rectIntersect==EdgePoint::BR &&
                                s->end->rectIntersect==EdgePoint::TR) ||
                            (s->start->rectIntersect==EdgePoint::TR &&
                                s->end->rectIntersect==EdgePoint::BR) ||
                            (s->start->rectIntersect==EdgePoint::TL &&
                                s->end->rectIntersect==EdgePoint::TR) || 
                            (s->start->rectIntersect==EdgePoint::TR &&
                                s->end->rectIntersect==EdgePoint::TL));
                continue;
            }
            double sx=s->start->posX(), sy=s->start->posY(),
                   ex=s->end->posX(), ey=s->end->posY();
            double xBorder=vpsc::Rectangle::xBorder;
            double yBorder=vpsc::Rectangle::yBorder;
            vpsc::Rectangle::setXBorder(xBorder-1e-6);
            vpsc::Rectangle::setYBorder(yBorder-1e-6);
            if((*v)->rect->overlaps(sx,sy,ex,ey)) {
                printf("ERROR: Segment on edge id=%d overlaps Node id=%d\n",
                        s->edge->id,(*v)->id);
                COLA_ASSERT(false);
            }
            vpsc::Rectangle::setXBorder(xBorder);
            vpsc::Rectangle::setYBorder(yBorder);
        }
    }
    const Nodes& vs;
};
bool assertNoSegmentRectIntersection(
        const Nodes& vs, const Edges& es) {
    for(Edges::const_iterator e=es.begin();e!=es.end();++e) {
        (*e)->forEachSegment(NoIntersection(vs));
    }
    return true;
}
bool assertNoZeroLengthEdgeSegments(const Edges& es) {
    for(Edges::const_iterator e=es.begin();e!=es.end();++e) {
        (*e)->forEachSegment(mem_fn(&Segment::assertNonZeroLength));
    }
    return true;
}
void printEdges(const Edges& es) {
    for(Edges::const_iterator e=es.begin();e!=es.end();++e) {
        cout<<(*e)->toString()<<endl;
    }
}
#endif
} // namespace topology

