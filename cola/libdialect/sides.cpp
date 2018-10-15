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

#include <sstream>
#include <memory>
#include <algorithm>
#include <vector>
#include <stdexcept>
#include <cmath>

#include "libvpsc/assertions.h"
#include "libavoid/geomtypes.h"

#include "libdialect/ortho.h"
#include "libdialect/graphs.h"
#include "libdialect/faces.h"
#include "libdialect/treeplacement.h"
#include "libdialect/constraints.h"

using namespace dialect;

using std::vector;

using Avoid::Point;

Side::Side(Nodes nodeSeq, CardinalDir direc)
    : m_nodeSeq(nodeSeq),
      m_forward(direc),
      m_inward(Compass::cardRotateCw90(direc)),
      m_vardim(Compass::varDim.at(direc)),
      m_constdim(Compass::constDim.at(direc)){}

bool Side::containsNode(id_type id) const {
    for (Node_SP v : m_nodeSeq) {
        if (v->id() == id) return true;
    }
    return false;
}

size_t Side::findNodeIndex(id_type id) const {
    for (size_t i = 0; i < m_nodeSeq.size(); ++i) {
        Node_SP v = m_nodeSeq[i];
        if (v->id() == id) return i;
    }
    return -1;
}

CardinalDir Side::getForwardDirec(void) const {
    return m_forward;
}

Node_SP Side::firstNode(void) const {
    if (m_nodeSeq.empty()) return nullptr;
    return m_nodeSeq.front();
}

Node_SP Side::lastNode(void) const {
    if (m_nodeSeq.empty()) return nullptr;
    return m_nodeSeq.back();
}

std::string Side::toString(void) const {
    std::ostringstream ss;
    ss << "Side: " << Compass::cardToString(m_forward) << ":";
    for (Node_SP u : getNodeSeq()) ss << " " << u->id();
    return ss.str();
}

size_t Side::getNumRootNodes(void) const {
    size_t N = 0;
    for (Node_SP u : m_nodeSeq) if (u->isRoot()) ++N;
    return N;
}

void Side::addTreePlacement(TreePlacement_SP tp) {
    // Sanity check: must not add a TreePlacement whose root Node does not
    // belong to this Side.
    COLA_ASSERT(containsNode(tp->getRootNode()->id()));
    // Record the placement.
    m_treePlacements.insert(tp);
}

ProjSeq_SP Side::computeCollateralProjSeq(TreePlacement_SP tp, double padding) {
    // Sanity check: TreePlacement's root Node must belong to this Side.
    COLA_ASSERT(containsNode(tp->getRootNode()->id()));
    // Initialise return value.
    ProjSeq_SP ps = std::make_shared<ProjSeq>();
    // Get the tree box for the new placement, and the root node.
    Node_SP treeBox = tp->buildTreeBox(padding),
            rootNode = tp->getRootNode();
    // Get their dimensions and coordinates.
    dimensions tDims = treeBox->getDimensions(),
               rDims = rootNode->getDimensions();
    Point tPt = treeBox->getCentre(),
          rPt = rootNode->getCentre();
    double tw = tDims.first,
           th = tDims.second,
           tu = tPt.x,
           tv = tPt.y,
           rw = rDims.first,
           rh = rDims.second,
           rx = rPt.x,
           ry = rPt.y;
    // Compute a segment representing the extent of the new tree node in
    // the Side's inward direction.
    LineSegment *seg = nullptr;
    switch(m_inward) {
    case CardinalDir::NORTH:
        seg = new LineSegment(Point(rx, ry-rh/2.0), Point(rx, ry+tv-th/2.0));
        break;
    case CardinalDir::EAST:
        seg = new LineSegment(Point(rx+rw/2.0, ry), Point(rx+tu+tw/2.0, ry));
        break;
    case CardinalDir::SOUTH:
        seg = new LineSegment(Point(rx, ry+rh/2.0), Point(rx, ry+tv+th/2.0));
        break;
    case CardinalDir::WEST:
        seg = new LineSegment(Point(rx-rw/2.0, ry), Point(rx+tu-tw/2.0, ry));
        break;
    }
    // May need constraints only if the segment points inward w.r.t. this Side.
    if (seg->direc == m_inward) {
        SepCoSet sepcos;
        // Handle nodes on each side of the root node.
        size_t i0 = findNodeIndex(rootNode->id()),
               n = m_nodeSeq.size(),
               nb = i0,
               na = n - 1 - i0;
        // Perimeter nodes:
        Nodes before(nb), after(na);
        std::copy_n(m_nodeSeq.begin(), nb, before.begin());
        std::copy(m_nodeSeq.begin()+i0+1, m_nodeSeq.end(), after.begin());
        // Tree nodes:
        for (TreePlacement_SP tp1 : m_treePlacements) {
            Node_SP r = tp1->getRootNode();
            size_t i1 = findNodeIndex(r->id());
            // In practice we typically will be computing this projection sequence
            // before the new treeplacement has been made, so i1 probably will not
            // be equal to i0. However, rather than demand this condition, we simply
            // skip that case if it arises.
            if (i1 == i0) continue;
            // Otherwise proceed.
            // The other tree placement should already have its box node.
            Node_SP b1 = tp1->getBoxNode();
            COLA_ASSERT(b1 != nullptr);
            // Add the box node to the appropriate list, before or after.
            if (i1 < i0) before.push_back(b1);
            else after.push_back(b1);
        }
        // Filter out any nodes that do not lie opposite the segment computed above.
        auto it = std::remove_if(before.begin(), before.end(), [seg](const Node_SP &u)->bool{
            return !u->liesOppositeSegment(*seg, true);
        });
        before.erase(it, before.end());
        it = std::remove_if(after.begin(), after.end(), [seg](const Node_SP &u)->bool{
            return !u->liesOppositeSegment(*seg, true);
        });
        after.erase(it, after.end());
        // Now we must process the "before" nodes and the "after" nodes.
        // We prepare data to manage these two jobs with a for-loop.
        vector<int> signs;
        if (Compass::isIncreasingCard(m_forward)) signs = {-1, 1}; else signs = {1, -1};
        vector<Nodes> nodeLists = {before, after};
        bool isHoriz = Compass::isHorizontalCard(m_forward);
        Node_SP left = nullptr,
                right = nullptr;
        double gap;
        for (size_t j = 0; j < 2; ++j) {
            Nodes nodes = nodeLists[j];
            int sign = signs[j];
            // For each node in the list, set up a constraint with the root node.
            for (Node_SP u : nodes) {
                left = sign == 1 ? rootNode : u;
                right = sign == 1 ? u : rootNode;
                // Compute gap.
                dimensions ud = u->getDimensions();
                double w = ud.first,
                       h = ud.second;
                gap = isHoriz ? w/2.0 + sign*tu + tw/2.0 : h/2.0 + sign*tv + th/2.0;
                // Add a SepCo.
                sepcos.insert(std::make_shared<SepCo>(m_vardim, left, right, gap));
            }
        }
        // If we generated any separation constraints, add them to the projection sequence.
        if (!sepcos.empty()) ps->addProjection(sepcos, m_vardim);
    }
    // Clean up.
    delete seg;
    // Return the projection sequence.
    return ps;
}

