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

#include <memory>
#include <algorithm>
#include <map>
#include <limits>
#include <sstream>
#include <functional>

#include "libvpsc/assertions.h"
#include "libvpsc/rectangle.h"
#include "libavoid/geomtypes.h"

#include "libdialect/commontypes.h"
#include "libdialect/ortho.h"
#include "libdialect/graphs.h"
#include "libdialect/faces.h"
#include "libdialect/expansion.h"
#include "libdialect/constraints.h"
#include "libdialect/logging.h"
#include "libdialect/util.h"
#include "libdialect/treeplacement.h"

using namespace dialect;

using Avoid::Point;

using std::string;

id_type TreePlacement::nextID = 0;

FaceSet_SP dialect::reattachTrees(Graph_SP core, Trees trees, HolaOpts opts, Logger *logger) {

    // Set up for logging.
    unsigned ln = logger != nullptr ? logger->nextLoggingIndex : 0;
    unsigned lns = 0;
    std::function<void(string)> log = [core, logger](string name)->void{
        if (logger!=nullptr) logger->log(*core, name);
    };

    // Compute the faces of the core.
    FaceSet_SP faceset = std::make_shared<FaceSet>(core);

    // Sort the Trees into the order in which we want to reattach them.
    // Our plan is to place those with largest perimeter first.
    std::sort(trees.begin(), trees.end(), [](const Tree_SP &a, const Tree_SP &b)->bool{
        return a->underlyingGraph()->getBoundingBox().perimeter() > b->underlyingGraph()->getBoundingBox().perimeter();
    });

    // Reattach the trees one by one, with padding.
    double padding = core->getIEL()/4.0;

    ColaOptions colaOpts;
    colaOpts.preventOverlaps = true;
    colaOpts.solidifyAlignedEdges = true;

    for (Tree_SP tree : trees) {
        // List all possible placements for this Tree.
        TreePlacements tps = faceset->listAllPossibleTreePlacements(tree);
        // Choose a best one.
        TreePlacement_SP best = nullptr;
        ProjSeq_SP ps = nullptr;
        while (!tps.empty()) {
            best = chooseBestPlacement(tps, opts);
            // Project, making room for the tree node.
            ps = best->buildBestProjSeq(padding, opts.expansion_doCostlierDimensionFirst, opts.expansion_estimateMethod);
            if (ps == nullptr) {
                // Best placement had no feasible projection sequence.
                // Delete it from `tps`, and try next best.
                tps.erase(std::remove(tps.begin(), tps.end(), best), tps.end());
            } else {
                break;
            }
        }
        if (ps == nullptr) {
            // No placement had a feasible projection sequence.
            std::ostringstream ss;
            ss << "No feasible expansions for tree rooted at node " << tree->getRootNodeID();
            throw std::runtime_error(ss.str());
        }
        core->applyProjSeq(colaOpts, *ps);
        // Insert the tree node.
        best->insertTreeNode(padding);
        log(string_format("%02d_%02d_insert_tree_box", ln, lns++));
    }

    return faceset;
}

