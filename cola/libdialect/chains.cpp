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

#include <stdexcept>
#include <algorithm>
#include <limits>
#include <map>
#include <cmath>
#include <functional>
#include <math.h>
#include <string>
#include <memory>
#include <vector>
#include <deque>
#include <utility>

#include "libvpsc/assertions.h"
#include "libavoid/geomtypes.h"

#include "libdialect/commontypes.h"
#include "libdialect/graphs.h"
#include "libdialect/ortho.h"
#include "libdialect/constraints.h"
#include "libdialect/chains.h"

using namespace dialect;

using Avoid::Point;

using std::pair;
using std::deque;
using std::vector;
using std::map;
using std::atan2;
using std::sqrt;
using std::fabs;

const LinkShapes dialect::bentLinkShapeCw{
    LinkShape::TLC, LinkShape::TRC, LinkShape::BRC, LinkShape::BLC
};

LinkShapes dialect::bentLinkShapeCwFromStartingPt(LinkShape start) {
    size_t i0 = Chain::npos;
    for (size_t i = 0; i < 4; ++i) if (bentLinkShapeCw[i] == start) { i0 = i; break; }
    // Make sure a straight LinkShape was not passed.
    COLA_ASSERT(i0 != Chain::npos);
    LinkShapes ls;
    ls.resize(4);
    for (size_t i = 0; i < 4; ++i) {
        ls[i] = bentLinkShapeCw[(i0+i)%4];
    }
    return ls;
}

const map<LinkShape, map<CardinalDir, CardinalDir>> dialect::applyBendToDir{
    {LinkShape::TLC, { {CardinalDir::NORTH, CardinalDir::EAST}, {CardinalDir::WEST, CardinalDir::SOUTH} }},
    {LinkShape::TRC, { {CardinalDir::NORTH, CardinalDir::WEST}, {CardinalDir::EAST, CardinalDir::SOUTH} }},
    {LinkShape::BRC, { {CardinalDir::SOUTH, CardinalDir::WEST}, {CardinalDir::EAST, CardinalDir::NORTH} }},
    {LinkShape::BLC, { {CardinalDir::SOUTH, CardinalDir::EAST}, {CardinalDir::WEST, CardinalDir::NORTH} }}
};

const map<LinkShape, CardinalDir> dialect::cwIncomingDirForBend{
    {LinkShape::TLC, CardinalDir::NORTH},
    {LinkShape::TRC, CardinalDir::EAST},
    {LinkShape::BRC, CardinalDir::SOUTH},
    {LinkShape::BLC, CardinalDir::WEST}
};

std::vector<std::vector<LinkShape>> dialect::lookupMinimalBendSeqs(Node_SP A, CardinalDir d0, Node_SP Z, CardinalDir d1) {
    // The case in which nodes Z and A are the same is a legitimate one. For example, in a figure-8 network, we have
    // two chains, each of which is not a cycle but does have the same node as both its left and right anchor.
    CompassDir c;
    if (Z == A) {
        // In this case it turns out the minimal bend sequences are exactly the same as when A differs from Z and lies
        // in the direction d1 from it. So we can simply set c equal to d1 in this case.
        c = (CompassDir)d1;
    } else {
        c = Compass::compassDirection(Z, A);
    }
    return minimalBendSeqs.at(c).at(d0).at(d1);
}

CardinalDirs dialect::possibleCardinalDirections(Node_SP node1, Node_SP node2) {
    Point c1 = node1->getCentre(),
          c2 = node2->getCentre();
    double dx = c2.x - c1.x,
           dy = c2.y - c1.y;
    CardinalDirs dirs;
    if (dx > 0) dirs.push_back(CardinalDir::EAST);
    if (dx < 0) dirs.push_back(CardinalDir::WEST);
    if (dy > 0) dirs.push_back(CardinalDir::SOUTH);
    if (dy < 0) dirs.push_back(CardinalDir::NORTH);
    return dirs;
}

