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
#include <map>
#include <set>
#include <algorithm>
#include <cmath>
#include <sstream>
#include <deque>
#include <sstream>
#include <stdexcept>
#include <vector>
#include <iterator>

#include "libavoid/geomtypes.h"
#include "libvpsc/rectangle.h"

#include "libdialect/commontypes.h"
#include "libdialect/expansion.h"
#include "libdialect/ortho.h"
#include "libdialect/constraints.h"

using namespace dialect;

using std::max;
using std::fabs;

using Avoid::Point;

ExpansionManager::ExpansionManager(TreePlacement_SP tp, vpsc::Dim primaryDim, double padding)
    : m_tp(tp),
      m_primaryDim(primaryDim)
{
    // Set padding.
    if (padding < 0) {
        padding = m_tp->getFace().getGraph()->getIEL()/4.0;
    }
    m_padding = padding;
    // Determine axial and transverse dimensions.
    m_axialDim = Compass::varDim.at(m_tp->getGrowthDir());
    m_transDim = vpsc::conjugate(m_axialDim);
    // Compute goals, first in the primary dimension...
    computeGoals(m_primaryDim);
    // ...and then in the complementary dimension.
    computeGoals(vpsc::conjugate(m_primaryDim));
}

ExpansionGoal_SP ExpansionManager::addGoalInDirec(CardinalDir dir) {
    ExpansionGoal_SP g = std::make_shared<ExpansionGoal>(m_goalCounter++, m_tp, dir, m_padding);
    m_goals.push_back(g);
    return g;
}

void ExpansionManager::computeGoals(vpsc::Dim dim) {
    CompassDir pdir = m_tp->getPlacementDir();
    CardinalDir gdir = m_tp->getGrowthDir();
    ExpansionGoals &goalsByDim = dim == m_primaryDim ? m_primaryDimGoals : m_secondaryDimGoals;
    if (isTransverse(dim)) {
        // For the transverse dimension, there is always just one goal, and it
        // points in the same direction as the growth direction of the tree placement.
        goalsByDim.push_back(addGoalInDirec(gdir));
    } else {
        // For the axial dimension, what we do depends on whether the placement direction is cardinal or ordinal.
        if (Compass::isCardinal(pdir)) {
            // Cardinal placement direction.
            // Need two goals for axial sepcos, one in each direction.
            CardinalDir cwdir = Compass::cardRotateCw90(gdir),
                        acwdir = Compass::cardRotateAcw90(gdir);
            goalsByDim.push_back(addGoalInDirec(cwdir));
            goalsByDim.push_back(addGoalInDirec(acwdir));
        } else {
            // Ordinal placement direction.
            // Need just one goal point for axial sepcos, in the direction which
            // is the complement of the growth dir, w.r.t. the placement dir.
            CardinalDirs comps = Compass::cardinalComponents(pdir);
            CardinalDir compDir = comps[0] == gdir ? comps[1] : comps[0];
            goalsByDim.push_back(addGoalInDirec(compDir));
        }
    }
}

double ExpansionManager::estimateCost(void) const {
    std::map<vpsc::Dim, double> costByDim = estimateCostByDimension();
    return costByDim.at(vpsc::XDIM) + costByDim.at(vpsc::YDIM);
}

std::map<vpsc::Dim, double> ExpansionManager::estimateCostByDimension(void) const {
    std::map<CardinalDir, double> costByDir = estimateCostByDirection();
    std::map<vpsc::Dim, double> costByDim;
    costByDim.insert({vpsc::XDIM, costByDir[CardinalDir::EAST] + costByDir[CardinalDir::WEST]});
    costByDim.insert({vpsc::YDIM, costByDir[CardinalDir::SOUTH] + costByDir[CardinalDir::NORTH]});
    return costByDim;
}

