#include <libvpsc/rectangle.h>
#include <libcola/cola.h>
#include <libcola/straightener.h>
#include "topology_graph.h"
#include "topology_constraints.h"
using namespace std;
using vpsc::Rectangle;
namespace topology {
    EdgePoint::~EdgePoint() {
        if(bendConstraint) {
            delete bendConstraint;
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
    straightener::Route* Edge::getRoute() {
        straightener::Route* r = new straightener::Route(nSegments+1);
        forEachEdgePointConst(copyEdgePointsToRoute(r));
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
        forEachSegmentConst(accumulateLength(totalLength));
        return totalLength;
    }
    struct buildPath {
        buildPath(ConstEdgePoints& vs) : vs(vs) {}
        void operator()(const EdgePoint* p) {
            vs.push_back(p);
        }
        ConstEdgePoints& vs;
    };
    /**
     * get a list of all the EdgePoints along the Edge path
     */
    void Edge::getPath(ConstEdgePoints& vs) const {
        forEachEdgePointConst(buildPath(vs));
    }
} // namespace topology

