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

#include <math.h>
#include <utility>
#include <memory>
#include <vector>
#include <algorithm>
#include <set>

#include "libavoid/geomtypes.h"

#include "libdialect/commontypes.h"
#include "libdialect/ortho.h"
#include "libdialect/graphs.h"
#include "libdialect/planarise.h"
#include "libdialect/util.h"
#include "libdialect/constraints.h"

using namespace dialect;

using std::pair;
using std::vector;

using Avoid::Point;

EdgeSegment::EdgeSegment(Node_SP node1, Node_SP node2) {
    Point c1 = node1->getCentre(),
          c2 = node2->getCentre();
    double dx = c2.x - c1.x,
           dy = c2.y - c1.y;
    if (fabs(dy) <= fabs(dx)) {
        orientation = vpsc::HORIZONTAL;
        constCoord = c1.y;
        if (dx > 0) {
            lowerBound = c1.x;
            upperBound = c2.x;
            openingNode = node1;
            closingNode = node2;
        } else {
            lowerBound = c2.x;
            upperBound = c1.x;
            openingNode = node2;
            closingNode = node1;
        }
    } else {
        orientation = vpsc::VERTICAL;
        constCoord = c1.x;
        if (dy > 0) {
            lowerBound = c1.y;
            upperBound = c2.y;
            openingNode = node1;
            closingNode = node2;
        } else {
            lowerBound = c2.y;
            upperBound = c1.y;
            openingNode = node2;
            closingNode = node1;
        }
    }
}

void EdgeSegment::setNewClosingNode(Node_SP u) {
    closingNode = u;
    Point c = u->getCentre();
    upperBound = orientation == vpsc::HORIZONTAL ? c.x : c.y;
}

std::pair<Event*, Event*> EdgeSegment::getEvents(void) {
    Event *opening = new Event(this, openingNode, EventType::OPEN);
    Event *closing = new Event(this, closingNode, EventType::CLOSE);
    opening->companion = closing;
    closing->companion = opening;
    return {opening, closing};
}

void EdgeSegment::addSep(SepMatrix &m) const {
    SepDir d = orientation == vpsc::HORIZONTAL ? SepDir::EAST : SepDir::SOUTH;
    m.addSep(openingNode->id(), closingNode->id(), GapType::CENTRE, d, SepType::INEQ, upperBound - lowerBound);
}

Event::Event(EdgeSegment *seg, Node_SP endpt, EventType type)
    : seg(seg),
      endpt(endpt),
      type(type),
      companion(nullptr)
{
    Point c = endpt->getCentre();
    constCoord = seg->orientation == vpsc::HORIZONTAL ? c.y : c.x;
    varCoord = seg->orientation == vpsc::HORIZONTAL ? c.x : c.y;
}

Event::Event(double varCoord, Node_SP node, EventType type)
    : seg(nullptr),
      endpt(node),
      varCoord(varCoord),
      type(type),
      companion(nullptr) {}

double Event::x(void) {
    return endpt->getCentre().x;
}

double Event::y(void) {
    return endpt->getCentre().y;
}