LinkShape dialect::shapeOfLink(Node_SP link) {
    const EdgesById &E = link->getEdgeLookup();
    if (E.size() != 2) throw std::runtime_error("Cannot compute LinkShape for non-link node.");
    // The order of values in the LinkShape enum is designed so that this works:
    int s = 0;
    for (auto p : E) {
        Edge_SP e = p.second;
        Node_SP u = e->getOtherEnd(*link);
        CardinalDir d = Compass::cardinalDirection(link, u);
        int a = (int) d;
        if (a == 0) --a;
        s += a;
    }
    return (LinkShape) s;
}

const size_t Chain::npos = -1;

Chain::Chain(Graph_SP G, std::deque<Node_SP> nodes, bool isCycle)
    : m_graph(G),
      m_isCycle(isCycle)
{
    size_t n = nodes.size();
    m_n = n;
    // Cannot have an empty chain.
    if (n == 0) throw std::runtime_error("Cannot have an empty chain");
    // Copy the nodes into member storage.
    // The way we do this depends on whether it is a cycle.
    bool mustReverse = false;
    if (m_isCycle) {
        COLA_ASSERT(n >= 3);
        // For cycles, we always store the nodes in clockwise order.
        // Start by getting the index of a node of minimal y-coord, plus the indices of its neighbours.
        double minY = std::numeric_limits<double>::max();
        size_t i1 = n;
        for (size_t i = 0; i < n; ++i) {
            Node_SP u = nodes[i];
            Point c = u->getCentre();
            if (c.y < minY) {
                minY = c.y;
                i1 = i;
            }
        }
        size_t i0 = (i1 + n - 1) % n,
               i2 = (i1 + 1) % n;
        // Get the Nodes at these indices, and their centre points.
        Node_SP u0 = nodes[i0],
                u1 = nodes[i1],
                u2 = nodes[i2];
        Point c0 = u0->getCentre(),
              c1 = u1->getCentre(),
              c2 = u2->getCentre();
        // If node u0 lies to the right of node u1, then the nodes are given in anticlockwise
        // order, and must be reversed.
        if (c0.x > c1.x) mustReverse = true;
        // Otherwise consider the case in which nodes u0 and u1 are vertically aligned.
        else if (c0.x == c1.x) {
            // Implied by the assumption that the cycle even /has/ an interior to speak of,
            // is that the cycle is not self-intersecting. Therefore, since both neighbouring
            // nodes u0 and u2 have y-coord >= that of u1, they cannot both have the same
            // x-coord as u1. Therefore...
            COLA_ASSERT(c2.x != c1.x);
            // And in this case if node u2 lies to the left of node u1, then we need to reverse.
            if (c2.x < c1.x) mustReverse = true;
        }
    }
    m_nodes.resize(n);
    if (mustReverse) {
        std::reverse_copy(nodes.begin(), nodes.end(), m_nodes.begin());
    } else {
        std::copy(nodes.begin(), nodes.end(), m_nodes.begin());
    }
    // Compute and store the shape of each link.
    for (Node_SP u : m_nodes) m_linkShapes.push_back(shapeOfLink(u));
    // Determine the sequence of internal edges, as well as the anchor nodes and edges if
    // it is not a cycle, or the "return edge" if it is.
    // Get the first node.
    Node_SP u0 = m_nodes.front();
    // Get its two incident edges.
    Edges E;
    for (auto p : u0->getEdgeLookup()) E.push_back(p.second);
    Edge_SP e1 = E[0],
            e2 = E[1];
    // Get the nodes at the other ends of those edges.
    Node_SP u1 = e1->getOtherEnd(*u0),
            u2 = e2->getOtherEnd(*u0);
    // In order to compute the sequence of internal edges, we need to identify the edge e0 that
    // leads into node u0 from the left.
    Edge_SP e0 = nullptr;
    if (n==1) {
        // In this case 'left' and 'right' are meaningless, so record the nodes and edges in any way.
        m_anchorNodeLeft = u1;
        m_anchorEdgeLeft = e1;
        m_anchorNodeRight = u2;
        m_anchorEdgeRight = e2;
    } else {
        // There are at least two nodes.
        // First determine the left anchor node and edge.
        if (u1 == m_nodes[1]) {
            m_anchorNodeLeft = u2;
            m_anchorEdgeLeft = e2;
            e0 = e1;
        } else {
            m_anchorNodeLeft = u1;
            m_anchorEdgeLeft = e1;
            e0 = e2;
        }
        // Now we can compute the sequence of edges.
        for (auto it = m_nodes.begin() + 1; it != m_nodes.end(); ++it) {
            // Record edge.
            m_edges.push_back(e0);
            // Get next edge.
            u0 = *it;
            for (auto p : u0->getEdgeLookup()) {
                if (p.second != e0) {
                    e0 = p.second;
                    break;
                }
            }
        }
        // And determine the right anchor node and edge.
        m_anchorEdgeRight = e0;
        m_anchorNodeRight = e0->getOtherEnd(*u0);
        // And finally, if it's a cycle, the "anchors" are meaningless (but harmless), and the
        // "return edge" is equal to either of the anchor edges.
        if (m_isCycle) m_returnEdge = m_anchorEdgeRight;
    }

}