std::map<vpsc::Dim, double> ExpansionManager::estimateCostByDimension2(void) const {
    ExpansionGoals xGoals = m_primaryDim == vpsc::XDIM ? m_primaryDimGoals : m_secondaryDimGoals;
    ExpansionGoals yGoals = m_primaryDim == vpsc::YDIM ? m_primaryDimGoals : m_secondaryDimGoals;
    // We will not actually do any projections; just compute projection sequences.
    bool doProject = false;
    // XDIM
    ProjSeq_SP psX = std::make_shared<ProjSeq>();
    for (ExpansionGoal_SP g : xGoals) {
        ContainedSegment_SP cs = g->computeContainedSegment(false);
        cs->makeRoomForTreeNode(psX, doProject);
    }
    double xCost = psX->violation();
    // YDIM
    ProjSeq_SP psY = std::make_shared<ProjSeq>();
    for (ExpansionGoal_SP g : yGoals) {
        ContainedSegment_SP cs = g->computeContainedSegment(false);
        cs->makeRoomForTreeNode(psY, doProject);
    }
    double yCost = psY->violation();
    std::map<vpsc::Dim, double> costByDim{ {vpsc::XDIM, xCost}, {vpsc::YDIM, yCost} };
    return costByDim;
}

std::map<CardinalDir, double> ExpansionManager::estimateCostByDirection(void) const {
    std::map<CardinalDir, double> costs;
    // First we consider collateral expansion constraints, if any.
    ProjSeq_SP ps0 = m_tp->getFace().computeCollateralProjSeq(m_tp, m_padding);
    // For each direction in which these constraints act (relative to
    // the root node of our TreePlacement), we want to know the maximum
    // amount of violation in that direction. We set this as the cost in
    // that direction.
    // We also want to keep track of the directions addressed by collateral expansion,
    // so we form a set for that.
    std::set<CardinalDir> collateral_direcs;
    // Get ahold of the root node.
    Node_SP root = m_tp->getRootNode();
    // Iterate over all constraints in the projection sequence.
    SepCoSet sepCos = ps0->getAllConstraints();
    for (SepCo_SP sc : sepCos) {
        CardinalDir d = sc->getDirecRelativeToNode(root);
        collateral_direcs.insert(d);
        double v = sc->violation();
        costs[d] = max(v, costs[d]);
    }
    // Now we also want to consider any of our goal segments that are in
    // a direction other than one already addressed by collateral expansion
    // (if any). The idea is that, as an estimate, we will guess that colatteral
    // expansion will already buy enough room, in any direction in which it acts.
    for (ExpansionGoal_SP eg : m_goals) {
        CardinalDir d = eg->getDirec();
        if (collateral_direcs.find(d) == collateral_direcs.end()) {
            // In such a case we base our estimate on a comparison of the goal segment
            // against the available space (i.e. the "contained segment").
            // However, since the collateral expansion has only been /described/ (by the
            // necessary constraints), and not actually /performed/, we need to ignore
            // collateral tree boxes. This is because those tree boxes might currently
            // obstruct the contained segment, giving a misrepresentation of the space
            // that would likely become available were the collateral expansion
            // actually to be performed.
            bool ignoreCollateralTreeBoxes = true;
            ContainedSegment_SP cs = eg->computeContainedSegment(ignoreCollateralTreeBoxes);
            double sh = cs->measureShortage();
            costs[d] = sh;
        }
    }
    // Done.
    return costs;
}

ProjSeq_SP ExpansionManager::extendProjSeq(ProjSeq_SP ps0) {
    // Start with the first goal.
    ExpansionGoal_SP g0 = m_goals.front();
    // Prepare the queue of remaining goals.
    std::deque<ExpansionGoal_SP> remainingGoals(m_goals.size()-1);
    std::copy(m_goals.begin()+1, m_goals.end(), remainingGoals.begin());
    // Initiate the recursive expansion.
    ProjSeq_SP ps1 = g0->tryExpansionRec(ps0, remainingGoals);
    if (ps1) {
        // Reset so can be applied again by client.
        ps1->reset();
    }
    return ps1;
}


