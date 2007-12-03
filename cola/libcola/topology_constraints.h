/**
 * \file topology_constraints.h
 *
 * Classes used in generating and managing topology constraints, i.e. constraints
 * of the form (e.g.) \f$w_x + \frac{1}{2}\mathrm{width}(w) \le u_x + (v_x - u_x) \frac {(w_y-u_y)}{(v_y-u_y)}\f$
 * where (u,v) is an edge segment and w is a node constrained to lie to the left of
 * that segment.  Right-, above- and below-of constraints are similarly defined.
 */
#ifndef TOPOLOGY_CONSTRAINTS_H
#define TOPOLOGY_CONSTRAINTS_H
#include <vector>
#include <set>
#include <list>
#include "commondefs.h"
namespace vpsc {
    class Rectangle;
}
namespace project {
    class Variable;
    class Constraint;
}
namespace straightener {
    class Route;
}
namespace cola {
    class SparseMap;
}
/**
 * namespace for classes uses in generating and solving forces and constraints associated with
 * topology preserving layout.
 */
namespace topology {
    extern cola::Dim dim;
    using std::valarray;
    class Segment;
    class TopologyConstraint;
    class Edge;
    /**
     * Each node is associated with a rectangle and a solver variable
     */
    struct Node {
        /// the index of the associated node / variable / rectangle
        const unsigned id;
        /// the bounding box of the associated node
        vpsc::Rectangle* rect;
        /// the variable (associated with node) to be passed to libproject solver
        project::Variable* var;
        Node(unsigned id, vpsc::Rectangle* r, project::Variable* v)
            : id(id), rect(r), var(v) {}
    };
    typedef std::vector<Node*> Nodes;
    /**
     * An EdgePoint is a point along an edge path.  It must correspond to either the middle
     * of a Node (the start/end of the edge) or to a corner of a Node (a bend point around
     * an edge).
     */
    class EdgePoint {
    public:
        /// the node / variable / rectangle associated with this EdgePoint
        const Node* node;
        /** the position, computed based on rectIntersect and rectangle position and boundary
         *  @param dim the axis (either horizontal or vertical) of the coordinate to return
         */
        double pos[2];
        /// where the node lies on the rectangle
        enum RectIntersect { 
            TL, ///< bends around rectangle's top-left corner
            TR, ///< top right corner
            BR, ///< bottom right corner
            BL, ///< bottom left corner
            CENTRE ///< connected to the rectangle's centre, hence the end of the edge.
        } rectIntersect;
        /// the incoming segment to this EdgePoint on the edge path
        Segment* inSegment;
        /// the outgoing segment to this EdgePoint on the edge path
        Segment* outSegment;
        /**
         * the constructor sets up the position 
         */
        EdgePoint(Node* node, RectIntersect i) 
                : node(node), rectIntersect(i)
                , inSegment(NULL), outSegment(NULL) 
        {
            setPos();
        }
        /**
         * @return true if the EdgePoint is the end of an edge otherwise asserts that
         * it is a valid bend point.
         */
        bool isReal() const {
            if(rectIntersect==CENTRE
               && (inSegment!=NULL || outSegment!=NULL)
               && (inSegment==NULL || outSegment==NULL)
              ) {
                return true;
            }
            // verify that this is a bend around a corner of a rectangle 
            assert(rectIntersect!=CENTRE);
            assert(inSegment!=NULL && outSegment!=NULL);
            return false;
        }
        /**
         * update the position from the position of the associated rectangle
         */
        void setPos();
        /**
         * @return offset from centre of node
         */
        double offset() const;
    };
    typedef std::vector<EdgePoint*> EdgePoints;
    /**
     * a Segment is one straightline segment between two EdgePoint which are
     * either bend points and/or ends of the edge.
     */
    class Segment {
    public:
        /**
         * Create segment for a given edge between two EdgePoints.
         * @param edge the edge to which this segment belongs
         * @param start the EdgePoint at the start of the segment
         * @param end the EdgePoint at the end of the segment
         */
        Segment(Edge* edge, EdgePoint* start, EdgePoint* end) 
            : edge(edge), start(start), end(end) {
            start->outSegment=this;
            end->inSegment=this;
        }
        /// the edge which this segment is part of
        Edge* edge;
        /// the position of the segment in the list of segments associated
        /// with the edge
        std::list<Segment*>::iterator edgePos;
        /// the start point of the segment - either the end of the edge
        /// if connected to a real node, or a bend point
        EdgePoint* start;
        /// the end point of the segment
        EdgePoint* end;
        /// a set of topology constraints (left-/right-/above-/below-of
        /// relationships / between this segment and nodes
        std::vector<TopologyConstraint*> topologyConstraints;
        /// @return the EdgePoint at the minimum extent of this segment on the scan axis
        EdgePoint* getMin() const {
            if(start->pos[!dim] <= end->pos[!dim]) {
                return start;
            }
            return end;
        }
        /// @return the EdgePoint on the maximum extent of this segment on the scan axis
        EdgePoint* getMax() const {
            if(start->pos[!dim] > end->pos[!dim]) {
                return start;
            }
            return end;
        }
        /** test a given point to see if it lies within the scan range of this segment
         * @param pos position to test
         * @return true if point does lie in the scan range
         */
        bool intersects(double pos) const {
            if(pos>=getMin()->pos[!dim] && pos<=getMax()->pos[!dim]) {
                return true;
            }
            return false;
        }
        /** 
         * compute the intersection with the scanline.
         * if called when Segment is parallel to scan line it will throw an assertion error.
         * @param pos position of scanline
         * @param p distance along line from start to end at which intersection occurs (where 0
         * is at the start and 1 is at the end).
         * @return position along scanline of intersection with the line along this edge segment
         */
        double intersection(double pos, double &p) const {
            double ux=start->pos[dim] , vx=end->pos[dim],
                   uy=start->pos[!dim], vy=end->pos[!dim];
            double denom = vy - uy;
            assert(denom!=0); // must not be parallel to scanline!
            p = (pos - uy)/denom;
            return ux + p * (vx-ux);
        }
        /**
         * Compute the euclidean distance between #start and #end.
         */
        double length() const;
    };
    typedef std::list<Segment*> Segments;
    /**
     * An Edge provides a doubly linked list of segments, each involving a pair of
     * EdgePoints
     */
    class Edge {
    public:
        /**
         * Doubly linked list of segments each involving a pair of EdgePoints
         */
        Segments segments;
        /**
         * Construct an edge from a list of EdgePoint in sequence
         */
        Edge(double idealLength, EdgePoints &vs) : idealLength(idealLength) {
            EdgePoints::iterator a=vs.begin();
            for(EdgePoints::iterator b=a+1;b!=vs.end();++a,++b) {
                segments.push_back(new Segment(this,*a,*b)); 
            }
        }
        /**
         * cleanup segments
         */
        ~Edge() {
            for_each(segments.begin(),segments.end(),delete_object());
        }
        /**
         * the sum of the lengths of all the segments
         */
        double pathLength() const;
        /**
         * get a list of all the EdgePoints along the path
         */
        void getPath(EdgePoints &vs) const;
        /**
         * @return the start of the edge
         */
        EdgePoint* start() const;
        /**
         * @return the end of the edge
         */
        EdgePoint* end() const;
        /// the ideal length which the layout should try to obtain for this edge
        double idealLength;
        void splitSegment(Segment* o, Segment* n1, Segment* n2) {
            n1->edgePos=segments.insert(o->edgePos, n1);
            n2->edgePos=segments.insert(o->edgePos, n2);
            segments.erase(o->edgePos);
        }
        void mergeSegment(Segment* o1, Segment* o2, Segment* n) {
            n->edgePos=segments.insert(o1->edgePos,n);
            segments.erase(o1->edgePos);
            segments.erase(o2->edgePos);
        }
        /**
         * @return a list of the coordinates along the edge route
         */
        straightener::Route* getRoute();
        /*
        void print() {
            EdgePoint *u=NULL, *v=segments.front()->start;
            double minTheta=PI;
            for(std::list<Segment*>::const_iterator i=segments.begin();i!=segments.end();i++) {
                EdgePoint* w=(*i)->end;
                if(u) {
                    double ax=v->pos[0]-u->pos[0],
                           ay=v->pos[1]-u->pos[1],
                           bx=w->pos[0]-v->pos[0],
                           by=w->pos[1]-v->pos[1];
                    double ab=ax*bx+ay*by;
                    double la=sqrt(ax*ax+ay*ay);
                    double lb=sqrt(bx*bx+by*by);
                    double theta=PI-acos(ab/(la*lb));
                    //printf("    angle=%f,a=(%f,%f),b=(%f,%f)\n",theta,ax,ay,bx,by);
                    if(theta<minTheta) minTheta = theta;
                }
                //(*i)->print();
                u=v;
                v=w;
            }
            //assert(minTheta>PI/2.);
        }
        */
    };
    typedef std::vector<Edge*> Edges;