Node_SP Chain::getNode(int i) const {
    if (i == -2) {
        return m_anchorNodeLeft;
    }
    size_t i_pos = (size_t) i;
    COLA_ASSERT(i_pos%2 == 0 && i_pos <= 2*m_n);
    if (i_pos == 2*m_n) {
        return m_anchorNodeRight;
    } else {
        return m_nodes[(size_t)(i_pos/2)];
    }
}

Edge_SP Chain::getEdge(int i) const {
    if (i == -1) {
        return m_anchorEdgeLeft;
    }
    size_t i_pos = (size_t) i;
    COLA_ASSERT(i_pos%2 == 1 && i_pos <= 2*m_n - 1);
    if (i_pos == 2*m_n - 1) {
        return m_anchorEdgeRight;
    } else {
        return m_edges[(size_t)((i_pos-1)/2)];
    }
}

double Chain::bendCost(LinkShape bendType, size_t i0) const {
    // First compute the angle alpha for position i0.
    // This is the atan2 for a vector z from point p to point q, where
    //   if i0 is an edge then p is centre of node i0 - 1 and q centre of node i0 + 1;
    //   if i0 is a node then p and q are points on edges i0 - 1 and i0 + 1 resp,
    //     each a unit distance from centre of node i0.
    Point p, q;
    if (i0 % 2 == 1) {
        Node_SP u = getNode(i0 - 1),
                w = getNode(i0 + 1);
        p = u->getCentre();
        q = w->getCentre();
    } else {
        Node_SP u = getNode(i0 - 2),
                v = getNode(i0),
                w = getNode(i0 + 2);
        Point r = u->getCentre(),
              s = v->getCentre(),
              t = w->getCentre();
        double px = r.x - s.x,
               py = r.y - s.y,
               qx = t.x - s.x,
               qy = t.y - s.y,
               lp = sqrt(px*px + py*py),
               lq = sqrt(qx*qx + qy*qy);
        p.x = px/lp;
        p.y = py/lp;
        q.x = qx/lq;
        q.y = qy/lq;
    }
    double dx = q.x - p.x,
           dy = q.y - p.y;
    // Get angle in degrees.
    double alpha0 = atan2(dy, dx) * 180 / 3.141592653589793;
    double cost;
    // Want a little helper function for checking angle ranges.
    std::function<void(double, double)> check = [](double a, double L)->void{
        COLA_ASSERT(-L < a && a <= L);
    };
    if (m_isCycle) {
        // For a cycle each type of bend has a specific angle associated with it,
        // so you can be up to +/-180 degrees off.
        double beta;
        switch(bendType) {
        case LinkShape::TLC: beta = -45; break;
        case LinkShape::BLC: beta = -135; break;
        case LinkShape::TRC: beta = 45; break;
        case LinkShape::BRC: beta = 135; break;
        default:
            throw std::runtime_error("No bend cost for unbent link shape.");
        }
        double alpha1 = alpha0 - beta;
        check(alpha1, 360);
        if (alpha1 <= -180) alpha1 += 360;
        else if (alpha1 > 180) alpha1 -= 360;
        check(alpha1, 180);
        // Normalise the cost.
        cost = fabs(alpha1/180.0);
    } else {
        // For a non-cycle we don't distinguish between TLC and BRC, or between TRC and BLC
        // bends, so you can only be up to +/- 90 degrees off.
        check(alpha0, 180);
        if (alpha0 <= -90) alpha0 += 180;
        else if (alpha0 > 90) alpha0 -= 180;
        check(alpha0, 90);
        double beta;
        switch(bendType) {
        case LinkShape::TLC: beta = -45; break;
        case LinkShape::BLC: beta = 45; break;
        case LinkShape::TRC: beta = 45; break;
        case LinkShape::BRC: beta = -45; break;
        default:
            throw std::runtime_error("No bend cost for unbent link shape.");
        }
        double alpha1 = alpha0 - beta;
        check(alpha1, 135);
        if (alpha1 <= -90) alpha1 += 180;
        else if (alpha1 > 90) alpha1 -= 180;
        check(alpha1, 90);
        // Normalise the cost.
        cost = fabs(alpha1/90.0);
    }
    return cost;
}

