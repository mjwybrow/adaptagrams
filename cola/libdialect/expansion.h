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

#ifndef DIALECT_EXPANSION_H
#define DIALECT_EXPANSION_H

#include <memory>
#include <map>
#include <string>
#include <deque>

#include "libvpsc/rectangle.h"
#include "libavoid/geomtypes.h"

#include "libdialect/commontypes.h"
#include "libdialect/treeplacement.h"
#include "libdialect/ortho.h"

namespace dialect {

class ExpansionGoal;

/**
 * @brief The ExpansionManager class
 *
    An instance of this class is meant to manage the generation of nonoverlap
    constraints, when making room for a TreePlacement in a Face of a planar
    orthogonal layout.

    The idea is that we choose strategic "goal points" in the face, and compute
    the line segments from the tree's attachment point to these goal points.
    We can then use these line segments to determine the nonoverlap constraints
    by pushing everything on one side of the line to one side, and everything
    on the other side of the line to the other side.

    A TreePlacement has both a "placement direction" dp, and a "growth
    direction" dg. The former is any compass point, cardinal or ordinal, while
    the latter must be cardinal. Expansion constraints are said to be "axial"
    when they operate in the dimension to which dg belongs, and to be
    "transverse" when they operate in the complementary dimension.
    For example if dg is NORTH, then SepCo's in the y-dim are axial,
    while those in the x-dim are transverse.

    Similarly, goal points are called axial or transverse according to the type
    of constraints they generate. If the placement direction dp is cardinal, then
    there is one transverse goal point, and there are two axial goal poitns.
    If the placement direction dp is ordinal, there there is one transverse and
    one axial goal point.

    For example, for a north-growing tree that was also placed north, see the
    figure on the left, below. Rooted at the "o", the three goal points are marked
    by "a", "b", "c". Goal point "a" is the one transverse goal point, since it will
    generate expansion constraints operating left and right, i.e. in the x-dimension.
    Meanwhile points "b" and "c" are the two axial goal points, since they will
    generate expansion constraints operating up and down, i.e. in the y-dimension.

                ---a---         a-------
               |      |         |      |
               |      |         |      |
               c---o--b         o------b

    The figure on the right shows the two goal points for a north-growing tree that
    was placed north-east. Here there is one transverse goal point ("a"), and one
    axial ("b").

    For each goal point the ExpansionManager constructs an ExpansionGoal object,
    which in turn uses the ContainedSegment class to generate expansion constraints.

    Note: In an earlier version of HOLA (the Python version from 2015), this process
    was much more complex, potentially considering many more goal points, spanning
    face concavities. In those caes libavoid was used to find an orthogonal route
    to the goal point, and the segments of the route were used to select the expansion
    constraints. That system allowed us to consider all possible ways of expanding
    a face. The present system is faster and simpler, and is deemed to produce
    results of equal quality.
 */
class ExpansionManager {
public:
    //! @brief  Standard constructor.
    //! @param[in] tp  The TreePlacement for which expansion is required.
    //! @param[in] primaryDim  The dimension in which to expand first.
    //! @param[in] padding  Optional padding to add to tree boxes involved in the
    //!                     expansion process. Leave negative for automatic setting
    //!                     equal to one quarter of the ideal edge length of the
    //!                     underlying graph.
    ExpansionManager(TreePlacement_SP tp, vpsc::Dim primaryDim=vpsc::HORIZONTAL, double padding=-1);

    //! @brief  Check whether a given dimension is axial w.r.t. this manager's placement.
    bool isAxial(vpsc::Dim dim) { return dim == m_axialDim; }

    //! @brief  Check whether a given dimension is transverse w.r.t. this manager's placement.
    bool isTransverse(vpsc::Dim dim) { return dim == m_transDim; }

    //! @brief  Estimate the cost of expanding.
    double estimateCost(void) const;

    //! @brief  Estimate the cost of expanding in each dimension.
    //!         This method looks at the length of contained segments, and reports the
    //!         sum of shortfalls.
    std::map<vpsc::Dim, double> estimateCostByDimension(void) const;

    //! @brief  Estimate the cost of expanding in each dimension.
    //!         This method computes the initial separation constraints in each dimension and
    //!         reports the sum of their violations.
    std::map<vpsc::Dim, double> estimateCostByDimension2(void) const;

    //! @brief  Estimate the cost of expanding in each cardinal direction.
    std::map<CardinalDir, double> estimateCostByDirection(void) const;

    //! @brief  Access the goals.
    ExpansionGoals getGoals(void) { return m_goals; }

    //! @brief  Extend a given projection sequence with those projections necessary
    //!         to achieve all expansion goals of this manager.
    //! @param[in] ps0  The given ProjSeq.
    //! @return A new ProjSeq that extends the given one, or nullptr if any attempted
    //!     projection is impossible.
    ProjSeq_SP extendProjSeq(ProjSeq_SP ps0);

private:

    ExpansionGoal_SP addGoalInDirec(CardinalDir dir);
    void computeGoals(vpsc::Dim dim);