void ExpansionGoal::computeGoalSegmentEndpoints(Avoid::Point &p0, Avoid::Point &p1) {
    CompassDir pdir = m_tp->getPlacementDir();
    Node_SP root = m_tp->getRootNode();
    // Base point on the boundary of the root node:
    Point b = root->getBoundaryCompassPt(pdir);
    // Centre point of root node:
    Point r = root->getCentre();
    // Extreme coords of tree box:
    Node_SP treeBox = m_tp->buildTreeBox(m_padding);
    dimensions td = treeBox->getDimensions();
    Point t = treeBox->getCentre();
    double w = td.first,
           h = td.second,
           x = r.x + t.x - w/2.0,
           X = r.x + t.x + w/2.0,
           y = r.y + t.y - h/2.0,
           Y = r.y + t.y + h/2.0;
    // possible goal coordinates:
    double gx[3] = {x, b.x, X};
    double gy[3] = {y, b.y, Y};
    // p0 = b
    p0.x = b.x;
    p0.y = b.y;
    // select p1 according to vector signs for our direction
    Point sgn = Compass::vectorSigns((CompassDir)m_direc);
    p1.x = gx[(int)sgn.x + 1];
    p1.y = gy[(int)sgn.y + 1];
}

ContainedSegment_SP ExpansionGoal::computeContainedSegment(bool ignoreCollateralTreeBoxes) {
    // Start by constructing the closed and open boundary segments facing the direction of interest.
    CardinalDir facingDir = Compass::cardFlip(m_direc);
    LineSegments closedSegs, openSegs;
    TreePlacement_SP ignoreTP = ignoreCollateralTreeBoxes ? m_tp : nullptr;
    m_tp->getFace().buildBdrySegsFacingOneDir(facingDir, closedSegs, openSegs, ignoreTP);
    // Build the endpoints of the goal segment.
    Point p0, p1;
    computeGoalSegmentEndpoints(p0, p1);
    // Determine the constant and extreme coords.
    double z, w0, w1;
    bool isVertical = Compass::isVerticalCard(m_direc);
    if (isVertical) {
        z = p0.x; w0 = p0.y; w1 = p1.y;
    } else {
        z = p0.y; w0 = p0.x; w1 = p1.x;
    }
    // Build list of boundary segments that contain the goal segment's constant coord.
    LineSegments segs(closedSegs.size() + openSegs.size());
    auto it = std::copy_if(closedSegs.begin(), closedSegs.end(), segs.begin(), [z](const LineSegment_SP &seg)->bool{
        return seg->closedIntervalIncludesCoord(z);
    });
    it = std::copy_if(openSegs.begin(), openSegs.end(), it, [z](const LineSegment_SP &seg)->bool{
        return seg->openIntervalIncludesCoord(z);
    });
    segs.resize(it-segs.begin());
    // Now suppose p2 is the point on the line segment (p0, p1) farthest from p0 but
    // still inside the face. Then, for some w2 between w0 and w1, we have either p2 = (z, w2)
    // (for vertical goal segs) or p2 = (w2, z) (for horizontal goal segs).
    // Our task now is to compute w2.
    // We initialize w2 to equal w1.
    double w2 = w1,
           d2 = fabs(w2 - w0),
           e1 = w1 - w0;
    // Then we look through the boundary segments and consider each one's constant coord wb.
    // If wb is on the same side of w0 as w1 is, and if wb is closer to w0 than w2 is, this
    // represents the nearest crossing found so far. So we set w2 = wb.
    for (LineSegment_SP seg : segs) {
        double wb = seg->getConstCoord(),
               eb = wb - w0;
        if (eb*e1 <= 0) {
            // eb * e1 == 0 means eb == 0, i.e. we're considering a boundary segment that
            // actually crosses through the base point. We don't want to consider those.
            // eb * e1 < 0 means we're on the wrong side of the base point.
            continue;
        }
        // If we make it this far, we have a line segment that is at least on the correct side
        // of the base point. Now we need to check whether it is closer than w2.
        double db = fabs(eb);
        if (db < d2) {
            // The segment is closer. So update the point w2, and its distance d2.
            w2 = wb;
            d2 = db;
        }
    }
    // Having considered all possible crossing segments, we have found the contained segment.
    Point localGoalPt(isVertical ? z : w2, isVertical ? w2 : z);
    ContainedSegment_SP cs = std::make_shared<ContainedSegment>(m_id, p0, localGoalPt, p1, m_padding, m_tp, m_dim);
    return cs;
}

