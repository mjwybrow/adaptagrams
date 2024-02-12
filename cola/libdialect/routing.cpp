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

#include <vector>
#include <utility>
#include <math.h>
#include <string>

#include "libvpsc/assertions.h"
#include "libavoid/libavoid.h"

#include "libdialect/util.h"
#include "libdialect/graphs.h"
#include "libdialect/ortho.h"
#include "libdialect/routing.h"

using namespace dialect;

using Avoid::Router;
using Avoid::Point;
using Avoid::Polygon;
using Avoid::PolyLine;
using Avoid::ConnRef;
using Avoid::ConnEnd;
using Avoid::ConnDirFlags;
using Avoid::ShapeRef;

using std::vector;

typedef std::pair<ConnEnd, ConnEnd> ConnEndPair;

void RoutingAdapter::addNodes(const NodesById &nodes) {
    for (auto p : nodes) {
        // Ask the node to compute its polygon.
        Polygon poly = p.second->makeLibavoidPolygon();
        // Allocate a ShapeRef and add it to the router.
        ShapeRef *sr = new ShapeRef(&router, poly);
        // Store in local lookup.
        nodeIdToShapeRef.insert({p.first, sr});
    }
}

void RoutingAdapter::addEdges(const EdgesById &edges, const EdgeConnDirsById *connDirs) {
    // We take advantage of the ordered nature of the maps.
    auto it = edges.begin();
    if (connDirs != nullptr) {
        auto jt = connDirs->begin();
        // Before falling off the end of either map...
        while (it != edges.end() && jt != connDirs->end()) {
            auto p = *it;
            auto q = *jt;
            // Record the Edge in the local lookup.
            this->edges.insert(p);
            // Get the two IDs.
            id_type i = p.first,
                    j = q.first;
            // Advance the connDirs iterator iff its ID lags the edges ID.
            if (i > j) ++jt;
            // In all other cases we are able to test whether the current Edge's ID
            // is in the connDirs map.
            else {
                // Allocate a ConnRef and add it to the router.
                ConnRef *cr = new ConnRef(&router);
                // Store it in the lookup map by Edge ID.
                edgeIdToConnRef.insert({i, cr});
                // Get the ConnEnds.
                ConnEndPair ends;
                if (i==j) {
                    // If the two IDs are equal then this Edge has special connDirs.
                    auto cd = q.second;
                    ends = p.second->makeLibavoidConnEnds(cd.first, cd.second);
                } else {
                    // Otherwise it does not.
                    ends = p.second->makeLibavoidConnEnds();
                }
                // Set the ends in the ConnRef.
                cr->setEndpoints(ends.first, ends.second);
                ++it;
            }
        }
    }
    // In case we fell off the end of the connDirs map first, then all remaining edges
    // take the default of ConnDirAll.
    while (it != edges.end()) {
        auto p = *it;
        // Record the Edge in the local lookup.
        this->edges.insert(p);
        // Allocate a ConnRef and add it to the router.
        ConnRef *cr = new ConnRef(&router);
        // Store it in the lookup map by Edge ID.
        edgeIdToConnRef.insert({p.first, cr});
        // Get the ConnEnds.
        ConnEndPair ends = p.second->makeLibavoidConnEnds();
        // Set the ends in the ConnRef.
        cr->setEndpoints(ends.first, ends.second);
        ++it;
    }
}

void RoutingAdapter::route(RouteProcessing processing) {
    router.processTransaction();
    if (processing >= RouteProcessing::RECORD) recordRoutes(processing == RouteProcessing::REFINE_AND_RECORD);
}

void RoutingAdapter::recordRoutes(bool refine) {
    // Our Edge lookup and ConnRef lookup maps should always have exacty the same (ordered) domain,
    // so we can iterate over them in tandem.
    auto it = edges.begin();
    auto jt = edgeIdToConnRef.begin();
    while (it != edges.end()) {
        auto p = *it;
        auto q = *jt;
        COLA_ASSERT(p.first == q.first);
        // Get the Points in the connector route.
        PolyLine poly = q.second->displayRoute();
        vector<Point> pts = poly.ps;
        if (refine) {
            // On some examples we were getting "S-bends" in which the middle segment
            // was about 10e-15 in length. We remove any such segments here.
            double epsilon = 0.001;
            // We will always keep the first and last route points.
            // As for the remaining, "interior" points of the route, we will scan
            // over them with a sliding window of length 2. Any time the two points
            // in the window are within epsilon of one another (in both dimensions),
            // we will skip both.
            vector<Point> refined_pts;
            size_t M = pts.size();
            // We expect that there are at least two route points.
            COLA_ASSERT( M >= 2);
            // the number of interior points:
            size_t N = M - 2;
            size_t i = 0;
            // We always keep the first route point.
            refined_pts.push_back(pts[0]);
            // Now the sliding window:
            while (i + 1 < N) {
                Point p = pts[i+1],
                      q = pts[i+2];
                double dx = q.x - p.x,
                       dy = q.y - p.y;
                if (fabs(dx) < epsilon && fabs(dy) < epsilon) {
                    // p and q are essentially coincident, so they add nothing to
                    // the route, and we can skip them both.
                    // Advance the sliding window by 2.
                    i += 2;
                } else {
                    // p and q are actually distinct points, so we keep the first,
                    // and advance the sliding window by 1.
                    refined_pts.push_back(p);
                    ++i;
                }
            }
            // When we exit the while loop, it could be that while there is no room left
            // for the entire sliding window of length 2, still there is one point left
            // to consider. We catch that case here:
            if (i < N) {
                refined_pts.push_back(pts[i+1]);
            }
            // And finally, we always keep the last route point.
            refined_pts.push_back(pts[N+1]);
            // Overwrite original pts vector.
            pts = refined_pts;
        }
        // Record the route.
        p.second->setRoute(pts);
        // Advance both iterators.
        ++it;
        ++jt;
    }
}