void OrthoPlanariser::removeEdgeOverlaps(void) {
    // Initialise the new Graph and give it a GhostNode to represent each node in the
    // given Graph G.
    // The GhostNodes will be set to masquerade as the original Nodes, i.e. to report
    // the original Nodes' IDs. This helps us to later create the Edges for the new Graph.
    m_overlapFreeGraph = std::make_shared<Graph>();
    for (auto pair : m_givenGraph->getNodeLookup()) {
        Node_SP &u = pair.second;
        GhostNode_SP g = GhostNode::allocate(*u);
        g->setMasquerade(true);
        m_overlapFreeGraph->addNode(g);
    }
    // We also add a Node for every bend point in G.
    // When we ask G to create the unique bend point nodes, as a side-effect those nodes
    // are also set the "bend nodes" of the Edges.
    NodesById bps = m_givenGraph->buildUniqueBendPoints();
    for (auto pair : bps) {
        Node_SP &b = pair.second;
        m_overlapFreeGraph->addNode(b);
    }
    // Since G's Edges now have their bend nodes, we can now build segments to represent the
    // connector routes.
    buildSegments(m_givenGraph);
    // Sort the segments by orientation.
    size_t n = m_edgeSegments.size();
    EdgeSegments hsegs, vsegs;
    hsegs.reserve(n);
    vsegs.reserve(n);
    for (EdgeSegment *seg : m_edgeSegments) {
        if (seg->orientation == vpsc::HORIZONTAL) {
            hsegs.push_back(seg);
        } else {
            vsegs.push_back(seg);
        }
    }
    hsegs.shrink_to_fit();
    vsegs.shrink_to_fit();
    // Compute node groups
    NodeGroups hgps = computeNodeGroups(hsegs),
               vgps = computeNodeGroups(vsegs);
    // Finally, we can construct the Edges for the new graph based on the node groups.
    vector<NodeGroups> allGps{hgps, vgps};
    for (NodeGroups gps: allGps) {
        for (Nodes gp: gps) {
            for (size_t i = 0; i + 1 < gp.size(); ++i) {
                // Subtle point: the Nodes in the groups are from the original graph.
                // We want to construct Edges not on these, but on Nodes of the same
                // ID, in the new overlap-free Graph.
                Node_SP &s_orig = gp[i],
                        &t_orig = gp[i+1],
                        s = m_overlapFreeGraph->getNode(s_orig->id()),
                        t = m_overlapFreeGraph->getNode(t_orig->id());
                Edge_SP e = Edge::allocate(s, t);
                m_overlapFreeGraph->addEdge(e);
            }
        }
    }
}


void OrthoPlanariser::buildSegments(Graph_SP G) {
    // We use this method twice, on different graphs. So we begin by making
    // sure any old segments are cleared out.
    deleteSegments();
    // Iterate over the edges of the graph.
    for (auto pair : G->getEdgeLookup()) {
        Edge_SP &e = pair.second;
        Nodes bps = e->getBendNodes();
        size_t num_bends = bps.size();
        Node_SP src = e->getSourceEnd(),
                tgt = e->getTargetEnd();
        if (num_bends == 0) {
            // In this case we only need a segment from src node to tgt node.
            m_edgeSegments.push_back(new EdgeSegment(src, tgt));
        } else {
            // Otherwise we start with a segment from src node to first bp.
            m_edgeSegments.push_back(new EdgeSegment(src, bps.front()));
            // Then run a sliding window of length two over the bps to create internal segments.
            for (size_t i = 0; i < num_bends - 1; ++i) {
                m_edgeSegments.push_back(new EdgeSegment(bps[i], bps[i+1]));
            }
            // And finish with a segment from the last bp to the tgt node.
            m_edgeSegments.push_back(new EdgeSegment(bps.back(), tgt));
        }
    }
}

void OrthoPlanariser::deleteSegments(void) {
    for (EdgeSegment *s : m_edgeSegments) {
        delete s;
    }
    m_edgeSegments.clear();
}

NodeGroups OrthoPlanariser::computeNodeGroups(EdgeSegments segs) {
    // Initialise return value.
    NodeGroups groups;
    // Partition the segments by their constant coordinate, with (experimentally determined) half-integer tolerance.
    vector<EdgeSegments> parts = partition<EdgeSegment*>(segs, [](EdgeSegment *s) -> double {return s->constCoord;}, 0.5);
    // Compute Node groups for each part.
    for (EdgeSegments part : parts) {
        // Build vector of events.
        Events evts;
        for (EdgeSegment *seg : part) {
            pair<Event*, Event*> p = seg->getEvents();
            evts.push_back(p.first);
            evts.push_back(p.second);
        }
        // Sort events by increasing variable coord.
        std::sort(evts.begin(), evts.end(), [](Event *a, Event *b) -> bool {return a->varCoord < b->varCoord;});
        // Initialise an empty Node group, and set of open segments.
        Nodes group;
        std::set<EdgeSegment*> openSegs;
        // Iterate over the events.
        for (Event *evt : evts) {
            Node_SP &endpt = evt->endpt;
            // Add the endpt to the group, but not if the group already ends with this Node.
            if (group.size() == 0 || group.back() != endpt) group.push_back(endpt);
            // Keep track of which segments are open.
            if (evt->type == EventType::OPEN) {
                openSegs.insert(evt->seg);
            } else if (evt->type == EventType::CLOSE) {
                openSegs.erase(evt->seg);
                // If no open segments remain, this group is complete.
                if (openSegs.empty()) {
                    // Record a copy of the group.
                    groups.push_back(group);
                    // Clear it, for the next group.
                    group.clear();
                }
            }
            // We are done using this Event, so we can delete it right now.
            delete evt;
        }
    }
    // Return
    return groups;
}

