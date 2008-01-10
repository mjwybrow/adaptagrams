/**
 * Class definitions for graph elements used in determining topology
 * preserving constraints.
 *
 * \file topology_graph.h
 * \author Tim Dwyer
 * \date 2007
 */

#ifndef TOPOLOGY_GRAPH_H
#define TOPOLOGY_GRAPH_H
#include <vector>
#include "util.h"
namespace vpsc {
    class Rectangle;
}
namespace straightener {
    class Route;
}
namespace topology {
    extern cola::Dim dim;
    class Segment;
    class BendConstraint;
    class StraightConstraint;
    class Edge;
    struct VarPos {
        double initial;
        double desired;
        double posOnLine(double alpha) const {
            double d=desired-initial;
            return initial+alpha*d; 
        }
    };
    /**
     * Each node is associated with a rectangle and solver variables
     * for the x and y axes
     */
    struct Node {
        /// the index of the associated node / variable / rectangle
        const unsigned id;
        /// the bounding box of the associated node
        vpsc::Rectangle* rect;
        /// variable positions used by solver
        VarPos varPos;
        /** 
         * when an edge path is being defined externally with a vector of EdgePoint,
         * a variable would not be specified.
         * @param id
         * @param r
         */
        Node(unsigned id, vpsc::Rectangle* r);
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
            TR, ///< top right corner
            BR, ///< bottom right corner
            BL, ///< bottom left corner
            TL, ///< bends around rectangle's top-left corner
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
        /// delete the bendConstraint and reset pointer to NULL
        void deleteBendConstraint();
        /**
         * the constructor sets up the position 
         */
        EdgePoint(const Node* n, RectIntersect i) 
                : node(n), rectIntersect(i)
                , inSegment(NULL), outSegment(NULL) 
                , bendConstraint(NULL)
        {
            setPos();
        }
        /**
         * for any two EdgePoint the following should always be false!
         * @param e an EdgePoint (not this one)
         */
        bool uniqueCheck(const EdgePoint* e) const {
            assert(this!=e);
            return node==e->node && rectIntersect==e->rectIntersect;
        }
        ~EdgePoint();
        /**
         * @return true if the EdgePoint is the end of an edge otherwise
         * asserts that it is a valid bend point.
         */
        bool isEnd() const;
        /**
         * asserts that, if this is a bend point, it does not double back in either
         * the horizontal or vertical directions.
         */
        bool assertConvexBend() const;
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
    typedef std::vector<const EdgePoint*> ConstEdgePoints;
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
            : edge(edge), start(start), end(end) 
        {
            // no self loops!
            assert(start!=end);
            // the ends of the segment should not involve the same rectangle vertex
            assert(!start->uniqueCheck(end));
            start->outSegment=this;
            end->inSegment=this;
            assert(length()!=0);
        }
        /**
         * clean up topologyConstraints
         */
        void deleteStraightConstraints();
        ~Segment();
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
    /// do nothing operator used in ForEach
    template <typename T>
    struct NoOp {
        void operator() (T t) {}
    };
    /**
     * defines (hopefully just once) a loop over the bipartite linked-list
     * of Segment and EdgePoint in an Edge.
     * In the case of a cluster boundary, the edge will be a cycle, where
     * the last EdgePoint is also the first.  Thus, we process from
     * Edge::firstSegment to Edge::lastSegment.  We visit every EdgePoint
     * (i.e. nSegments+1), in the case of a cycle, the first/last
     * point will be visited (PointOp applied) twice.
     */
    template <typename _Edge=Edge*, 
              typename PointOp=NoOp<EdgePoint*>,
              typename SegmentOp=NoOp<Segment*> >
    struct ForEach {
        ForEach(_Edge e, PointOp po, SegmentOp so) {
            Segment* s=e->firstSegment;
            po(s->start);
            bool last=false;
            do {
                EdgePoint* p=s->end;
                so(s);
                if(s==e->lastSegment) {
                    last=true;
                } else {
                    s=p->outSegment;
                }
                po(p);
            } while(!last);
        }
    };
    /**
     * An Edge provides a doubly linked list of segments, each involving a pair
     * of EdgePoints
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
        /**
         * apply an operation to every Segment and EdgePoint associated with
         * this Edge 
         * @param po operation to apply to each EdgePoint
         * @param so operation to apply to each Segment
         */
        template <typename PointOp, typename SegmentOp>
        void forEach(PointOp po, SegmentOp so) {
            ForEach<Edge*,PointOp,SegmentOp>(this,po,so);
        }
        /**
         * apply an operation to every Segment and EdgePoint associated with
         * this Edge, without changing anything
         * @param po operation to apply to each EdgePoint
         * @param so operation to apply to each Segment
         */
        template <typename PointOp, typename SegmentOp>
        void forEach(PointOp po, SegmentOp so) const {
            ForEach<const Edge*,PointOp,SegmentOp>(this,po,so);
        }
        /**
         * apply an operation to every Segment associated with this Edge
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
        void forEachSegment(T o) const {
            forEach(NoOp<const EdgePoint*>(),o);
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
        void forEachEdgePoint(T o) const {
            forEach(o,NoOp<const Segment*>());
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
        void getPath(ConstEdgePoints& vs) const;
        /**
         * @return a list of the coordinates along the edge route
         */
        straightener::Route* getRoute();
        bool assertConvexBends() const;
        bool cycle() {
            return firstSegment->start==lastSegment->end;
        }
        std::string toString() const;
    };
    typedef std::vector<Edge*> Edges;
    double compute_stress(const Edges& es);
} // namespace topology
#endif // TOPOLOGY_GRAPH_H

// vim: cindent ts=4 sw=4 et tw=0 wm=0
