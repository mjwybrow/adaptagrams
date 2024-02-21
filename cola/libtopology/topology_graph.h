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
 * Author(s):  Tim Dwyer
*/

/*
 * Class definitions for graph elements used in determining topology
 * preserving constraints.
 */

#ifndef TOPOLOGY_GRAPH_H
#define TOPOLOGY_GRAPH_H

#include <vector>
#include <functional>
#include "libvpsc/assertions.h"
#include "libvpsc/rectangle.h"

#include "libtopology/util.h"

namespace vpsc {
    class Variable;
}
namespace straightener {
    struct Route;
}
namespace topology {
    class Segment;
    class TopologyConstraint;
    class BendConstraint;
    class StraightConstraint;
    class Edge;
    /**
     * @brief  Topology representation for a node.
     *
     * Each node is associated with a rectangle and solver variables
     * for the x and y axes.
     *
     * @note You shouldn't need to create these yourself, but you may
     *       extract them from an existing ColaTopologyAddon and construct
     *       a new ColaTopologyAddon with that same topology information.
     */
    class Node {
    public:
        // the index of the associated node / variable / rectangle
        const unsigned id;
        // the bounding box of the associated node
        vpsc::Rectangle* rect;
        /*
         * When an edge path is being defined externally with a vector of
         * EdgePoint, a variable would not be specified.
         * @param id
         * @param r
         */
        Node(unsigned id, vpsc::Rectangle *r, vpsc::Variable *v = nullptr);
        void setDesiredPos(double d, double weight=1.0);
        double initialPos(vpsc::Dim scanDim) const;
        double finalPos() const;
        double posOnLine(vpsc::Dim scanDim, double alpha) const;
        vpsc::Variable* getVar() const { return var; }
        // variable positions used by solver
        vpsc::Variable* var;
    };
    /** 
     * @brief  A vector of pointers to Node objects.
     */
    typedef std::vector<Node *> Nodes;
    /*
     * let n=ns.size(), where n<=vs.size(), 
     * for i<n we set the variable for ns[i] to be vs[i].
     */
    void setNodeVariables(Nodes& ns, std::vector<vpsc::Variable*>& vs);
    /*
     * An EdgePoint is a point along an edge path.  It must correspond to
     * either the middle of a Node (the start/end of the edge) or to a corner
     * of a Node (a bend point around an edge).
     */
    class EdgePoint {
    public:
        // the node / variable / rectangle associated with this EdgePoint
        Node* node;
        // where the EdgePoint lies on the rectangle
        enum RectIntersect { 
            TR, //< top right corner
            BR, //< bottom right corner
            BL, //< bottom left corner
            TL, //< bends around rectangle's top-left corner
            CENTRE //< connected to the rectangle's centre, hence the end of the edge.
        } rectIntersect;
        // the incoming segment to this EdgePoint on the edge path
        Segment* inSegment;
        // the outgoing segment to this EdgePoint on the edge path
        Segment* outSegment;
        /* each articulation EdgePoint (where isReal()==false) 
         *  will be assigned (not immediately) a bendConstraint
         */
        BendConstraint* bendConstraint;
        // append bendConstraint (if not nullptr) to ts
        void getBendConstraint(std::vector<TopologyConstraint*>* ts);
        // @return true if constraint created
        bool createBendConstraint(vpsc::Dim scanDim);
        // delete the bendConstraint and reset pointer to nullptr
        void deleteBendConstraint();
        /*
         * Constructor associates the point with a node vertex but
         * not an edge.
         */
        EdgePoint(Node* n, RectIntersect i) 
                : node(n), rectIntersect(i)
                , inSegment(nullptr), outSegment(nullptr) 
                , bendConstraint(nullptr)
        {
        }
        /* 
         * @param dim the axis (either horizontal or
         * vertical) of the coordinate to return
         * @return the position, computed based on rectIntersect and rectangle
         * vertices of the node
         */
        double pos(vpsc::Dim dim) const;
        // @return x position
        double posX() const { return pos(vpsc::HORIZONTAL); }
        // @return y position
        double posY() const { return pos(vpsc::VERTICAL); }
        /* 
         *  @return where the EdgePoint on the rectangle as a vertex index
         *  for libavoid.
         */
        unsigned short rectIntersectAsVertexNumber(void) const
        {
            switch(rectIntersect) {
                case topology::EdgePoint::BR:
                    return 0;
                case topology::EdgePoint::TR: 
                    return 1;
                case topology::EdgePoint::TL:
                    return 2;
                case topology::EdgePoint::BL: 
                    return 3;
                default: 
                    return 4;
            }
        }
        /*
         * for any two EdgePoint the following should always be false!
         * @param e an EdgePoint (not this one)
         */
        bool uniqueCheck(const EdgePoint* e) const {
            COLA_ASSERT(this!=e);
            return node==e->node && rectIntersect==e->rectIntersect;
        }
// To prevent C++ objects from being destroyed in garbage collected languages
// when the libraries are called from SWIG, we hide the declarations of the
// destructors and prevent generation of default destructors.
#ifndef SWIG
        ~EdgePoint();
#endif
        /*
         * @return true if the EdgePoint is the end of an edge otherwise
         * asserts that it is a valid bend point.
         */
        bool isEnd() const;
        /*
         * asserts that, if this is a bend point, it does not double back in either
         * the horizontal or vertical directions.
         */
        bool assertConvexBend() const;
        /*
         * @return offset from centre of node
         */
        double offset(vpsc::Dim scanDim) const;
        /*
         * remove this point from the edge replacing its in and out
         * segments with a single new Segment.
         * @return the replacement Segment
         */
        Segment* prune(vpsc::Dim scanDim);
    };
    /*
     * A vector of pointers to EdgePoint objects.
     */
    typedef std::vector<EdgePoint *> EdgePoints;
    typedef std::vector<const EdgePoint *> ConstEdgePoints;
    /*
     * a Segment is one straightline segment between two EdgePoint which are
     * either bend points and/or ends of the edge.
     */
    class Segment {
    public:
        /*
         * Create segment for a given edge between two EdgePoints.
         * Note that segments can be zero length, for example between 
         * opposite corners of two rectangles.
         * @param edge the edge to which this segment belongs
         * @param start the EdgePoint at the start of the segment
         * @param end the EdgePoint at the end of the segment
         */
        Segment(Edge* edge, EdgePoint* start, EdgePoint* end) 
            : edge(edge), start(start), end(end)
        {
            // no self loops!
            COLA_ASSERT(start!=end);
            // the ends of the segment should not involve the same rectangle vertex
            COLA_ASSERT(!start->uniqueCheck(end));
            start->outSegment=this;
            end->inSegment=this;
        }
        /*
         * add a new StraightConstraint to this segment (if necessary)
         * @param node the node with which the constraint is associated
         * @param pos the scanPos, i.e. the position in the scan dimension
         * of the opening or closing of node.
         * @return true if a constraint was created
         */
        bool createStraightConstraint(vpsc::Dim dim, Node* node, double pos);
        /*
         * creates a copy of the StraightConstraint in our own
         * straightConstraints list, but only if this segment is not directly
         * connected to the centre of the StraightConstraint node.  @param s
         * the StraightConstraint to be copied across
         */
        void transferStraightConstraint(StraightConstraint* s);
        /*
         * this typedef can be used to declare a wrapper functor
         * for transferStraightConstraint
         */
        typedef std::function<void(StraightConstraint*)> TransferStraightConstraint;
        /*
         * TransferStraightConstraint might for example be applied to
         * forEachStraightConstraint
         */
        template <typename T>
        void forEachStraightConstraint(T f) {
            for_each(straightConstraints.begin(),straightConstraints.end(),f);
        }
        /*
         * append straightConstraints to ts
         */
        void getStraightConstraints(std::vector<TopologyConstraint*>* ts) 
            const;
        /*
         * clean up topologyConstraints
         */
        void deleteStraightConstraints();
        ~Segment();
        // the edge which this segment is part of
        Edge* edge;
        // the start point of the segment - either the end of the edge
        // if connected to a real node, or a bend point
        EdgePoint* start;
        // the end point of the segment
        EdgePoint* end;
        /*
         * @return the EdgePoint at the minimum extent of this segment on the
         * scan axis
         */
        EdgePoint* getMin(vpsc::Dim scanDim) const
        {
            if (start->pos(vpsc::conjugate(scanDim)) <=
                    end->pos(vpsc::conjugate(scanDim)))
            {
                return start;
            }
            return end;
        }
        /*
         * @return the EdgePoint on the maximum extent of this segment on the
         * scan axis
         */
        EdgePoint* getMax(vpsc::Dim scanDim) const
        {
            if (start->pos(vpsc::conjugate(scanDim)) >
                    end->pos(vpsc::conjugate(scanDim)))
            {
                return start;
            }
            return end;
        }
        /* 
         * compute the intersection with the line !dim=pos.
         * if called when Segment is parallel to scan line it will throw an
         * assertion error.
         * @param pos position of scanline
         * @param p distance along line from start to end at which intersection
         * occurs (where 0 is at the start and 1 is at the end -- though
         * note that p will be outside this range for BendConstraints).
         * @return position along scanline of intersection with the line along
         * this edge segment
         */
        double forwardIntersection(vpsc::Dim scanDim, double pos, double &p) const {
            return intersection(scanDim, pos, start, end, p);
        }
        double reverseIntersection(vpsc::Dim scanDim, double pos, double &p) const {
            return intersection(scanDim, pos, end, start, p);
        }
        double forwardIntersection(vpsc::Dim scanDim, double pos) const {
            double p;
            return forwardIntersection(scanDim, pos,p);
        }
        double intersection(vpsc::Dim scanDim, const double pos,
                const EdgePoint* s, const EdgePoint* e, double& p) const
        {
            double ux=s->pos(scanDim);
            double vx=e->pos(scanDim);
            double uy=s->pos(vpsc::conjugate(scanDim));
            double vy=e->pos(vpsc::conjugate(scanDim));
            double denom = vy - uy;
            COLA_ASSERT(denom!=0); // must not be parallel to scanline!
            p = (pos - uy)/denom;
            return ux + p * (vx-ux);
        }
        std::string toString() const;
        /*
         * Compute the length in the specified dimension.
         */
        double length(vpsc::Dim dim) const;
        /*
         * Compute the euclidean distance between #start and #end.
         */
        double length() const;
        void assertNonZeroLength() const;
        /*
         * does this segment have Node v as a CENTRE start or end point?
         */
        bool connectedToNode(const Node* v) const;
    private:
        // a set of topology constraints (left-/right-/above-/below-of
        // relationships / between this segment and nodes
        std::vector<StraightConstraint*> straightConstraints;
    };
    // do nothing operator used in ForEach
    template <typename T>
    struct NoOp {
        void operator() (T t)
        {
            COLA_UNUSED(t);
        }
    };
    /*
     * defines (hopefully just once) a loop over the bipartite linked-list
     * of Segment and EdgePoint in an Edge.
     * In the case of a cluster boundary, the edge will be a cycle, where
     * the last EdgePoint is also the first.  Thus, we process from
     * Edge::firstSegment to Edge::lastSegment.  We visit every EdgePoint
     * (i.e. nSegments+1), in the case of a cycle, the first/last
     * point will be visited (PointOp applied) twice unless noCycle is set 
     * true.
     */
    template <typename PEdge,
              typename PointOp,
              typename SegmentOp >
    void ForEach(PEdge e, PointOp po, SegmentOp so, bool noCycle=false) {
        Segment* s=e->firstSegment;
        if(!(e->cycle()&&noCycle)) {
            po(s->start);
        }
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
    /**
     * @brief Topology representation of an edge.
     *
     * An edge provides a doubly linked list of segments, each involving a pair
     * of EdgePoints.  
     *
     * @note You shouldn't need to create these yourself, but you may
     *       extract them from an existing ColaTopologyAddon and construct
     *       a new ColaTopologyAddon with that same topology information.
     */
    class Edge {
    public:
        /// id specified by user.  Can be used to match to external edge.
        unsigned id;
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
        // size of segments list headed by firstSegment
        size_t nSegments;
        /**
         * Construct an edge from a list of EdgePoint in sequence
         */
        Edge(unsigned id, double idealLength, EdgePoints &vs) 
            : id(id)
            , idealLength(idealLength)
            , firstSegment(nullptr), lastSegment(nullptr)
            , nSegments(0)
        {
            EdgePoints::iterator a=vs.begin();
            for(EdgePoints::iterator b=a+1;b!=vs.end();++a,++b) {
                Segment* s = new Segment(this,*a,*b); 
                nSegments++;
                if(firstSegment==nullptr) {
                    firstSegment = s;
                }
                lastSegment = s;
            }
        }
        /*
         * apply an operation to every Segment and EdgePoint associated with
         * this Edge 
         * @param po operation to apply to each EdgePoint
         * @param so operation to apply to each Segment
         */
        template <typename PointOp, typename SegmentOp>
        void forEach(PointOp po, SegmentOp so, bool noCycle=false) {
            ForEach<Edge*,PointOp,SegmentOp>(this,po,so,noCycle);
        }
        /*
         * apply an operation to every Segment and EdgePoint associated with
         * this Edge, without changing anything
         * @param po operation to apply to each EdgePoint
         * @param so operation to apply to each Segment
         */
        template <typename PointOp, typename SegmentOp>
        void forEach(PointOp po, SegmentOp so, bool noCycle=false) const {
            ForEach<const Edge*,PointOp,SegmentOp>(this,po,so,noCycle);
        }
        /*
         * apply an operation to every Segment associated with this Edge
         * @param o operation (a function or functor that takes a pointer to
         * a segment as an argument)
         */
        template <typename T>
        void forEachSegment(T o) {
            forEach(NoOp<EdgePoint*>(),o);
        }
        /*
         * a version of forEachSegment for const edges
         * @param o an operation on a const Segment
         */
        template <typename T>
        void forEachSegment(T o) const {
            forEach(NoOp<const EdgePoint*>(),o);
        }
        /*
         * apply an operation to every EdgePoint associated with this edge
         * @param o operation (a function or functor that takes a pointer to
         * an EdgePoint as an argument)
         * @param noCycle if the edge is a cycle don't apply o to the 
         * start/end point twice.
         */
        template <typename T>
        void forEachEdgePoint(T o, bool noCycle=false) {
            forEach(o,NoOp<Segment*>(),noCycle);
        }
        /*
         * a version of forEachEdgePoint for const edges
         * @param o an operation on a const EdgePoint
         * @param noCycle if the edge is a cycle apply o to the 
         * start/end point only once.
         */
        template <typename T>
        void forEachEdgePoint(T o, bool noCycle=false) const {
            forEach(o,NoOp<const Segment*>(),noCycle);
        }

// To prevent C++ objects from being destroyed in garbage collected languages
// when the libraries are called from SWIG, we hide the declarations of the
// destructors and prevent generation of default destructors.
#ifndef SWIG
        /*
         * cleanup segments
         */
        ~Edge() {
            forEach(delete_object(),delete_object(),true);
        }
#endif

        /*
         * the sum of the lengths of all the segments
         */
        double pathLength() const;
        /*
         * get a list of all the EdgePoints along the Edge path
         */
        void getPath(ConstEdgePoints& vs) const;
        /*
         * @return a list of the coordinates along the edge route
         */
        straightener::Route* getRoute() const;
        void getTopologyConstraints(std::vector<TopologyConstraint*>* ts) 
        const {
            forEach(
                    std::bind(
                        &EdgePoint::getBendConstraint,std::placeholders::_1,ts),
                    std::bind(
                        &Segment::getStraightConstraints,std::placeholders::_1,ts),
                    true);
        }
        bool assertConvexBends() const;
        bool cycle() const {
            return firstSegment->start==lastSegment->end;
        }
        std::string toString() const;
    };
    /**
     * @brief  A vector of pointers to Edge objects.
     */
    typedef std::vector<Edge *> Edges;
    double compute_stress(const Edges&);
    void printEdges(const Edges&);
/*
 * CrossProduct of three points: If the result is 0, the points are collinear; 
 * if it is positive, the three points (in order) constitute a "left turn", 
 * otherwise a "right turn".
 */
inline double crossProduct(
        double x0, double y0,
        double x1, double y1,
        double x2, double y2) {
    return (x1-x0)*(y2-y0)-(x2-x0)*(y1-y0);
}

#ifndef NDEBUG
    bool assertConvexBends(const Edges&);
    /*
     * Asserts that there are no intersections between any of the segments
     * in edges and rectangles in nodes
     * @param nodes containing rectangles
     * @param edges containing segments
     * @return true if assertions succeed
     */
    bool assertNoSegmentRectIntersection(const Nodes&, const Edges&);
    bool assertNoZeroLengthEdgeSegments(const Edges& es);
#endif
} // namespace topology
#endif // TOPOLOGY_GRAPH_H