bool dialect::CompareActiveEvents(Event *a, Event *b) {
    // Experimentally determined tolerance:
    const double TOLERANCE = 1.0;
    double ya = a->y(),
           yb = b->y();
    // If the y-coord of the second Event is greater than that of the first Event
    // by more than the tolerance, then we say that the first Event comes first.
    if (yb - ya > TOLERANCE) return true;
    // If the other way around, the second Event comes first.
    if (ya - yb > TOLERANCE) return false;
    // Otherwise the two coords are within the tolerance of one another, so we base
    // the ordering instead on the event types.
    // We want CLOSE to come before SUSTAIN, which comes before OPEN.
    // We have defined the enum items in this order, so the following test works:
    return a->type < b->type;
}

Nodes OrthoPlanariser::computeCrossings(void) {
    /* Plan: Every edge in the overlap-free graph is a simple, straight segment. We build an
     * EdgeSegment to represent each of these, and collect the open and close events for all
     * segments. We partition the events by x-coord, and we process the parts one at a time,
     * working from left to right. As we go, we maintain the set of open horizontal segments.
     * As we work from top to bottom through a given x-part, we will encounter open and close
     * events for the vertical segments having that x-coord, as well as sustain events for
     * any horizontal segments that are still open at this x-coord. There is an intersection
     * if and only if there is an open horizontal segment while a vertical segment is also
     * open. When we encounter an intersection, we (a) create a new node to represent it,
     * (b) split the open horizontal segment into two halves at that point, and (c) do likewise
     * for the open vertical segment.
     */
    // Prepare return value.
    Nodes crossingNodes;
    // For display/testing purposes, we want the crossing nodes we create to
    // look smaller than the average node in the graph.
    double crossingNodeSize = m_givenGraph->getIEL()/8.0;
    // Build all Events for all segments.
    Events evts;
    for (EdgeSegment *seg : m_edgeSegments) {
        pair<Event*, Event*> p = seg->getEvents();
        evts.push_back(p.first);
        evts.push_back(p.second);
    }
    // Partition by x-coord.
    // Experimentally determined partition tolerance:
    const double tol = 0.8;
    vector<Events> xparts = partition<Event*>(evts, [](Event *e)->double{return e->x();}, tol);
    // Set of Events representing open horizontal segments:
    std::set<Event*> openH;
    // Iterate over the x-parts.
    for (Events part : xparts) {
        // Event pointer representing open vertical segment:
        Event *openV = nullptr;
        // Vector of active Events:
        Events active;
        // All open horizontals, and all events in the current x-part, are active.
        active.resize(openH.size() + part.size());
        auto it = std::copy(openH.begin(), openH.end(), active.begin());
        std::copy(part.begin(), part.end(), it);
        active.shrink_to_fit();
        // Sort the active events.
        std::sort(active.begin(), active.end(), CompareActiveEvents);
        // Now we can iterate over the active events.
        for (Event *evt : active) {
            switch(evt->type) {
            case EventType::CLOSE:
                // What is closing? A horizontal or a vertical segment? Act accordingly.
                if (evt->seg->orientation == vpsc::HORIZONTAL) {
                    // We have hit the close event for a horizontal segment. We must remove the
                    // corresponding sustain event from the set.
                    openH.erase(evt->companion);
                } else {
                    // There is no longer an open vertical segment.
                    openV = nullptr;
                }
                break;
            case EventType::OPEN:
                // What is opening? A horizontal or a vertical segment? Act accordingly.
                if (evt->seg->orientation == vpsc::HORIZONTAL) {
                    // Turn the open event into a sustain event, and add it to the set.
                    evt->type = EventType::SUSTAIN;
                    openH.insert(evt);
                } else {
                    // Note the open vertical segment.
                    openV = evt;
                }
                break;
            case EventType::SUSTAIN:
                // This event indicates to us that a horizontal segment is open at this point.
                // Is there also an open vertical segment?
                if (openV != nullptr) {
                    // There is also an open vertical segment, so we have an intersection.
                    // First create a new node to represent the crossing.
                    Node_SP cr = Node::allocate();
                    cr->setCentre(openV->constCoord, evt->constCoord);
                    cr->setDims(crossingNodeSize, crossingNodeSize);
                    // Store it in the return vector.
                    crossingNodes.push_back(cr);
                    // Set it as the new closing node of the two open segments.
                    evt->seg->setNewClosingNode(cr);
                    openV->seg->setNewClosingNode(cr);
                    // Make a new horizontal segment to carry on for the open one, and store it globally.
                    EdgeSegment *hseg = new EdgeSegment(cr, evt->companion->endpt);
                    evt->companion->seg = hseg;
                    m_edgeSegments.push_back(hseg);
                    // Make a new vertical segment to carry on for the open one, and store it globally.
                    EdgeSegment *vseg = new EdgeSegment(cr, openV->companion->endpt);
                    openV->companion->seg = vseg;
                    m_edgeSegments.push_back(vseg);
                    // Update the two open events.
                    // By updating the event representing the open horizontal segment...
                    evt->seg = hseg;
                    evt->endpt = cr;
                    evt->varCoord = openV->constCoord;
                    // ...we can simply leave it in the openH set.
                    // Likewise, by updating the event representing the open vertical segment...
                    openV->seg = vseg;
                    openV->endpt = cr;
                    openV->varCoord = evt->constCoord;
                    // ...it continues to correctly represent the open vertical segment.
                }
                break;
            }
        }
    }
    // We're done using all the Events, so we can clean them up.
    for (Event *e : evts) delete e;
    // Return the vector of new crosing nodes.
    return crossingNodes;
}