TreePlacement_SP dialect::chooseBestPlacement(TreePlacements tps, HolaOpts opts) {
    TreePlacement_SP bestPlacement = nullptr;

    // If favouring cardinal placement, we make this the primary consideration.
    if (opts.treePlacement_favourCardinal) {
        // Sort so that all cardinal placements come first.
        std::sort(tps.begin(), tps.end(), [](const TreePlacement_SP &a, const TreePlacement_SP &b)->bool{
            return Compass::isCardinal(a->getPlacementDir()) && !Compass::isCardinal(b->getPlacementDir());
        });
        // Determine how many placements are cardinal.
        size_t numCardinal = 0;
        for (TreePlacement_SP tp : tps) if (Compass::isCardinal(tp->getPlacementDir())) ++numCardinal;
        // If there is precisely one cardinal placement, then we choose it.
        if (numCardinal == 1) bestPlacement = tps.front();
        // If there are several cardinal placements, then we choose only from among them.
        else if (numCardinal > 1) tps.erase(tps.begin() + numCardinal, tps.end());
    }

    // If we haven't chosen a best placement yet, and we are favouring external placement, then
    // we consider that next.
    if (bestPlacement == nullptr && opts.treePlacement_favourExternal) {
        // Sort so that all external placements come first.
        std::sort(tps.begin(), tps.end(), [](const TreePlacement_SP &a, const TreePlacement_SP &b)->bool{
            return a->isExternal() && !b->isExternal();
        });
        // Determine how many placements are external.
        size_t numExternal = 0;
        for (TreePlacement_SP tp : tps) if (tp->isExternal()) ++numExternal;
        // If there is precisely one external placement, then we choose it.
        if (numExternal == 1) bestPlacement = tps.front();
        // If there are several external placements, then we choose only from among them.
        else if (numExternal > 1) tps.erase(tps.begin() + numExternal, tps.end());
    }

    // If we haven't chosen a best placement yet, and we are favouring isolation, then
    // we consider that next.
    if (bestPlacement == nullptr && opts.treePlacement_favourIsolation) {
        // Build a lookup of number of potential neighbours, by TreePlacement ID.
        // Also determine the minimal number.
        std::map<id_type, size_t> numPotNbrs;
        size_t minimalNum = std::numeric_limits<size_t>::max();
        for (TreePlacement_SP tp : tps) {
            size_t n = tp->getNumPotentialNbrs();
            numPotNbrs.insert({tp->id(), n});
            if (n < minimalNum) minimalNum = n;
        }
        // Sort by increasing number of potential neighbours.
        std::sort(tps.begin(), tps.end(), [&numPotNbrs](const TreePlacement_SP &a, const TreePlacement_SP &b)->bool{
            return numPotNbrs.at(a->id()) < numPotNbrs.at(b->id());
        });
        // Determine how many placements have the minimal number.
        size_t numMinimal = 0;
        for (TreePlacement_SP tp : tps) if (numPotNbrs.at(tp->id()) == minimalNum) ++numMinimal;
        // If there is precisely one placement with the minimal number of potential neighbours, then we choose it.
        if (numMinimal == 1) bestPlacement = tps.front();
        // If there are several placements with the minimal number, then we choose only from among them.
        else if (numMinimal > 1) tps.erase(tps.begin() + numMinimal, tps.end());
    }

    // If we still haven't chosen a best placement, then we finally come to the case in which we must
    // estimate the cost of each remaining potential placement. Then we choose a cheapest one.
    if (bestPlacement == nullptr) {
        double minCost = std::numeric_limits<double>::max();
        for (TreePlacement_SP tp : tps) {
            double cost = tp->estimateCost();
            if (cost < minCost) {
                minCost = cost;
                bestPlacement = tp;
            }
        }
    }

    // By this point, we should always have made a choice.
    COLA_ASSERT(bestPlacement != nullptr);
    // Return it.
    return bestPlacement;
}

std::string TreePlacement::toString(void) const {
    std::ostringstream ss;
    ss << "TreePlacement " << m_ID << ": ";
    ss << "Placed " << Compass::dirToString(m_placementDir);
    ss << ", Growing " << Compass::cardToString(m_growthDir);
    ss << (m_flip ? ", Flipped." : ".") << std::endl;
    ss << "    Rooted at Node " << m_faceRoot->id() << std::endl;
    ss << "    Into " << m_face.toString() << std::endl;
    return ss.str();
}

size_t TreePlacement::getNumPotentialNbrs(void) {
    // Get the relevant sides:
    Sides relevantSides = m_face.getRelevantSidesForPlacement(shared_from_this());
    // First compute the total number of root nodes present:
    size_t N = 0;
    for (Side_SP S : relevantSides) N += S->getNumRootNodes();
    // Since this TreePlacement's own root node has been counted once per side, we subtract that.
    N -= relevantSides.size();
    return N;
}

double TreePlacement::estimateCost(void) {
    ExpansionManager em(shared_from_this());
    m_cost = em.estimateCost();
    return m_cost;
}

Node_SP TreePlacement::buildTreeBox(double padding) const {
    Node_SP initialBox = m_tree->buildRootlessBox(m_growthDir);
    dimensions initialDims = initialBox->getDimensions();
    Point initialCentre = initialBox->getCentre();
    double w1 = initialDims.first,
           h1 = initialDims.second,
           u0 = initialCentre.x,
           v0 = initialCentre.y,
           u1 = 0,
           v1 = 0;
    if (m_placementDir == (CompassDir)m_growthDir) {
        // Placement direction is cardinal and equals the growth direction.
        // In this case we need to check whether the tree is to be flipped,
        // and if so alter the (u0, v0) vector accordingly.
        // Also add padding if requested.
        if (Compass::isVerticalCard(m_growthDir)) {
            // Vertical growth direction.
            u1 = m_flip ? -u0 : u0;
            v1 = v0;
            if (padding > 0) {
                w1 += 2*padding;
                h1 += padding;
                // Want half width padding on each side, but all height padding on outside.
                v1 += Compass::isIncreasingCard(m_growthDir) ? padding/2.0 : -padding/2.0;
            }
        } else {
            // Horizontal growth direction.
            u1 = u0;
            v1 = m_flip ? -v0 : v0;
            if (padding > 0) {
                w1 += padding;
                h1 += 2*padding;
                // Want half height padding on each side, but all width padding on outside.
                u1 += Compass::isIncreasingCard(m_growthDir) ? padding/2.0 : -padding/2.0;
            }
        }
    } else {
        // Placement direction is ordinal, and growth direction is one of its components.
        // In this case we throw away the given (u0, v0), and compute this vector based
        // solely on the dimensions of the tree and root, and on the placement direction.
        // It does not matter if the tree is to be flipped.
        // First consider padding.
        if (padding > 0) {
            if (Compass::isVerticalCard(m_growthDir)) {
                w1 += 2*padding;
                h1 += padding;
            } else {
                w1 += padding;
                h1 += 2*padding;
            }
        }
        // Now adjust coordinates.
        dimensions rootDims = m_faceRoot->getDimensions();
        Point sgn = Compass::vectorSigns(m_placementDir);
        double rootW = rootDims.first,
               rootH = rootDims.second;
        u1 = sgn.x * (rootW + w1)/2.0;
        v1 = sgn.y * (rootH + h1)/2.0;
    }
    // Build representative Node and return.
    Node_SP boxNode = Node::allocate();
    boxNode->setCentre(u1, v1);
    boxNode->setDims(w1, h1);
    return boxNode;
}