ProjSeq_SP ExpansionGoal::tryExpansionRec(ProjSeq_SP ps0, std::deque<ExpansionGoal_SP> &remainingGoals) {
    // We need the ContainedSegment.
    ContainedSegment_SP cs = computeContainedSegment();
    // We use a fresh copy of the given projeciton sequence, so that it remains unaltered,
    // and other expansions can be attempted with it.
    ProjSeq_SP ps1 = std::make_shared<ProjSeq>(*ps0);
    bool ok = cs->makeRoomForTreeNode(ps1);
    if (!ok) {
        return nullptr;
    }
    // Are there any further goals?
    if (!remainingGoals.empty()) {
        // If so, then recurse.
        // Take the first remaining goal.
        ExpansionGoal_SP g0 = remainingGoals.front();
        remainingGoals.pop_front();
        // Try the expansion, recursively.
        ps1 = g0->tryExpansionRec(ps1, remainingGoals);
    }
    return ps1;
}


ContainedSegment::ContainedSegment(unsigned goalID, Avoid::Point basept, Avoid::Point localGoalPt, Avoid::Point globalGoalPt,
                 double padding, TreePlacement_SP tp, vpsc::Dim sepDim)
    : goalID(goalID), basept(basept), localGoalPt(localGoalPt), globalGoalPt(globalGoalPt),
      padding(padding), tp(tp), sepDim(sepDim),
      localGoalSeg(basept, localGoalPt),
      globalGoalSeg(basept, globalGoalPt) {}


double ContainedSegment::measureShortage(void) const {
    double excess = globalGoalSeg.length - localGoalSeg.length;
    return excess < 0 ? 0 : excess;
}

std::string ContainedSegment::toString(void) const {
    std::ostringstream ss;
    ss << "ContainedSegment:";
    ss << " from (" << basept.x << "," << basept.y << ")";
    ss << " to (" << localGoalPt.x << "," << localGoalPt.y << ")";
    ss << " toward (" << globalGoalPt.x << "," << globalGoalPt.y << ")";
    return ss.str();
}

void ContainedSegment::addSepCoForPtAndNode(Avoid::Point &pt, Node_SP &node, dimensions nodeDims, SepCoSet &sepcos, Node_SP &treeBox) {

    // Set a very small extra gap, just to ensure that boxes do not wind up being regarded
    // as overlapping when only by a hair's breadth (and thereby engendering further expansion
    // constraints that really aren't needed).
    double extraGap = 1;

    // Get root node of tree placement.
    Node_SP root = tp->getRootNode();
    // Determine dimension.
    bool isVertGoalSeg = Compass::isVerticalCard(globalGoalSeg.direc);
    // Determine on which side of the segment the given point lies.
    int sign = localGoalSeg.ptOnWhichSide(pt);
    // In all cases where we need this utility method, it is appropriate to simply do nothing
    // in the case where the given pt turns out to lie on neither side of this segment (i.e. to
    // lie in-line with it).
    if (sign == 0) return;

    // Determine the left and right nodes for the sep co.
    Node_SP left  = sign == -1 ? node : root,
            right = sign == -1 ? root : node;
    // Compute the gap for the sep co.
    Point treeCentre = treeBox->getCentre();
    dimensions treeDims = treeBox->getDimensions();

    double gap = isVertGoalSeg ? nodeDims.first/2.0  + treeDims.first/2.0  + sign*treeCentre.x :
                          nodeDims.second/2.0 + treeDims.second/2.0 + sign*treeCentre.y;
    gap += extraGap;

    // Note: Due to sign term, the gap may be negative. However, we do want to support that case, and
    // our SepCo class does support it.
    SepCo_SP sc = std::make_shared<SepCo>(sepDim, left, right, gap);
    sepcos.insert(sc);
}

