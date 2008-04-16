#include <sstream>
#include <libvpsc/rectangle.h>
#include <libcola/cola.h>
#include <libcola/straightener.h>
#include "topology_log.h"
#include "topology_graph.h"
#include "topology_constraints.h"
using namespace std;
using vpsc::Rectangle;
namespace topology {

void setNodeVariables(Nodes& ns, std::vector<vpsc::Variable*>& vs) {
    assert(ns.size()<=vs.size());
    std::vector<vpsc::Variable*>::iterator v=vs.begin();
    for(Nodes::iterator i=ns.begin();i!=ns.end();++i,++v) {
        (*i)->var=(*v);
    }
}
Node::Node(unsigned id, vpsc::Rectangle* r, vpsc::Variable* v)
    : id(id), rect(r), var(v) 
{ 
    assert(initialPos()>-10000);
    assert(initialPos()<10000);
}
Node::Node(unsigned id, vpsc::Rectangle* r)
    : id(id), rect(r), var(NULL) 
{
    assert(initialPos()>-10000);
    assert(initialPos()<10000);
}
void Node::setDesiredPos(double d, double weight) {
    var->desiredPosition=d;
    var->weight=weight;
}
double Node::initialPos() const {
    return rect->getCentreD(dim);
}
double Node::finalPos() const {
    return var->finalPosition;
}
double Node::posOnLine(double alpha) const {
    double i=initialPos();
    double d=finalPos()-i;
    return i+alpha*d; 
}
void EdgePoint::deleteBendConstraint() {
    if(bendConstraint) {
        delete bendConstraint;
        bendConstraint=NULL;
    }
}
EdgePoint::~EdgePoint() {
    deleteBendConstraint();
}
bool EdgePoint::createBendConstraint() {
    // edges shouldn't double back!
    assert(assertConvexBend());
    // don't generate BendConstraints for Edge end points
    if(bendConstraint) {
        delete bendConstraint;
        bendConstraint=NULL;
    }
    if(isEnd()) {
        return false;
    }
    bendConstraint = new BendConstraint(this);
    return true;
}
void EdgePoint::getBendConstraint(vector<TopologyConstraint*>* ts) {
    if(bendConstraint) {
        ts->push_back(bendConstraint);
    }
}
double EdgePoint::pos(unsigned dim) const {
    double p;
    Rectangle* r=node->rect;
    switch(rectIntersect) {
        case TL:
            p=dim==cola::HORIZONTAL?
                r->getMinX():r->getMaxY();
            break;
        case TR:
            p=r->getMaxD(dim);
            break;
        case BL:
            p=r->getMinD(dim);
            break;
        case BR:
            p=dim==cola::HORIZONTAL?
                r->getMaxX():r->getMinY();
            break;
        default:
            p=r->getCentreD(dim);
    }
    return p;
}
double EdgePoint::offset() const {
    if(rectIntersect==CENTRE) {
        return 0;
    }
    double o = node->rect->length(dim)/2.0;
    if(dim==cola::HORIZONTAL && (rectIntersect == TL || rectIntersect == BL)
     ||dim==cola::VERTICAL && (rectIntersect == BL || rectIntersect == BR)) {
            return -o;
        }
        return o;
    }
    /**
     * @return true if the EdgePoint is the end of an edge.
     * Note that cluster boundary edges are cycles, and therefore have no
     * ends.
     */
    bool EdgePoint::isEnd() const {
        if(outSegment==NULL || inSegment==NULL) {
            return true;
        }
        return false;
    }
inline void normalise(double& x, double& y) {
    double l=sqrt(x*x+y*y);
    x/=l;
    y/=l;
}
/**
 * CrossProduct of three points: If the result is 0, the points are collinear; 
 * if it is positive, the three points (in order) constitute a "left turn", 
 * otherwise a "right turn".
 */
inline double crossProduct(
        double x0, double y0,
        double x1, double y1,
        double x2, double y2) {
    return (x1-x0)*(y2-y0)-(x2-x0)*(y1-y0);
    /*
    double ux=x1-x0, uy=y1-y0;
    double vx=x2-x0, vy=y2-y0;
    normalise(ux,uy);
    normalise(vx,vy);
    return ux*vy-uy*vx;
    */
}

bool EdgePoint::assertConvexBend() const {
    const double eps=1e-7;
    if(inSegment && outSegment 
       && inSegment->length()>eps && outSegment->length()>eps) 
    {
        int fail=0;
        EdgePoint* u=inSegment->start;
        EdgePoint* w=outSegment->end;
        assert(
                !(u->node->id==w->node->id
                    &&u->rectIntersect==w->rectIntersect));

        if(w->posX()==posX()&&u->posX()==posX()) {
            assert( u->posY()<=posY()+eps && posY()<=w->posY()+eps
                  ||u->posY()>=posY()-eps && posY()>=w->posY()-eps);
            return true;
        }
        if(w->posY()==posY()&&u->posY()==posY()) {
            assert( u->posX()<=posX()+eps && posX()<=w->posX()+eps
                  ||u->posX()>=posX()-eps && posX()>=w->posX()-eps);
            return true;
        }

        // monotonicity:
        if(!( u->posX()<=posX()+eps && posX()<=w->posX()+eps
            ||u->posX()>=posX()-eps && posX()>=w->posX()-eps)) {
            fail=1;
        }
        if(!( u->posY()<=posY()+eps && posY()<=w->posY()+eps
            ||u->posY()>=posY()-eps && posY()>=w->posY()-eps) )
        {
            fail=2;
        }
        // cp>0: left turn, cp<0: right turn
        double cp = crossProduct(u->posX(),u->posY(),posX(),posY(),w->posX(),w->posY());
        double dx = w->posX() - u->posX(), dy = w->posY() - u->posY();
        // ensure tight turn
        switch(rectIntersect) {
            case TR:
                if(dx>0) { // ux<wx
                    if(dy>0) { // uy<wy
                        fail=3;
                    } else if(cp>eps) {
                        fail=4;
                    }
                } else { // ux>=wx
                    if(dy<0) {
                        fail=5;
                    } else if(cp<-eps) {
                        fail=6;
                    }
                }
                break;
            case BR:
                if(dx>0) { // ux<wx
                    if(dy<0) {
                        fail=7;
                    } else if(cp<-eps) {
                        fail=8;
                    }
                } else { // ux>=wx
                    if(dy>0) {
                        fail=9;
                    } else if(cp>eps) {
                        fail=10;
                    }
                }
                break;
            case BL:
                if(dx>0) { //ux<wx
                    if(dy>0) {
                        fail=11;
                    } else if(cp<-eps) {
                        fail=12;
                    }
                } else { //ux>=wx
                    if(dy<0) {
                        fail=13;
                    } else if(cp>eps) {
                        fail=14;
                    }
                }
                break;
            case TL:
                if(dx>0) { // ux<wx
                    if(dy<0) { // uy>wy
                        fail=15;
                    } else if(cp>eps) {
                        fail=16;
                    }
                } else { // ux>=wx
                    if(dy>0) {
                        fail=17;
                    } else if(cp<-eps) {
                        fail=18;
                    }
                }
                break;
            default:
                // a bend point must be associated with one of the
                // corners of a rectangle!
                assert(false);
        }
        if(fail==0) return true;
        printf("  convexity bend point test failed, condition=%d, dx=%f, dy=%f, cp=%f:\n",fail,dx,dy,cp);
        printf("    (nid=%d,ri=%d):u={%f,%f}\n",
                u->node->id,u->rectIntersect,u->posX(),u->posY());
        printf("    (nid=%d,ri=%d):v={%f,%f}\n",
                node->id,rectIntersect,posX(),posY());
        printf("    (nid=%d,ri=%d):w={%f,%f}\n",
                w->node->id,w->rectIntersect,w->posX(),w->posY());
        printf("    turn cross product=%e\n",cp);
        assert(false);

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
/**
 * clean up topologyConstraints
 */
Segment::~Segment() {
    deleteStraightConstraints();
}
double Segment::length(unsigned dim) const {
    return fabs(end->pos(dim)-start->pos(dim));
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
    forEachEdgePoint(mem_fun(&EdgePoint::assertConvexBend),true);
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
/**
 * Get a list of all the EdgePoints along the Edge path.  Note that for
 * cycles the start/end point will be at the start and end of the list,
 * i.e. it will appear in the list twice.
 */
void Edge::getPath(ConstEdgePoints& vs) const {
    forEachEdgePoint(buildPath(vs));
}
bool assertConvexBends(const Edges& es) {
    for_each(es.begin(),es.end(),mem_fun(&Edge::assertConvexBends));
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
                assert(s->start->rectIntersect==EdgePoint::BL
                        &&s->end->rectIntersect==EdgePoint::BR
                     ||s->start->rectIntersect==EdgePoint::BR
                        &&s->end->rectIntersect==EdgePoint::BL
                     ||s->start->rectIntersect==EdgePoint::BL
                        &&s->end->rectIntersect==EdgePoint::TL
                     ||s->start->rectIntersect==EdgePoint::TL
                        &&s->end->rectIntersect==EdgePoint::BL
                     ||s->start->rectIntersect==EdgePoint::BR
                        &&s->end->rectIntersect==EdgePoint::TR
                     ||s->start->rectIntersect==EdgePoint::TR
                        &&s->end->rectIntersect==EdgePoint::BR
                     ||s->start->rectIntersect==EdgePoint::TL
                        &&s->end->rectIntersect==EdgePoint::TR
                     ||s->start->rectIntersect==EdgePoint::TR
                        &&s->end->rectIntersect==EdgePoint::TL);
                continue;
            }
            double sx=s->start->posX(), sy=s->start->posY(),
                   ex=s->end->posX(), ey=s->end->posY();
            double xBorder=Rectangle::xBorder, yBorder=Rectangle::yBorder;
            Rectangle::setXBorder(xBorder-1e-6);
            Rectangle::setYBorder(yBorder-1e-6);
            if((*v)->rect->overlaps(sx,sy,ex,ey)) {
                printf("ERROR: Segment on edge id=%d overlaps Node id=%d\n",
                        s->edge->id,(*v)->id);
                assert(false);
            }
            Rectangle::setXBorder(xBorder);
            Rectangle::setYBorder(yBorder);
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
void printEdges(const Edges& es) {
    for(Edges::const_iterator e=es.begin();e!=es.end();++e) {
        cout<<(*e)->toString()<<endl;
    }
}
#endif
} // namespace topology

// vim: cindent ts=4 sw=4 et tw=0 wm=0