    TreePlacement_SP m_tp;
    vpsc::Dim m_primaryDim;
    double m_padding;
    //! The axial dimension:
    vpsc::Dim m_axialDim;
    //! The transverse dimension:
    vpsc::Dim m_transDim;
    //! A counter, to enumerate goals:
    unsigned m_goalCounter = 0;
    //! Keep vector of goals:
    ExpansionGoals m_goals;
    ExpansionGoals m_primaryDimGoals;
    ExpansionGoals m_secondaryDimGoals;
};

struct ContainedSegment;
typedef std::shared_ptr<ContainedSegment> ContainedSegment_SP;

/**
 * @brief The ExpansionGoal class
 *
 * Manages a single expansion goal point, as utilised by the ExpansionManager class.
 *
 */
class ExpansionGoal {
public:

    //! @brief Standard constructor.
    //!
    //! @param[in] id  An ID for the goal (unique for the manager).
    //! @param[in] tp  The TreePlacement in question.
    //! @param[in] direc  The direction of the goal, from base point to goal point.
    //! @param[in] padding  Padding for tree boxes.
    ExpansionGoal(unsigned id, TreePlacement_SP tp, CardinalDir direc, double padding)
        : m_id(id), m_tp(tp), m_direc(direc), m_padding(padding) {
        // The generated SepCos will operate at right angles to the direction of this
        // goal, i.e. in the constant dimension of this goal's direction.
        m_dim = Compass::constDim.at(m_direc);
    }

    //! @brief  Compute the contained segment for this goal.
    //!
    //! @param[in] ignoreCollateralTreeBoxes  This is useful when we are interested in
    //! computing estimates of expansion costs. At such times, we want to guess how long
    //! the contained segment might be once any collateral tree boxes have been moved away
    //! (but without actually doing the moving).
    ContainedSegment_SP computeContainedSegment(bool ignoreCollateralTreeBoxes=false);

    //! @brief  Get the direction of the goal.
    CardinalDir getDirec(void) const { return m_direc; }

    //! @brief  Try to expand, and work recursively, attempting all remaining goals.
    //! @param[in] ps0  The projection sequence to be extended.
    //! @param[in] remainingGoals  Deque of ExpansionGoals on which to recurse.
    //! @return  The resulting projection sequence, in which all expansion goals
    //!          have been achieved, or nullptr if any attempted projection is impossible.
    ProjSeq_SP tryExpansionRec(ProjSeq_SP ps0, std::deque<ExpansionGoal_SP> &remainingGoals);

private:

    //! @brief  Compute the endpoints of the goal segment.
    //! @param[out] p0  The base point.
    //! @param[out] p1  The goal point.
    void computeGoalSegmentEndpoints(Avoid::Point &p0, Avoid::Point &p1);

    unsigned m_id;
    TreePlacement_SP m_tp;
    //! The direction from base point to goal point:
    CardinalDir m_direc;
    double m_padding;
    //! The dimension in which the generated SepCos are to operate:
    vpsc::Dim m_dim;
};

struct ContainedSegment {

    //! @brief  Standard constructor.
    ContainedSegment(unsigned goalID, Avoid::Point basept, Avoid::Point localGoalPt, Avoid::Point globalGoalPt,
                     double padding, TreePlacement_SP tp, vpsc::Dim sepDim);

    //! @brief  Report the "shortage", i.e. the extent to which the global goal
    //!         segment exceeds the local in length. Returns zero if this value
    //!         would be negative.
    double measureShortage(void) const;

    //! @brief  Write a string representation.
    std::string toString(void) const;

    //! @brief  Perform expansion, to make room for the tree node, extending a given
    //!         projection sequence with the projections performed.
    //! @param[in] ps0  The ProjSeq to be extended.
    //! @param[in] doProject  Set false in order to do a 'trial' in which you only extend the given
    //!                       ProjSeq, but do not actually project.
    //! @return  false if any attempted projection is impossible; else true
    bool makeRoomForTreeNode(ProjSeq_SP ps0, bool doProject = true);

    //! Note the ID of the associated expansion goal:
    unsigned goalID;
    //! start of goal segment:
    Avoid::Point basept;
    //! end of goal segment:
    Avoid::Point localGoalPt;
    //! point at end of segment representing space needed by tree node:
    Avoid::Point globalGoalPt;
    //! padding for tree boxes:
    double padding;
    //! the TreePlacement in question:
    TreePlacement_SP tp;
    //! the dimension in which the generated SepCos are to operate:
    vpsc::Dim sepDim;
    //! LineSegment representing the local goal segment:
    LineSegment localGoalSeg;
    //! LineSegment representing the global goal segment:
    LineSegment globalGoalSeg;

private:
    //! @brief  Utility method for building sepcos while making room for tree node:
    //!
    //! In all cases we wish to build a sep co between the tree box for our tree placement, and some other node.
    //! However we may need to set up dummy dimensions for that node, and we need a point to tell us which node
    //! is on which side.
    //!
    //! @param[in] pt  The point we use to determine which node is on which side.
    //! @param[in] node  The node to be separated from the tree box.
    //! @param[in] nodeDims  The dummy dimensions for the node.
    //! @param[out] sepcos  The place where the generated sep co is to be recorded.
    //! @param[in] treeBox  The tree box for our tree placement.
    //!
    //! @note If the point pt lies on the line of which this ContainedSegment is a subset, then we simply do nothing.
    void addSepCoForPtAndNode(Avoid::Point &pt, Node_SP &node, dimensions nodeDims, SepCoSet &sepcos, Node_SP &treeBox);
};


} // namespace dialect

#endif // DIALECT_EXPANSION_H