bool ContainedSegment::makeRoomForTreeNode(ProjSeq_SP ps0, bool doProject) {
    // We build a set of SepCos, add it to the projection sequence, and attempt to project.
    SepCoSet sepcos;
    // Get the tree box and root node.
    Node_SP treeBox = tp->buildTreeBox(padding),
            root = tp->getRootNode();
    // We use open intervals.
    bool openInterval = true;

    // Phase 1: Sides
    // Here we set up constraints between the root node and any of the Face's Sides that lie
    // opposite the desired treebox.

    // As we work, we build the set of all TreePlacements attached to any Sides with which we set up any
    // constraints. This is in order to then remove these from consideration in Phase 2 below.
    // Otherwise conflicting constraints may accidentally be set up.
    std::set<TreePlacement_SP> sideTps;

    // We can only work with obstacles that lie opposite the /local/ goal seg.
    Sides sides = tp->getFace().getAllSidesOppositeSegment(localGoalSeg, openInterval);

    for (Side_SP S : sides) {
        // If the root node lies on the Side, then we need no constraints.
        if (S->containsNode(root->id())) continue;
        // Check any Node on the Side.
        Node_SP someNode = S->firstNode();
        // If it is aligned with the root node in the same dimension as the Side itself is aligned,
        // then we cannot (and need not) set any constraints with this Side.
        vpsc::Dim algnDim = S->getAlignmentDimension();
        if (tp->rootIsAlignedWith(algnDim, someNode->id())) continue;

        // We need to know the maximum thickness of the Side opposite the longer of the two segments,
        // which is always the global goal segment.
        double sideHalfWidth = S->halfWidthOppositeSegment(globalGoalSeg);
        // If the Side is actually in-line with this segment, then we can't set up any constraint.
        // This is okay, since a ContainedSegment in the other dimension can handle any overlap with
        // such a Side.
        if (sideHalfWidth == -1) continue;
        // Otherwise get a point opposite the segment.
        Point pt = S->getFirstPtOppositeSegment(localGoalSeg);

        // We will set up a constraint with this Side by setting up a constraint between the
        // TreePlacement's root node, and the random node ('someNode') we got earlier.
        // But we need to set dummy dimensions for that node, so that its relevant half-dimension
        // winds up being that of the Side in question, which we computed above.
        double w = 2*sideHalfWidth;
        dimensions dummyDims = {w, w};
        // And now can add a separation constraint.
        addSepCoForPtAndNode(pt, someNode, dummyDims, sepcos, treeBox);
        // Having set up a constraint with this Side, we must remove all its attached TreePlacements
        // from separate consideration.
        const std::set<TreePlacement_SP> &tps = S->getTreePlacements();
        sideTps.insert(tps.cbegin(), tps.cend());
    }

    // Phase 2: Tree boxes
    // Here we set up constraints between the root node and any existing treeboxes in the Face,
    // if they are not attached to any of the Sides we have already considered.

    // Begin by getting the set of all TreePlacements in the Face.
    std::set<TreePlacement_SP> allTps = tp->getFace().getSetOfAllTreePlacements();
    // Subtract those that were attached to Sides with which we set up constraints.
    std::set<TreePlacement_SP> tps;
    std::set_difference(allTps.begin(), allTps.end(), sideTps.begin(), sideTps.end(), std::inserter(tps, tps.end()));
    for (TreePlacement_SP tp1 : tps) {
        // Determine whether the *unpadded* tree box lies opposite the local goal segment, and
        // if so obtain some point that lies opposite.
        Point pt;
        double noPadding = 0;
        bool liesOpposite = tp1->somePointOppositeSegment(localGoalSeg, pt, noPadding, openInterval);
        // If it does not lie opposite the segment, then continue to the next case.
        if (!liesOpposite) continue;
        // Otherwise proceed.
        // For computing the gap for the sepco, use the actual box node, which (a) should have been computed by
        // now, and which (b) *does* include padding.
        Node_SP nbrTreeBox = tp1->getBoxNode();
        addSepCoForPtAndNode(pt, nbrTreeBox, nbrTreeBox->getDimensions(), sepcos, treeBox);
    }

    // Finally extend the projection sequence. Attempt to project if so requested; otherwise, simply
    // assume feasible.
    ps0->addProjection(sepcos, sepDim);
    bool feasible = true;
    if (doProject) {
        feasible = tp->getFace().applyProjSeq(*ps0);
    }
    return feasible;
}