void OrthoPlanariser::removeEdgeCrossings(void) {
    m_planarGraph = std::make_shared<Graph>();
    // Start the new graph with a ghost of each node in the overlap-free graph.
    for (auto pair : m_overlapFreeGraph->getNodeLookup()) {
        Node_SP &u = pair.second;
        GhostNode_SP g = GhostNode::allocate(*u);
        g->setMasquerade(true);
        m_planarGraph->addNode(g);
    }
    // Build segments based on the Edges in the overlap-free graph.
    buildSegments(m_overlapFreeGraph);
    // Compute the crossing nodes, adding new segments as needed.
    Nodes crossingNodes = computeCrossings();
    // Add the crossing nodes to the graph.
    for (Node_SP u : crossingNodes) {
        m_planarGraph->addNode(u);
    }
    // The graph gets one edge for each of the current segments.
    for (EdgeSegment *s : m_edgeSegments) {
        // Subtle point: Although we're going to use the EdgeSegments as a guide
        // to tell us what Edges to create, the Edges are not to link the Nodes
        // named in the EdgeSegments; they are to link those Nodes in the new,
        // planar Graph, that have the same IDs as those Nodes.
        Node_SP src = m_planarGraph->getNode(s->openingNode->id()),
                tgt = m_planarGraph->getNode(s->closingNode->id());
        m_planarGraph->addEdge(Edge::allocate(src, tgt));
    }
    // Add constraints, if requested.
    if (m_opts.generateConstraints) {
        SepMatrix &m = m_planarGraph->getSepMatrix();
        for (EdgeSegment *s : m_edgeSegments) {
            s->addSep(m);
        }
    }
}