double Chain::nextLocalOptimalPoint(size_t i0, LinkShape bendType, size_t remaining, size_t &i1) const {
    size_t candidate = Chain::npos;
    double bestCost = 2;  // effectively infinity since costs are at most 1
    i1 = i0;
    size_t M = getNumPotentialBendPlaces();
    M -= remaining;
    double cost = bestCost;
    bool madeChoice = false;
    for (size_t i = i0; i < M; ++i) {
        cost = bendCost(bendType, i);
        // If we already have a candidate and it has lower cost than the current position,
        // then accept the candidate. This is how our simple, greedy, local optimisation works.
        if (candidate != Chain::npos && cost > bestCost) {
            i1 = candidate;
            cost = bestCost;
            madeChoice = true;
            break;
        }
        // To even be considered a candidate for optimal position, the cost has to be less
        // than 0.5. Else we might start at bad and go to worse, and thereby accept bad.
        if (cost < 0.5 && cost < bestCost) {
            candidate = i;
            bestCost = cost;
        }
    }
    if (!madeChoice && candidate != Chain::npos) {
        i1 = candidate;
        cost = bestCost;
    }
    return cost;
}

double Chain::globalOptimalPoint(LinkShape bendType, size_t &i1, size_t beginAt) const {
    i1 = Chain::npos;
    double cost = 2;  // effectively infinity since costs are at most 1
    size_t M = getNumPotentialBendPlaces();
    for (size_t i = beginAt; i < M; ++i) {
        double c = bendCost(bendType, i);
        if (c < cost) {
            i1 = i;
            cost = c;
        }
    }
    return cost;
}

BendSequences Chain::computePossibleBendSequences(void) const {
    // Prepare return value.
    BendSequences seqs;
    // We cannot accept any bend sequence that is longer than our number of nodes plus internal edges.
    size_t M = getNumPotentialBendPlaces();
    // We operate differently for cycles and non-cycles.
    if (m_isCycle) {
        // In the case of a cycle, there are always exactly four bends to be made, namely the four bent
        // LinkShapes, in clockwise order. However we could begin at any of the four positions in that
        // list, so these are all the possibilities.
        if (M >= 4) {
            seqs.reserve(4);
            for (LinkShape start : bentLinkShapeCw) {
                LinkShapes shapes = bentLinkShapeCwFromStartingPt(start);
                seqs.push_back(std::make_shared<BendSequence>(shapes));
            }
        }
    } else {
        // For a non-cycle, we begin by determining the incoming and outgoing directions.
        Node_SP A = m_anchorNodeLeft,
                Z = m_anchorNodeRight,
                b = m_nodes.front(),
                y = m_nodes.back();
        CardinalDirs inDirs, outDirs;
        // Incoming directions:
        try {
            CardinalDir dIn = m_graph->getSepMatrix().getCardinalDir(A->id(), b->id());
            inDirs.push_back(dIn);
        } catch (std::runtime_error const&) {
            // In this case the left anchor edge is not yet aligned, so we get to consider
            // both possible non-aggressive alignments of it.
            inDirs = possibleCardinalDirections(A, b);
        }
        // Same idea, this time for outgoing directions:
        try {
            CardinalDir dOut = m_graph->getSepMatrix().getCardinalDir(y->id(), Z->id());
            outDirs.push_back(dOut);
        } catch (std::runtime_error const&) {
            outDirs = possibleCardinalDirections(y, Z);
        }
        // Now we can look up the possible bend sequences.
        for (CardinalDir d0 : inDirs) {
            for (CardinalDir d1 : outDirs) {
                vector<LinkShapes> shapeSeqs = lookupMinimalBendSeqs(A, d0, Z, d1);
                for (LinkShapes shapes : shapeSeqs) {
                    if (shapes.size() <= M) {
                        seqs.push_back(std::make_shared<BendSequence>(shapes, d0, d1));
                    }
                }
            }
        }
    }
    return seqs;
}