bool TreePlacement::somePointOppositeSegment(LineSegment &seg, Avoid::Point &pt, double padding, bool openInterval) {
    if (padding < 0) {
        padding = m_face.getGraph()->getIEL()/4.0;
    }
    Node_SP treeBox = buildTreeBox(padding);
    Point rc = m_faceRoot->getCentre(),
          tc = treeBox->getCentre();
    dimensions dims = treeBox->getDimensions();
    bool segIsHoriz = Compass::isHorizontalCard(seg.direc);
    // tc actually gives the vector from the centre of the root node to that of the tree box.
    // So to get the actual coordinates of the centre of the tree box we need to add rc and tc.
    double hw = dims.first/2.0,
           hh = dims.second/2.0,
           x = rc.x + tc.x - hw,
           X = rc.x + tc.x + hw,
           y = rc.y + tc.y - hh,
           Y = rc.y + tc.y + hh,
           a = segIsHoriz ? x : y,
           b = segIsHoriz ? X : Y;
    interval K = openInterval ? seg.openIntervalIntersection(a, b) : seg.closedIntervalIntersection(a, b);
    // If K is empty, we cannot find a point.
    if (K.second < K.first) return false;
    // Otherwise there is an intersection so we can build a point.
    double w = K.first;
    if (segIsHoriz) {
        pt.x = w;
        pt.y = rc.y + tc.y;
    } else {
        pt.x = rc.x + tc.x;
        pt.y = w;
    }
    return true;
}

void TreePlacement::applyGeometryToTree(void) {
    // Rotate and flip.
    m_tree->rotate(m_growthDir);
    if (m_flip) m_tree->flip();
    // How we translate depends on whether the placement direction is cardinal or not.
    Point rc = m_faceRoot->getCentre();
    if (Compass::isCardinal(m_placementDir)) {
        // When the placement direction is cardinal, we simply translate the tree so its own root
        // node lies directly atop the root node in the face.
        m_tree->translate(rc);
    } else {
        // When the placement direction is ordinal, we need to displace the tree's root from the face's root.
        // The displacement is based on the padding that we add to the treebox.
        double padding = m_face.getGraph()->getIEL()/4.0;
        Node_SP unpaddedTreeBox = m_tree->buildRootlessBox(m_growthDir),
                paddedTreeBox = buildTreeBox(padding);
        Point uc = unpaddedTreeBox->getCentre(),
              pc = paddedTreeBox->getCentre();
        double dw = Compass::isIncreasingCard(m_growthDir) ? -padding/2.0 : padding/2.0;
        Point dp = Compass::isVerticalCard(m_growthDir) ? Point(0, dw) : Point(dw, 0);
        m_tree->translate(rc + pc - uc + dp);
    }
}

void TreePlacement::insertTreeIntoGraph(Graph &G, NodesById &treeNodes, NodesById &bufferNodes, EdgesById &treeEdges) {
    // Add the nodes and edges for the tree into the graph.
    m_tree->addNetwork(G, treeNodes, treeEdges);
    bool alignRoot = Compass::isCardinal(m_placementDir);
    // Remove the box node for the tree if it is in the graph.
    if (m_boxNode != nullptr && G.hasNode(m_boxNode->id())) G.severAndRemoveNode(*m_boxNode);
    // Add constraints to maintain the layout of the tree.
    m_tree->addConstraints(G, alignRoot);
    // Add buffer nodes to keep space around the tree, and constraints to hold them in place.
    m_tree->addBufferNodesAndConstraints(G, bufferNodes);
}

bool TreePlacement::rootIsAlignedWith(vpsc::Dim dim, id_type id) {
    return m_rootAligns[dim].count(id) > 0;
}