interval Side::closedInterval(void) const {
    BoundingBox b;
    // Among the proper nodes belonging to this Side, we need only consider the first and last.
    b += m_nodeSeq.front()->getBoundingBox();
    b += m_nodeSeq.back()->getBoundingBox();
    // We also must consider any attached treeboxes.
    for (TreePlacement_SP tp : m_treePlacements) b += tp->getBoxNode()->getBoundingBox();
    return b.getInterval(m_vardim);
}

interval Side::getIntervalOppositeSegment(LineSegment &seg, bool openInterval) const {
    // If we do not run parallel, return empty interval.
    if (Compass::arePerpendicular(m_forward, seg.direc)) return {1, 0};
    // Otherwise compute and return intersection.
    interval I = closedInterval();
    double a = I.first,
           b = I.second;
    return openInterval ? seg.openIntervalIntersection(a, b) : seg.closedIntervalIntersection(a, b);
}

bool Side::liesOppositeSegment(LineSegment &seg, bool openInterval) const {
    // Compute the interval of intersection of this Side with the given segment.
    interval I = getIntervalOppositeSegment(seg, openInterval);
    // And say whether it's nonempty.
    return I.first <= I.second;
}

double Side::getCentreCoord(void) const {
    // Since we assume all Nodes on the Side are centre-aligned, we can check any one of them.
    Node_SP u = m_nodeSeq.front();
    Point c = u->getCentre();
    return m_constdim == vpsc::XDIM ? c.x : c.y;
}

Avoid::Point Side::getFirstPtOppositeSegment(LineSegment &seg) const {
    // Compute the interval of intersection of this Side with the given segment.
    interval I = getIntervalOppositeSegment(seg);
    // If it's empty, that's an error.
    if (I.first > I.second) throw std::runtime_error("Empty intersection between Side and LineSegment");
    // Otherwise determine the first point on the interval.
    double w = I.first,
           z = getCentreCoord();
    return m_constdim == vpsc::XDIM ? Point(z, w) : Point(w, z);
}

double Side::halfWidthOppositeSegment(LineSegment &seg) const {
    // Compute the interval of intersection of this Side with the given segment.
    interval I = getIntervalOppositeSegment(seg);
    // If it's empty, return -1.
    if (I.first > I.second) return -1;
    // Determine whether the segment lies on our low side or high side.
    double centreCoord = getCentreCoord();
    // NB: Since we ask the segment about the side (opposite of what we want to know),
    // we must negate the result!
    int side = -seg.coordOnWhichSide(centreCoord);
    // If in-line, we consider that we have no half-width opposite such a segment.
    if (side == 0) return -1;

    // Initialize the half-width to that of an edgenode in the underlying Graph.
    Graph *G = m_nodeSeq.front()->getGraph();
    // The Nodes in Sides always belong to some Graph. Check:
    COLA_ASSERT(G != nullptr);
    double halfWidth = G->getEdgeThickness()/2.0;

    // Now iterate over all Nodes, checking the relevant dimension of those
    // whose relevant interval intersects the given segment.
    // We consider both the proper Nodes belonging to this Side, and any attached treeboxes.

    // Prepare a Nodes vector.
    Nodes nodes(m_nodeSeq.size() + m_treePlacements.size());
    // Copy all the proper Nodes into it.
    auto it = std::copy(m_nodeSeq.begin(), m_nodeSeq.end(), nodes.begin());
    // Copy all the treeboxes into it.
    std::transform(m_treePlacements.begin(), m_treePlacements.end(), it, [](const TreePlacement_SP &tp)->Node_SP{
        return tp->getBoxNode();
    });

    double a = I.first,
           b = I.second;
    bool sideIsHoriz = Compass::isHorizontalCard(m_forward);
    for (Node_SP u : nodes) {
        BoundingBox box = u->getBoundingBox();
        double c, d, e, hw;
        if (sideIsHoriz) {
            c = box.x;
            d = box.X;
            e = side == -1 ? box.y : box.Y;
            hw = fabs(e - centreCoord);
        } else {
            c = box.y;
            d = box.Y;
            e = side == -1 ? box.x : box.X;
            hw = fabs(e - centreCoord);
        }
        if (b > c && d > a && hw > halfWidth) halfWidth = hw;
    }
    return halfWidth;
}