LeaflessOrthoRouter::LeaflessOrthoRouter(Graph_SP G, const HolaOpts &opts)
    : m_graph(G),
      m_n(G->getNumNodes()),
      m_ra(Avoid::OrthogonalRouting),
      m_iel(G->getIEL())
{
    // Set up the router.
    m_ra.router.setRoutingOption(Avoid::nudgeSharedPathsWithCommonEndPoint, false);
    m_ra.router.setRoutingParameter(Avoid::crossingPenalty, opts.routingScalar_crossingPenalty*m_iel);
    m_ra.router.setRoutingParameter(Avoid::segmentPenalty, opts.routingScalar_segmentPenalty*m_iel);
    m_ra.addNodes(G->getNodeLookup());
    EdgesById edges = G->getEdgeLookup();
    m_ra.addEdges(edges);
    // Record that all connection directions are initially allowed for both ends of each Edge.
    for (auto p : edges) {
        m_allowedConnDirs[p.first][p.second->getSourceEnd()->id()] = Avoid::ConnDirAll;
        m_allowedConnDirs[p.first][p.second->getTargetEnd()->id()] = Avoid::ConnDirAll;
    }
}

void LeaflessOrthoRouter::setShapeBufferDistanceIELScalar(double a) {
    m_ra.router.setRoutingParameter(Avoid::shapeBufferDistance, a*m_iel);
}

CardinalDir LeaflessOrthoRouter::departureDir(const Edge_SP &e, const Node_SP &u) const {
    ConnRef *cr = m_ra.edgeIdToConnRef.at(e->id());
    PolyLine poly = cr->displayRoute();
    vector<Point> pts = poly.ps;
    Point p0, p1;
    if (u->id() == e->getSourceEnd()->id()) {
        // Node u is the source end of Edge e.
        // So we want the first two points in the route, in order.
        p0 = pts[0];
        p1 = pts[1];
    } else {
        // Node u is the target end of Edge e.
        // So we want the last two points in the route, in reverse order.
        size_t L = pts.size();
        p0 = pts[L-1];
        p1 = pts[L-2];
    }
    return Compass::cardinalDirection(p0, p1);
}

