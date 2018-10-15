/*
 * vim: ts=4 sw=4 et tw=0 wm=0
 *
 * libdialect - A library for computing DiAlEcT layouts:
 *                 D = Decompose/Distribute
 *                 A = Arrange
 *                 E = Expand/Emend
 *                 T = Transform
 *
 * Copyright (C) 2018  Monash University
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
 * Author(s):   Steve Kieffer   <http://skieffer.info>
*/

#ifndef DIALECT_PLANARISE_H
#define DIALECT_PLANARISE_H

#include <vector>
#include <utility>

#include "libvpsc/rectangle.h"

#include "libdialect/commontypes.h"
#include "libdialect/constraints.h"

namespace dialect {

struct Event;

//! Represents an axis-aligned segment of an orthogonal connector route.
struct EdgeSegment {

    //! @brief  Standard constructor
    //! @param[in] node1  a Node representing one endpoint of the segment
    //! @param[in] node2  a Node representing the other endpoint of the segment
    EdgeSegment(Node_SP node1, Node_SP node2);

    //! @brief  Substitute a new Node as the closing node of the segment.
    //!         This is useful during our scan line process.
    void setNewClosingNode(Node_SP u);

    //! @brief  Generate the two Events, in order, representing the opening
    //!         and closing of this segment.
    std::pair<Event*, Event*> getEvents(void);

    //! @brief  Update a SepMatrix with a constraint requiring this segment to
    //!         remain aligned and at least its current length.
    void addSep(SepMatrix &m) const;

    //! The orientation of the segment (horizontal or vertical):
    vpsc::Dim orientation;
    //! The value of the coordinate that does not change:
    double constCoord;
    //! The lower bound on the coordinate that does change:
    double lowerBound;
    //! The upper bound on the coordinate that does change:
    double upperBound;
    //! The node that sits at the end where the lower bound on the variable coordinate occurs:
    Node_SP openingNode;
    //! The node that sits at the end where the upper bound on the variable coordinate occurs:
    Node_SP closingNode;

};

enum class EventType {
    // Note: It is actually important that they be in this order:
    CLOSE, SUSTAIN, OPEN
    // (See CompareActiveEvents function.)
};

struct Event {

    //! @brief  Constructor for use with planarisation.
    Event(EdgeSegment *seg, Node_SP endpt, EventType type);

    //! @brief  Constructor for use with the "near alignments" process.
    //!         See nearalign.h.
    Event(double varCoord, Node_SP node, EventType type);

    Node_SP getNode(void) { return endpt; }

    EdgeSegment *seg;
    Node_SP endpt;
    double constCoord;
    double varCoord;
    EventType type;
    Event *companion;
    double x(void);
    double y(void);
};

//! We need a special function for comparing Events, using a positive tolerance.
//! Here's why. Suppose vertical segment A has its south end at (0, 0), and horizontal
//! segment B has its east end at (0, -0.00000000001). This means that /technically/
//! A and B intersect. However (http://xkcd.com/1475/) you probably don't actually want
//! to treat this as an intersection. The comparison function is designed so that,
//! when the list of active events is sorted, the "close" event for segment A will come
//! /before/ the "sustain" event for segment B, instead of the other way around, as
//! dictated by their exact y-coordinates. This way we will /not/ detect an intersection
//! between A and B.
bool CompareActiveEvents(Event *a, Event *b);

typedef std::vector<EdgeSegment*> EdgeSegments;
typedef std::vector<Event*> Events;
typedef std::vector<Nodes> NodeGroups;

struct OrthoPlanariserOptions {
    //! When planarising an orthogonally routed graph, we may generate new nodes to
    //! stand in place of bend points and crossing points. Unconstrained, these would
    //! introduce new articulation points, at which formerly straight connector segments
    //! could now bend. If it is desired that these new nodes be so constrained that
    //! no such bends can occur, leave this option set to true.
    bool generateConstraints = true;
};

//! This class is for planarising orthogonally routed graphs.
//! To say that a graph G is orthogonally routed means a certain condition on
//! the connector route stored in each Edge; namely, that precisely one coordinate
//! change between consecutive route points.
class OrthoPlanariser {
public:
    //! @brief  Standard constructor.
    //!
    //! @param[in] G  The Graph to be planarised.
    //!
    //! @note  The given Graph is not altered. A new, planar version is constructed.
    OrthoPlanariser(const Graph_SP &G) : m_givenGraph(G) {}