void Chain::evaluateBendSequence(BendSequence_SP bendSeq) const {
    // Put the desired bend shapes in a queue.
    deque<LinkShape> queue(bendSeq->size());
    std::copy(bendSeq->bendTypes.begin(), bendSeq->bendTypes.end(), queue.begin());
    // Set up
    size_t i = 0;
    double &cost = bendSeq->cost;
    vector<size_t> &bendPoints = bendSeq->bendPoints;
    while (queue.size() >= 2) {
        // As long as there are at least two places left to choose, we must do a local
        // search.
        LinkShape ls = queue.front();
        queue.pop_front();
        cost += nextLocalOptimalPoint(i, ls, queue.size(), i);
        bendPoints.push_back(i++);
    }
    if (queue.size() == 1) {
        // If there is precisely one bend place left to choose, then we can do a global
        // search over the remaining places.
        LinkShape ls = queue.front();
        queue.pop_front();
        cost += globalOptimalPoint(ls, i, i);
        bendPoints.push_back(i);
    }
}

ChainConfigSeq Chain::writeConfigSeq(BendSequence_SP bendSeq) const {
    ChainConfigSeq config;
    size_t m = m_edges.size(),
           numBends = bendSeq->size();
    vector<size_t> &bendPoints = bendSeq->bendPoints;
    LinkShapes &bendTypes = bendSeq->bendTypes;
    CardinalDir direc;
    if (m_isCycle) {
        ++m;
        // Since we always run cycles clockwise, we can infer from the first bendtype
        // what the incoming direction must be.
        direc = cwIncomingDirForBend.at(bendSeq->bendTypes[0]);
    } else {
        // Not a cycle. In this case we will have determined the incoming direction of the bend sequence
        // at construction time.
        direc = bendSeq->incomingDirec;
    }
    size_t ptr = 0;
    for (size_t j = 0; j < m; ++j) {
        if (ptr == numBends) {
            // All remaining edges get the current direction.
            config.push_back({direc, direc});
        } else {
            size_t k = 2*j + 1;
            vector<pair<size_t, LinkShape>> bs;
            while (ptr < numBends && (bendPoints[ptr] == k || bendPoints[ptr] == k - 1)) {
                bs.push_back({bendPoints[ptr], bendTypes[ptr]});
                ++ptr;
            }
            // At this point, k is an odd number, referring to an edge in the chain,
            // direc is the incoming direction into node k - 1, and bs is a list of
            // bend points of length 0, 1, or 2, occurring at node k - 1 and/or edge k.
            // Our job is to: (a) describe what happens at edge k, namely either that
            // it be configured to run in a certain compass direction, or that it contain
            // a bend point, with certain incoming and outgoing compass directions;
            // and (b) to set direc equal to the (outgoing) direction of the edge that
            // leads into node k + 1.
            size_t n = bs.size();
            if (n == 2) {
                LinkShape bt0 = bs[0].second,
                          bt1 = bs[1].second;
                CardinalDir dir0 = applyBendToDir.at(bt0).at(direc),
                            dir1 = applyBendToDir.at(bt1).at(dir0);
                config.push_back({dir0, dir1});
                direc = dir1;
            } else if (n == 1) {
                size_t bp = bs[0].first;
                LinkShape bt = bs[0].second;
                CardinalDir nextDir = applyBendToDir.at(bt).at(direc);
                if (bp == k) {
                    // Next bend should occur at this edge.
                    config.push_back({direc, nextDir});
                } else if (bp == k - 1) {
                    // Next bend should occur at the node before this edge.
                    config.push_back({nextDir, nextDir});
                }
                direc = nextDir;
            } else {
                COLA_ASSERT(n == 0);
                // Carry on with current direction. In particular, this case handles what
                // happens if the final bend is to occur at the final node. For in that
                // case all we can do is carry on with the current direction, and it is
                // up to the anchorEdgeRight to make the final bend happen.
                config.push_back({direc, direc});
            }
        }
    }
    return config;
}

