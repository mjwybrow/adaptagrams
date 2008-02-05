#include <sstream>
#include <libvpsc/rectangle.h>
#include <libcola/cola.h>
#include <libcola/straightener.h>
#include "topology_graph.h"
#include "topology_constraints.h"
using namespace std;
using vpsc::Rectangle;
namespace topology {
    Node::Node(unsigned id, vpsc::Rectangle* r)
        : id(id), rect(r) 
    { }
    const VarPos* Node::updateVarPos(double desired) {
        varPos.initial=rect->getCentreD(dim);     
        varPos.desired=desired;
        return &varPos;
    }
    void Node::moveRect(bool interrupted, double alpha) {
        double p=interrupted ? varPos.posOnLine(alpha)
                             : varPos.desired;
        rect->moveCentreD(dim,p);
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
        // or for segments parallel to scan line
        double v = pos[!dim];
        if(isEnd()
         ||inSegment->start->pos[!dim]==v
         ||outSegment->end->pos[!dim]==v) {
            return false;
        }
        if(bendConstraint) {
            delete bendConstraint;
        }
        bendConstraint = new BendConstraint(this);
        return true;
    }
    void EdgePoint::getBendConstraint(vector<TopologyConstraint*>* ts) {
        if(bendConstraint) {
            ts->push_back(bendConstraint);
        }
    }
    void EdgePoint::setPos() {
        double &x=pos[0],&y=pos[1];
        Rectangle* r=node->rect;
        switch(rectIntersect) {
            case TL:
                x=r->getMinX();
                y=r->getMaxY();
                break;
            case TR:
                x=r->getMaxX();
                y=r->getMaxY();
                break;
            case BL:
                x=r->getMinX();
                y=r->getMinY();
                break;
            case BR:
                x=r->getMaxX();
                y=r->getMinY();
                break;
            default:
                x=r->getCentreX();
                y=r->getCentreY();
        }
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
     * @return true if the EdgePoint is the end of an edge otherwise
     * asserts that it is a valid bend point.
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
    //return (x1-x0)*(y2-y0)-(x2-x0)*(y1-y0);
    double ux=x1-x0, uy=y1-y0;
    double vx=x2-x0, vy=y2-y0;
    normalise(ux,uy);
    normalise(vx,vy);
    return ux*vy-uy*vx;
}

    bool EdgePoint::assertConvexBend() const {
        if(inSegment && outSegment) {
            int fail=0;
            EdgePoint* u=inSegment->start;
            EdgePoint* w=outSegment->end;
            assert(
                    !(u->node->id==w->node->id
                        &&u->rectIntersect==w->rectIntersect));

            double* upos = u->pos;
            double* wpos = w->pos;

            /*
            // monotonicity:
            if(!( upos[0]<=pos[0] && pos[0]<=wpos[0]
                ||upos[0]>=pos[0] && pos[0]>=wpos[0])) {
                fail=1;
            }
            if(!( upos[1]<=pos[1] && pos[1]<=wpos[1]
                ||upos[1]>=pos[1] && pos[1]>=wpos[1]) )
            {
                fail=2;
            }
            */
            // ensure clockwise winding order
            switch(rectIntersect) {
                case TR:
                    if(upos[0]>wpos[0]||upos[0]==wpos[0]&&upos[1]<wpos[1]) {
                        swap(upos,wpos);
                    }
                    break;
                case BR:
                    if(upos[0]<wpos[0]||upos[0]==wpos[0]&&upos[1]<wpos[1]) {
                        swap(upos,wpos);
                    }
                    break;
                case BL:
                    if(upos[0]<wpos[0]||upos[0]==wpos[0]&&upos[1]>wpos[1]) {
                        swap(upos,wpos);
                    }
                    break;
                case TL:
                    if(upos[0]>wpos[0]||upos[0]==wpos[0]&&upos[1]>wpos[1]) {
                        swap(upos,wpos);
                    }
                    break;
                default:
                    // a bend point must be associated with one of the
                    // corners of a rectangle!
                    assert(false);
            }
            // angle must be a "right turn"
            double cp
                = crossProduct(upos[0],upos[1],pos[0],pos[1],wpos[0],wpos[1]);
            if(cp>1e-7) {
                fail=3;
            }
            if(fail==0) return true;
            printf("  convexity bend point test failed, condition=%d:\n",fail);
            printf("    (nid=%d,ri=%d):u={%f,%f}\n",
                    u->node->id,u->rectIntersect,upos[0],upos[1]);
            printf("    (nid=%d,ri=%d):v={%f,%f}\n",
                    node->id,rectIntersect,pos[0],pos[1]);
            printf("    (nid=%d,ri=%d):w={%f,%f}\n",
                    w->node->id,w->rectIntersect,wpos[0],wpos[1]);
            printf("    turn cross product=%e\n",cp);
            assert(false);

        } 
        return true;
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
    double Segment::length() const {
        double dx = end->pos[0] - start->pos[0];
        double dy = end->pos[1] - start->pos[1];
        return sqrt(dx*dx + dy*dy);
    }

    struct copyEdgePointsToRoute {
        copyEdgePointsToRoute(straightener::Route* r) : x(r->xs), y(r->ys) {}
        void operator() (const EdgePoint* p) {
            *x++=p->pos[0];
            *y++=p->pos[1];
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
            ss << "EP@" << p <<": pos=("
               << p->pos[0]<<","<<p->pos[1]<<")"<<endl;
        }
        stringstream& ss;
    };
    string Edge::toString() const {
        stringstream ss;
        forEachEdgePoint(PointToString(ss));
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
} // namespace topology

// vim: cindent ts=4 sw=4 et tw=0 wm=0