    /* Memory model:
     *
     * Many objects are allocated in the lifecycle of an OrthoPlanariser, but
     * cleaning up is nevertheless quite simple.
     *
     * Among the three Graphs -- given, overlap-free, and planar -- the first
     * and the last are the client's responsibility, since those are this object's
     * input and output. The overlap-free graph is an intermediate workspace and
     * should be destroyed, and it will be destroyed automatically, since all the
     * allocated objects of which it is made -- Nodes, Edges, and the Graph itself --
     * are managed solely by shared pointers.
     *
     * Apart from Graphs, Nodes, and Edges, we allocate EdgeSegments and Events.
     *
     * Events are always cleaned up before even exiting those methods that use them
     * (namely computeNodeGroups and computeCrossings).
     *
     * In every place where an EdgeSegment is allocated (there are only two placed:
     * buildSegments, and computeCrossings), a pointer is pushed into m_edgeSegments.
     * Pointers are removed from that vector in only one place: the
     * deleteSegments method; and here of course, it comes only after deleting those
     * EdgeSegments to which they pointed. Therefore calling deleteSegments is all
     * that is needed to free all EdgeSegments ever allocated. Since that is called
     * at the end of the planarise method, there is nothing for the destructor to do.
     */
    ~OrthoPlanariser(void) {}

    //! @brief  Set the options.
    void setOpts(const OrthoPlanariserOptions &opts) { m_opts = opts; }

    //! @brief  Compute a planarisation of the given Graph.
    //! @return  The planarised Graph.
    Graph_SP planarise(void) {
        // There are two steps.
        // (1) First we construct an overlap-free graph based on the given one.
        // In this graph, there are no edge-overlaps, but there may still be edge-crossings.
        removeEdgeOverlaps();
        // (2) Then we construct a planar graph based on the overlap-free one.
        // Here we remove any edge-crossings.
        removeEdgeCrossings();
        // There are some allocated EdgeSegment objects left over afterward.
        // Clean them up now, since they are no longer needed.
        deleteSegments();
        // The planar graph is the result we produce for the client.
        return m_planarGraph;
    }

private:

    //! @brief  Remove the edge overlaps in the given routed orthogonal graph.
    //!
    //! Compute new Graph having all the nodes of the given Graph G, plus
    //! a node for each bend point in the routes of G. Its edges
    //! cover the routes of G, in the sense of point-sets in the
    //! plane, but none of its edges overlap.
    void removeEdgeOverlaps(void);
    //! @brief  Remove the edge crossings in the overlap-free graph.
    //!
    //! Again, a new Graph is computed, having all the nodes of hte overlap-free one,
    //! plus a "crossing node" at each place where edges cross in that one.
    //! This graph is planar.
    void removeEdgeCrossings(void);
    //! @brief  Build EdgeSegment objects for all the Edges in the Graph G.
    //!         Store the segments in m_edgeSegments.
    void buildSegments(Graph_SP G);
    //! @brief  Clean up m_edgeSegments, freeing allocated objects.
    void deleteSegments(void);
    //! @brief  A utility function for removing edge overlaps.
    //! @param[in] segs  a vector of EdgeSegments, all of which should be of
    //!                 the same orientation
    //! @return  NodeGroups (vector of vectors of Node_SP's), where each group
    //!          gives the Nodes (bend or otherwise) that participate in a common
    //!          sequence of overlapping segments, listed in order of increasing
    //!          variable-coord.
    NodeGroups computeNodeGroups(EdgeSegments segs);
    //! @brief  Locate and remove all edge crossings in the overlap-free graph.
    //!
    //! This is the method that actually generates new nodes and new edge segments,
    //! each time an intersection is found. Meanwhile it is the removeEdgeCrossings
    //! method that actually puts it all together into a new, planar graph.
    //!
    //! @return  The new crossing nodes generated. (New edge segments are just stashed
    //!          in m_edgeSegments.)
    Nodes computeCrossings(void);

    OrthoPlanariserOptions m_opts;

    EdgeSegments m_edgeSegments;

    Graph_SP m_givenGraph;
    Graph_SP m_overlapFreeGraph;
    Graph_SP m_planarGraph;
};


} // namespace dialect

#endif // DIALECT_PLANARISE_H