    /**
     * A constraint between variables \f$u,v,w\f$ where \f$w\f$ is required to be to one
     * side of the line between \f$u,v\f$.
     * That is, e.g. if we require \f$w\f$ to be some minimum distance \f$g\f$ to the left
     * of the parameterised distance \f$p\f$ along the line \f$(u,v)\f$ we have:
     * \f[ w + g\le u + p*(v - u) \f]
     * Right-of constraints are similar.
     */
    class TopologyConstraint {
    public:
        /// Variables are directly entered into the libproject solver
        project::Variable *u, *v, *w;
        /// p is the parameter for the constraint line, g is the offset constant
        double p, g;
        /// determines direction of inequality, w to the left of uv or to the right
        bool leftOf;
        TopologyConstraint(
                project::Variable *u, 
                project::Variable *v, 
                project::Variable *w, 
                double p, double g, bool left)
            : u(u), v(v), w(w), p(p), g(g), leftOf(left) {
                printf("TopologyConstraint created!\n");
        }
    };
    /**
     * Define a topology over a diagram by generating a set of TopologyConstraint
     */
    class TopologyConstraints {
    public:
        const Edges& edges;
        TopologyConstraints(
            const cola::Dim dim, 
            const Nodes &vs,
            const Edges &es);
        ~TopologyConstraints() { }
        void violated(std::vector<TopologyConstraint*> & ts) const;
        TopologyConstraint* mostViolated() const;
        void computeForces(cola::SparseMap &H, valarray<double> &g);
        double computeStress() const;
        void finalizeRoutes();
    private:
        double len(const EdgePoint* u, const EdgePoint* v, 
                double& dx, double& dy,
                double& dx2, double& dy2);
        double gRule1(const EdgePoint* a, const EdgePoint* b);
        double gRule2(const EdgePoint* a, const EdgePoint* b, const EdgePoint* c);
        double hRuleD1(const EdgePoint* u, const EdgePoint* v, const double sqrtf);
        double hRuleD2(const EdgePoint* u, const EdgePoint* v, const EdgePoint* w, 
                const double sqrtf);
        double hRule2(const EdgePoint* u, const EdgePoint* v, const EdgePoint* w,
                const double sqrtf);
        double hRule3(const EdgePoint* u, const EdgePoint* v, const EdgePoint* w,
                const double sqrtf);
        double hRule4(const EdgePoint* a, const EdgePoint* b,
                const EdgePoint* c, const EdgePoint* d);
        double hRule56(const EdgePoint* u, const EdgePoint* v, 
                const EdgePoint* a, const EdgePoint* b, const EdgePoint* c);
        double hRule7(const EdgePoint* a, const EdgePoint* b, 
                const EdgePoint* c, const EdgePoint* d, const double sqrtf);
        double hRule8(const EdgePoint* u, const EdgePoint* v, const EdgePoint* w,
                const EdgePoint* a, const EdgePoint* b, const EdgePoint* c);
    };
} // namespace topology
#endif // TOPOLOGY_CONSTRAINTS_H
// vim: cindent ts=4 sw=4 et tw=0 wm=0