void Chain::takeShapeBasedConfiguration(void) {
    // Compute the vector of all possible bend sequences.
    BendSequences seqs = computePossibleBendSequences();
    // If there are no possible bend sequences, there is nothing for us to do.
    if (seqs.empty()) return;
    // Otherwise proceed to evaluate the possible sequences, and choose one of least cost.
    for (auto bs : seqs) evaluateBendSequence(bs);
    BendSequence_SP bestSeq = nullptr;
    double leastCost = std::numeric_limits<double>::max();
    for (auto bs : seqs) if (bs->cost < leastCost) { bestSeq = bs; leastCost = bs->cost; }
    COLA_ASSERT(bestSeq != nullptr);
    // Get the configuration sequence for the least-cost sequence.
    ChainConfigSeq configSeq = writeConfigSeq(bestSeq);
    // Set the configuration in the underlying graph.
    size_t j = 0;
    SepMatrix &matrix = m_graph->getSepMatrix();
    double bpSize = m_graph->getIEL()/8.0;
    for (auto p : configSeq) {
        // Get the edge to be configured, and the nodes u, v that come before and after it in the chain, resp.
        size_t k = 2*j + 1;
        Edge_SP edge = getEdge(k);
        Node_SP u = getNode(k - 1),
                v = getNode(k + 1);
        id_type uid = u->id(),
                vid = v->id();
        if (p.first == p.second) {
            // In this case the edge is to be aligned in a compass direction.
            matrix.setCardinalOP(uid, vid, p.first);
        } else {
            // In this case we are to build a new bend point.
            // First sever the edge and free and constraint on the nodes.
            m_graph->severEdge(*edge);
            matrix.free(uid, vid);
            // Build a bend point, giving it a reasonable initial location midway
            // between u and v. It is not yet aligned with either of them; that will
            // wait until we project onto the new constraints.
            Point uc = u->getCentre(),
                  vc = v->getCentre();
            double x = (uc.x + vc.x)/2.0,
                   y = (uc.y + vc.y)/2.0;
            Node_SP bp = Node::allocate();
            bp->setCentre(x, y);
            bp->setDims(bpSize, bpSize);
            m_graph->addNode(bp);
            // Set constraints.
            id_type bpid = bp->id();
            matrix.setCardinalOP(uid, bpid, p.first);
            matrix.setCardinalOP(bpid, vid, p.second);
            // Add edges.
            Edge_SP e1 = Edge::allocate(u, bp),
                    e2 = Edge::allocate(bp, v);
            m_graph->addEdge(e1);
            m_graph->addEdge(e2);
            // Save a record.
            m_aestheticBends.push_back(std::make_shared<AestheticBend>(edge, bp, u, v));
        }
        // Next j.
        ++j;
    }
}

void Chain::addAestheticBendsToEdges(void) {
    for (AestheticBend_SP ab : m_aestheticBends) ab->addBendToEdge();
}

void AestheticBend::addBendToEdge(void) {
    edge->addBendNode(bendNode);
}

Chains dialect::buildAllChainsInGraph(std::shared_ptr<Graph> graph) {
    vector<deque<Node_SP>> chains, cycles;
    graph->getChainsAndCycles(chains, cycles);
    Chains allChains;
    for (auto d : chains) allChains.push_back(std::make_shared<Chain>(graph, d));
    for (auto d : cycles) allChains.push_back(std::make_shared<Chain>(graph, d));
    return allChains;
}
