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
    class BendConstraint;
    class StraightConstraint;
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
        /** each articulation EdgePoint (where isReal()==false) 
         *  will be assigned (not immediately) a bendConstraint
         */
        BendConstraint* bendConstraint;
        /**
         * the constructor sets up the position 
         */
        EdgePoint(const Node* node, RectIntersect i) 
                : node(node), rectIntersect(i)
                , inSegment(NULL), outSegment(NULL) 
                , bendConstraint(NULL)
        {
            setPos();
        }
        ~EdgePoint();
        /**
         * @return true if the EdgePoint is the end of an edge otherwise
         * asserts that it is a valid bend point.
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
        /**
         * clean up topologyConstraints
         */
        ~Segment() {
            for_each(straightConstraints.begin(),straightConstraints.end(),
                    delete_object());
        }
        /// the edge which this segment is part of
        Edge* edge;
        /// the start point of the segment - either the end of the edge
        /// if connected to a real node, or a bend point
        EdgePoint* start;
        /// the end point of the segment
        EdgePoint* end;
        /// a set of topology constraints (left-/right-/above-/below-of
        /// relationships / between this segment and nodes
        std::vector<StraightConstraint*> straightConstraints;
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
        /** test a given point to see if it lies within the scan range of this
         * segment
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
         * compute the intersection with the line !dim=pos.
         * if called when Segment is parallel to scan line it will throw an
         * assertion error.
         * @param pos position of scanline
         * @param p distance along line from start to end at which intersection
         * occurs (where 0 is at the start and 1 is at the end).
         * @return position along scanline of intersection with the line along
         * this edge segment
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
    template <typename T>
    struct NoOp {
        void operator() (T t) {}
    };
    /**
     * An Edge provides a doubly linked list of segments, each involving a pair of
     * EdgePoints
     */
    class Edge {
    public:
        /// the ideal length which the layout should try to obtain for this edge
        double idealLength;
        /**
         * Head of a doubly-linked list of Segment each involving a pair of
         * EdgePoints
         */
        Segment* firstSegment;
        /**
         * End of list of Segment
         */
        Segment* lastSegment;
        /// size of segments list headed by firstSegment
        size_t nSegments;
        /**
         * Construct an edge from a list of EdgePoint in sequence
         */
        Edge(double idealLength, EdgePoints &vs) 
            : idealLength(idealLength)
            , firstSegment(NULL), lastSegment(NULL)
            , nSegments(0) 
        {
            EdgePoints::iterator a=vs.begin();
            for(EdgePoints::iterator b=a+1;b!=vs.end();++a,++b) {
                Segment* s = new Segment(this,*a,*b); 
                nSegments++;
                if(firstSegment==NULL) {
                    firstSegment = s;
                }
                lastSegment = s;
            }
        }
        template <typename PointOp, typename SegmentOp>
        void forEach(PointOp po, SegmentOp so) {
            Segment* s=firstSegment;
            po(s->start);
            while(s!=NULL) {
                EdgePoint* p=s->end;
                so(s);
                s=p->outSegment;
                po(p);
            }
        }
        template <typename PointOp, typename SegmentOp>
        void forEachConst(PointOp po, SegmentOp so) const {
            const Segment* s=firstSegment;
            po(s->start);
            while(s!=NULL) {
                const EdgePoint* p=s->end;
                so(s);
                s=p->outSegment;
                po(p);
            }
        }
        /**
         * apply an operation to every segment associated with this edge
         * @param o operation (a function or functor that takes a pointer to
         * a segment as an argument)
         */
        template <typename T>
        void forEachSegment(T o) {
            forEach(NoOp<EdgePoint*>(),o);
        }
        /**
         * a version of forEachSegment for const edges
         * @param o an operation on a const Segment
         */
        template <typename T>
        void forEachSegmentConst(T o) const {
            forEachConst(NoOp<const EdgePoint*>(),o);
        }
        /**
         * apply an operation to every EdgePoint associated with this edge
         * @param o operation (a function or functor that takes a pointer to
         * an EdgePoint as an argument)
         */
        template <typename T>
        void forEachEdgePoint(T o) {
            forEach(o,NoOp<Segment*>());
        }
        /**
         * a version of forEachEdgePoint for const edges
         * @param o an operation on a const EdgePoint
         */
        template <typename T>
        void forEachEdgePointConst(T o) const {
            forEachConst(o,NoOp<const Segment*>());
        }
        /**
         * cleanup segments
         */
        ~Edge() {
            forEach(delete_object(),delete_object());
        }
        /**
         * the sum of the lengths of all the segments
         */
        double pathLength() const;
        /**
         * get a list of all the EdgePoints along the Edge path
         */
        void getPath(EdgePoints &vs) const;
        /**
         * @return a list of the coordinates along the edge route
         */
        straightener::Route* getRoute();
    };
    typedef std::vector<Edge*> Edges;

    /**
     * A constraint between three variables \f$u,v,w\f$ where \f$w\f$ is
     * required to be to one side of the line between \f$u,v\f$.  That is, e.g.
     * if we require \f$w\f$ to be some minimum distance \f$g\f$ to the left of
     * the parameterised distance \f$p\f$ along the line \f$(u,v)\f$ we have:
     * \f[ w + g\le u + p*(v - u) \f] Right-of constraints are similar.
     */
    class TriConstraint {
    public:
        /// Variables are directly entered into the libproject solver
        project::Variable *u, *v, *w;
        /// p is the parameter for the constraint line, g is the offset constant
        double p, g;
        /** 
         * determines direction of inequality, w to the left of uv or to the
         * right
         */
        bool leftOf;
        TriConstraint(
                project::Variable *u, 
                project::Variable *v, 
                project::Variable *w, 
                double p, double g, bool left)
            : u(u), v(v), w(w), p(p), g(g), leftOf(left) {
                printf("TopologyConstraint created!\n");
        }
        /** 
         * @return the maximum move we can make along the line from initial to
         * desired positions without violating this constraint
         */
        double maxSafeAlpha() const;
        double slack () const;
        void print() const;
    };
    class TopologyConstraint {
    public:
        TriConstraint* c;
        /**
         * depending on the type of constraint (i.e. whether it is a constraint
         * between a segment and a node or between two segments) we either
         * split the segment (creating a new bend EdgePoint) or merge 
         * the segment with its neighbour (removing an EdgePoint).
         */
        virtual void satisfy() = 0;
        virtual ~TopologyConstraint() {
            delete c;
        }
    };
    /**
     * A constraint around a bend point that becomes active when the bend
     * goes straight
     */
    class BendConstraint : public TopologyConstraint {
    public:
        EdgePoint* bendPoint;
        /**
         * create a constraint between the two segments joined by this
         * EdgePoint such that the constraint is activated when the segments
         * are aligned.
         * @param bendPoint the articulation point
         */
        BendConstraint(EdgePoint* bendPoint);
        void satisfy();
    };
    /**
     * A constraint between a Node and a Segment that is activated when
     * the Node wants to move through the Segment to create a bend point
     */
    class StraightConstraint : public TopologyConstraint {
    public:
        Segment* segment;
        const Node* node;
        const double pos;
        EdgePoint::RectIntersect ri;
        /** 
         * create a constraint between a segment and one corner of a node such
         * that the constraint is activated when the segment needs to be bent
         * (divided into two new segments)
         * @param s the segment
         * @param node the node
         * @param pos the position of the scan line along which the constraint
         * lies
         */
        StraightConstraint(Segment* s, const Node* node, double pos);
        void satisfy();
    };
    /**
     * Define a topology over a diagram by generating a set of
     * TopologyConstraint
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
        void constraints(std::vector<TopologyConstraint*> & ts) const;
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