void LeaflessOrthoRouter::route(Logger *logger) {

    // Set up for logging.
    unsigned ln = logger != nullptr ? logger->nextLoggingIndex : 0;
    std::function<void(unsigned)> log = [ln, this, logger](unsigned n)->void{
        if (logger!=nullptr) {
            std::string fn = string_format("%02d_%02d_routing_attempt", ln, n);
            std::string path = logger->writeFullPathForFilename(fn);
            this->m_ra.router.outputInstanceToSVG(path);
        }
    };

    /*
     * We may need to route multiple times to ensure that at least two sides of each node are being used,
     * but in theory we should never have to route more than 4n+1 times.
     *
     *  Proof: We always begin with an initial routing. We want to show it could be necessary to re-route
     *  at most 4n times.
     *
     *  In order to see this, we first argue that the worst-case scenario for any single node is that it
     *  require four routings. Consider some node u all of whose edges have been routed to one side, s0. We
     *  pick some edge e0 incident to u, say that it may not connect to side s0, and re-route for the first time.
     *
     *  While unlikely, it could be that, for whatever reason, now all edges incident to node u are routed to some other side,
     *  s1. We then pick some edge e1 (could be the same or different from e0), forbid it from connecting to
     *  side s1, and re-route for a second time.
     *
     *  Again, for whatever reason, all edges could now connect to one
     *  of the two remaining sides, s2. Continuing in this way, we could be led to re-route a third and a fourth time. But
     *  prior to the fourth re-routing it would be the case that for each side si of node u, there was
     *  some edge ei incident to u that had been forbidden from connecting on side si. Therefore on the fourth
     *  re-routing it would be impossible for all edges to connect on any single side of u.
     *
     *  So far we have handled a single node. However, in again a highly unlikely worst-case scenario, it could be
     *  that during the first five routings no other node besides u was a pseudoleaf (had all edges routed to one side),
     *  but after the fifth some other node became a pseudoleaf. In this way we could be led to do four re-routings
     *  for each node in the graph. QED
     *
     * In practice, it would probably be very rare for more that two routings to ever be necessary. For this
     * requires the odd circumstance, considered in the proof, that forbidding one edge from connecting on a
     * given side somehow results in /all/ edges incident at that node migrating to some other, single side.
     */
    // As explained in the comments above, at most 4n+1 routings should ever be needed.
    // We let our for-loop go to a 4n+2nd iteration, so we can catch any error with an assertion.
    size_t maxRoutings = 4*m_n + 1;
    for (size_t numRoutings = 0; numRoutings <= maxRoutings; numRoutings++) {
        m_ra.router.processTransaction();
        log(numRoutings + 1);
        COLA_ASSERT(numRoutings < maxRoutings);
        // For testing purposes, we may want to record the results of
        // each routing attempt.
        if (recordEachAttempt) {
            m_ra.recordRoutes(true);
            routingAttemptTglf.push_back(m_graph->writeTglf());
        }
        // Are there any nodes having all of their edges routed
        // out of just one side? This is what we want to prevent.
        // Such nodes would become leaves in a planarisation, so we
        // call them "pseudoleaves".
        Nodes pseudoLeaves;
        // For each such Node (if any), there is a sole direction in which
        // all connectors depart. We keep track of those directions as we work.
        vector<CardinalDir> soleDepartureDirecs;
        // Check each Node in the Graph:
        for (auto p : m_graph->getNodeLookup()) {
            Node_SP &u = p.second;
            const EdgesById edgeLookup = u->getEdgeLookup();
            // Sanity check, that Node u is not an actual leaf:
            COLA_ASSERT(edgeLookup.size() > 1);
            // Determine the departure direction from Node u for its first Edge.
            auto edge_it = edgeLookup.cbegin();
            CardinalDir d0 = departureDir((*edge_it).second, u);
            // If two or more directions have been used, some edge must depart
            // in a different direction than this one.
            bool isPseudoLeaf = true;
            for (auto jt = ++edge_it; jt != edgeLookup.cend(); ++jt) {
                CardinalDir d1 = departureDir((*jt).second, u);
                if (d1 != d0) {
                    isPseudoLeaf = false;
                    break;
                }
            }
            if (isPseudoLeaf) {
                pseudoLeaves.push_back(u);
                soleDepartureDirecs.push_back(d0);
            }
        }
        // Are there any pseudoleaves?
        if (pseudoLeaves.empty()) {
            // If there are none, then we're done routing, and can break out of the outer loop.
            break;
        } else {
            for (size_t i = 0; i < pseudoLeaves.size(); ++i) {
                Node_SP u = pseudoLeaves[i];
                CardinalDir d0 = soleDepartureDirecs[i];
                // Now among all Edges incident at this Node we must select one that is still
                // allowed to depart in at least two directions (hence at least one different
                // from d0), and remove direction d0 from its list of allowed directions.
                //
                // Secondarily, we would like to choose an Edge e such that if v is the Node
                // at the other end, then the predominant cardinal direction from Node u to Node v
                // is different than d0. That is likely a suitable Edge to depart in a different
                // direction.
                Edge_SP candidate = nullptr;
                for (auto p : u->getEdgeLookup()) {
                    Edge_SP &e = p.second;
                    // If this Edge is only allowed the one direction, then skip it.
                    if (isSoleDirec(m_allowedConnDirs.at(e->id()).at(u->id()))) continue;
                    // Otherwise mark it as the candidate.
                    candidate = e;
                    // Determine the predominant cardinal direction from Node u to the Node v at
                    // the opposite end of Edge e.
                    Node_SP v = e->getOtherEnd(*u);
                    CardinalDir d1 = Compass::cardinalDirection(u, v);
                    // If this is different from direction d0, then we're happy to accept this candidate.
                    if (d1 != d0) break;
                }
                // Start with the directions allowed last time:
                ConnDirFlags available = m_allowedConnDirs.at(candidate->id()).at(u->id());
                // XOR with the connection flag corresponding to cardinal direction d0,
                // so that this direction is no longer allowed.
                available ^= Compass::libavoidConnDirs.at(d0);
                // Record the new value.
                m_allowedConnDirs[candidate->id()][u->id()] = available;
                // Set a new ConnEnd.
                Point p = u->getCentre();
                ConnEnd end(p, available);
                ConnRef *cr = m_ra.edgeIdToConnRef.at(candidate->id());
                if (u->id() == candidate->getSourceEnd()->id()) {
                    cr->setSourceEndpoint(end);
                } else {
                    cr->setDestEndpoint(end);
                }
            }
        }
    }
    // Finally, the routing is done and we can set the connector routes in the Edge objects.
    m_ra.recordRoutes(true);
}
