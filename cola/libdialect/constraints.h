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

#ifndef DIALECT_CONSTRAINTS_H
#define DIALECT_CONSTRAINTS_H

#include <map>
#include <set>
#include <string>
#include <vector>
#include <utility>
#include <memory>

#include "libvpsc/rectangle.h"
#include "libcola/compound_constraints.h"

#include "libdialect/commontypes.h"
#include "libdialect/ortho.h"

namespace dialect {

class Graph;

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// SepMatrices

enum class GapType {
    //! Gap between the centres of two nodes:
    CENTRE,
    //! Gap between the opposite boundaries of two nodes:
    BDRY
};

enum class SepDir {
    //! Cardinal constraints imply both a separation and an alignment:
    EAST, SOUTH, WEST, NORTH,
    //! Lateral constraints imply only a separation:
    RIGHT, DOWN, LEFT, UP
};

enum class SepType {
    //! No constraint:
    NONE,
    //! An exact separation:
    EQ,
    //! A minimal separation:
    INEQ
};

SepDir negateSepDir(SepDir sd);

bool sepDirIsCardinal(SepDir sd);

CardinalDir sepDirToCardinalDir(SepDir sd);

SepDir cardinalDirToSepDir(CardinalDir dir);

SepDir lateralWeakening(SepDir sd);

SepDir cardinalStrengthening(SepDir sd);

enum class SepTransform {
    //! Rotate 90 degrees clockwise.
    ROTATE90CW,
    //! Rotate 90 degrees anticlockwise.
    ROTATE90ACW,
    //! Rotate 180 degrees.
    ROTATE180,
    //! Flip over vertical axis.
    FLIPV,
    //! Flip over horizontal axis.
    FLIPH,
    //! Flip over the "main diagonal" i.e. the axis whose slope is positive
    //! in the graphics plane. This goes from upper left to lower right,
    //! since in the graphics plane the positive y-axis points downward.
    FLIPMD,
    //! Flip over the "off diagonal".
    FLIPOD
};

class SepMatrix;

struct SepPair {
    SepPair(void) : src(0), tgt(0), xgt(GapType::CENTRE), ygt(GapType::CENTRE), xst(SepType::NONE), yst(SepType::NONE), xgap(0.0), ygap(0.0) {}

    id_type src;
    id_type tgt;
    GapType xgt;
    GapType ygt;
    SepType xst;
    SepType yst;
    double xgap;
    double ygap;

    //! String formatting precision for doubles, for writing TGLF:
    unsigned tglfPrecision = 3;

    //! Generally, clients do not interact directly with SepPairs; they interact with a SepMatrix.
    //! They either want to set a constraint, or check for an existing constraint, FROM a node of
    //! ID id1, TO a node if ID id2. (Constraints, in general, are directed.)
    //!
    //! However, the SepMatrix stores only an upper-triangular matrix of SepPairs, so that in every stored
    //! SepPair, src < tgt. This is possible because every constraint has an opposite. However, this means that every
    //! getter and setter method in the SepMatrix has to be wary of this fact, and when the retrieved
    //! SepPair has been flipped relative to the given IDs (i.e. when src == id2 and tgt == id1), then
    //! the getter/setter method has to flip the constraint before reporting/recording.
    //!
    //! The SepMatrix works in multiple layers. At the deepest layer is a pair of private methods -- getSepPair
    //! and checkSepPair -- which are responsible for retrieving/allocating (getSepPair), and checking for
    //! preexistence (checkSepPair). These are the only two methods that are ever to interact with the upper-triangular
    //! storage matrix itself. Above this deepest layer is the layer of getter/setter methods, which are public and
    //! for use by clients.
    //!
    //! As an aid to this design, the SepPair struct stores a 'flippedRetrieval' field. The field is set by the deep
    //! layer methods, to report whether the SepPair is flipped relative to the given IDs. It is used by the public
    //! layer methods when getting and setting, in order to know when the constraint needs to be flipped.
    //! Clients must understand that, outside of this context, this field is meaningless!
    bool flippedRetrieval = false;

    //! @brief  Add a constraint.
    //! @sa  addSep method of SepMatrix class
    void addSep(GapType gt, SepDir sd, SepType st, double gap);
    //! @brief  Apply a transformation.
    void transform(SepTransform tf);
    //! @brief  Check whether this SepPair represents a separation in a vertical cardinal compass direction.
    bool isVerticalCardinal(void) const;
    //! @brief  Check whether this SepPair represents a separation in a horizontal cardinal compass direction.
    bool isHorizontalCardinal(void) const;
    //! @brief  Check whether this SepPair represents a vertical alignment.
    bool isVAlign(void) const;
    //! @brief  Check whether this SepPair represents a horizontal alignment.
    bool isHAlign(void) const;
    //! @brief  Check whether this SepPair represents a separation in a cardinal compass direction.
    bool isCardinal(void) const;
    //! @brief  Get the cardinal direction of this separation.
    //! @throws  Runtime exception if this separation is not cardinal.
    //! @returns The cardinal direction.
    CardinalDir getCardinalDir(void) const;
    //! @brief  Round gaps upward in absolute value. Useful when trying to move nodes to integer coords.
    //! 
    //! E.g., -2.3 goes to -3.0; 2.3 goes to 3.0.
    void roundGapsUpAbs(void);
    //! @brief  Write a representation of this constraint in the format of
    //!         the SEPCO'S section of the TGLF file format.
    //! @param[in] id2ext  Mapping from internal Node IDs to external IDs for the TGLF output.
    std::string writeTglf(std::map<id_type, unsigned> id2ext, const SepMatrix &m) const;
    //! @brief  Check whether there is a constraint in a given dimension.
    bool hasConstraintInDim(vpsc::Dim dim) const;
    //! @brief  Write the VPSC constraint in one dimension.
    vpsc::Constraint *generateSeparationConstraint(const vpsc::Dim dim, const ColaGraphRep &cgr, SepMatrix *m, vpsc::Variables &vs);
};

//! Since each SepPair may represent up to two VPSC constraints (one in each dimension), we need a simple wrapper
//! struct to represent each dimension.
struct SepPairSubConstraintInfo : public cola::SubConstraintInfo {
    SepPairSubConstraintInfo(SepPair_SP sp, vpsc::Dim dim) : cola::SubConstraintInfo(0), sp(sp), dim(dim) {}
    SepPair_SP sp = nullptr;
    vpsc::Dim dim;
};


static const unsigned int PRIORITY_SEPMATRIX = cola::DEFAULT_CONSTRAINT_PRIORITY;

class SepMatrix : public cola::CompoundConstraint {
public:
    //! @brief  Standard constructor.
    //!
    //! @note  A SepMatrix does not have a "primary dimension" in the sense of the
    //!        cola::CompoundConstraint superclass, so we pass vpsc::UNSET for this.
    SepMatrix(Graph *G) : cola::CompoundConstraint(vpsc::UNSET, PRIORITY_SEPMATRIX), m_graph(G) {
        _combineSubConstraints = true;
    }
    //! @brief  No default constructor. There is always an associated Graph.
    SepMatrix(void) = delete;
    //! @brief  Copy constructor.
    SepMatrix(const SepMatrix &m) = default;
    //! @brief  Destructor.
    ~SepMatrix(void) = default;
    //! @brief  Add a constraint.
    //!
    //! @note   The specified separation is /added/ to any existing constraints
    //!         between the two nodes. It overwrites anything with which it is
    //!         in conflict, but leaves everything else intact. For example, if
    //!         the second node was already constrained to lie UP and to the LEFT
    //!         of the first, and then a RIGHT sep is added, then the second node
    //!         will be constrained to lie UP and to the RIGHT of the first.
    //!
    //! @note   You cannot /remove/ a constraint using this method. If NONE is passed
    //!         as the SepType, nothing happens.
    //!
    //! @param[in]  id1  The ID of the first node.
    //! @param[in]  id2  The ID of the second node.
    //! @param[in]  gt  The gap type (centre-to-centre, or between opposing boundaries).
    //! @param[in]  sd  The direction of the separation from the first node
    //!                 to the second node.
    //! @param[in]  st  The type of the separation.
    //! @param[in]  gap  The size of the separation.
    void addSep(id_type id1, id_type id2, GapType gt, SepDir sd, SepType st, double gap);

    //! @brief  Convenience method, for setting that particular type of cardinal separation that merely
    //!         prevents overlap in the stated direction.
    //! @sa SepMatrix::addSep
    void setCardinalOP(id_type id1, id_type id2, dialect::CardinalDir dir) {
        addSep(id1, id2, GapType::BDRY, (SepDir) dir, SepType::INEQ, 0.0);
    }

    //! @brief  Constrain two Nodes to sit at a fixed separation between their centre coordinates.
    //!
    //! @param[in]  id1  The ID of the first node, u.
    //! @param[in]  id2  The ID of the second node, v.
    //! @param[in]  dx   The desired fixed value for v.x - u.x (centre coordinates)
    //! @param[in]  dy   The desired fixed value for v.y - u.y (centre coordinates)
    void addFixedRelativeSep(id_type id1, id_type id2, double dx, double dy);

    //! @brief  Constrain two Nodes to sit at their present exact separation.
    //!
    //! @param[in]  id1  The ID of the first node, u.
    //! @param[in]  id2  The ID of the second node, v.
    void addFixedRelativeSep(id_type id1, id_type id2);

    //! @brief  Align a pair of nodes horizontally.
    void hAlign(id_type id1, id_type id2) { addSep(id1, id2, GapType::CENTRE, SepDir::DOWN, SepType::EQ, 0); }

    //! @brief  Align a pair of nodes vertically.
    void vAlign(id_type id1, id_type id2) { addSep(id1, id2, GapType::CENTRE, SepDir::RIGHT, SepType::EQ, 0); }

    //! @brief  Align a pair of nodes by saying which is the coordinate to be equated.
    //! @param[in]  id1  The ID of the first node, u.
    //! @param[in]  id2  The ID of the second node, v.
    //! @param[in]  eqCoord  vpsc::Dim naming the coordinate to be equated. Thus XDIM means x-coords will
    //!                      be equated, giving a vertical alignment; YDIM means y-coords will be equated,
    //!                      giving a horizontal alignment.
    void alignByEquatedCoord(id_type id1, id_type id2, vpsc::Dim eqCoord);

    //! @brief  Free a pair of Nodes; i.e. remove the SepPair for these Nodes
    //!         completely.
    void free(id_type id1, id_type id2);

    //! @brief  Clear all constraints.
    //!
    //! @note We do not destroy the SepPairs; we merely erase our pointers to them.
    void clear(void) { m_sparseLookup.clear(); }

    //! @brief  Set corresponding constraints in another SepMatrix.
    //!
    //!         This means that for each constraint between nodes of IDs id1 and id2 in this
    //!         SepMatrix, we set that constraint in the other SepMatrix if and only if its
    //!         underlying Graph contains Nodes of IDs id1 and id2.
    //! @param[out] matrix  The other SepMatrix.
    void setCorrespondingConstraints(SepMatrix &matrix) const;

    //! @brief  Set a SepPair directly into another SepMatrix.
    //! @param[in]  id1  The primary ID under which the SepPair is to be stored.
    //! @param[in]  id2  The secondary ID under which the SepPair is to be stored.
    //! @param[in]  sp  The SepPair to be stored.
    //! @throws  Runtime error if id1 >= id2.
    void setSepPair(id_type id1, id_type id2, SepPair_SP sp);

    //! @brief  Apply a transformation to all Nodes.
    void transform(SepTransform tf);
    //! @brief  Apply a transformation to a closed subset of all Nodes.
    //!
    //! @param[in]  tf  the transformation to be performed
    //! @param[in]  ids  the set of IDs of all Nodes to which the transformation
    //!                  should be applied. /Both/ Nodes must be in the set.
    //! @sa transformOpenSubset
    void transformClosedSubset(SepTransform tf, const std::set<id_type> &ids);
    //! @brief  Apply a transformation to an open subset of all Nodes.
    //!
    //! @param[in]  tf  the transformation to be performed
    //! @param[in]  ids  the set of IDs of all Nodes to which the transformation
    //!                  should be applied. /At least one/ Node must be in the set.
    //! @sa transformClosedSubset
    void transformOpenSubset(SepTransform tf, const std::set<id_type> &ids);
    //! @brief  Remove all records for the Node of given ID.
    void removeNode(id_type id);
    //! @brief  Remove all records for the given Nodes.
    void removeNodes(const NodesById &nodes);
    //! @brief  Get the cardinal direction of the separation between two Nodes.
    //! @param[in] id1  The ID of one of the Nodes.
    //! @param[in] id2  The ID of the other Node.
    //! @throws  Runtime exception if this separation is not cardinal.
    //! @returns The cardinal direction.
    CardinalDir getCardinalDir(id_type id1, id_type id2) const;
    //! @brief  Determine which sets of nodes are aligned with one another.
    //! @param[out] hSets  Will be populated with a lookup from Node ID to set of IDs of
    //!                    all Nodes that are horizontally aligned with that one.
    //! @param[out] vSets  Like hSets, only for vertical alignment.
    void getAlignedSets(std::map<id_type, std::set<id_type>> &hSets,
                        std::map<id_type, std::set<id_type>> &vSets) const;
    //! @brief  Check whether two nodes are horizontally aligned.
    bool areHAligned(id_type id1, id_type id2) const;
    //! @brief  Check whether two nodes are vertically aligned.
    bool areVAligned(id_type id1, id_type id2) const;
    //! @brief  Write a representation of all constraints in the format of
    //!         the SEPCO'S section of the TGLF file format.
    //! @param[in] id2ext  Mapping from internal Node IDs to external IDs for the TGLF output.
    std::string writeTglf(std::map<id_type, unsigned> id2ext) const;
    //! @brief  Set the related Graph.
    void setGraph(Graph *G) { m_graph = G; }
    //! @brief  Get the Graph.
    Graph *getGraph(void) { return m_graph; }
    //! @brief  Round gaps up to next largest integer. Useful if desiring to move all nodes to
    //!         integer coordaintes.
    void roundGapsUpward(void);
    //! @brief  Set a global value to be added onto the gap value for all GapType::BDRY constraints.
    void setExtraBdryGap(double extraBdryGap) { m_extraBdryGap = extraBdryGap; }
    double getExtraBdryGap(void) const { return m_extraBdryGap; }


    //! We implement the interface of a cola::CompoundConstraint
    void generateVariables(const vpsc::Dim dim, vpsc::Variables& vars);
    void generateSeparationConstraints(const vpsc::Dim dim,
            vpsc::Variables& vs, vpsc::Constraints& cs,
            vpsc::Rectangles& bbs);
    std::string toString(void) const;
    void markAllSubConstraintsAsInactive(void);
    cola::SubConstraintAlternatives getCurrSubConstraintAlternatives(vpsc::Variables vs[]);

private:

    //! In many cases it is useful to be able to set a global value to be added onto
    //! the gap value for all GapType::BDRY gaps.
    double m_extraBdryGap = 0.0;

    //! Method for retrieving the SepPair for any pair of Node IDs.
    //! This method should always be used IFF your intention is to set a new constraint
    //! for the named pair. If you only want to CHECK any possibly existing constraint
    //! for this pair, then you MUST use the checkSepPair method instead.
    //! OTHERWISE YOU WILL ALLOCATE GARBAGE CONSTRAINTS THAT YOU DON'T WANT.
    //!
    //! In all cases, the matrix should never be accessed directly. Use one of these two methods.
    //!
    //! This method ensures two things: (1) that the smaller ID always
    //! comes first, and (2) that the src and tgt IDs in the SepPair are always
    //! set.
    //!
    //! Clients can check whether the IDs had to be flipped by checking whether
    //! id1 equals the src ID in the returned SepPair.
    SepPair_SP &getSepPair(id_type id1, id_type id2);

    //! Method for checking any possibly existing constraint.
    //! Returns nullptr if no SepPair is found.
    //! Otherwise returns a COPY of the existing SepPair. This makes sense, because if your
    //! intention is to in any way modify an existing constraint then you should NOT be using
    //! this method.
    //! This method will NOT allocate a new SepPair in the matrix under any circumstance.
    SepPair_SP checkSepPair(id_type id1, id_type id2) const;

    //! We don't mind using a raw pointer since (a) it is quite clear that a SepMatrix
    //! will never be considered responsible for deleting its Graph, and (b) we see no
    //! reasonable scenario in which we need to use a Graph's SepMatrix after the Graph
    //! itself has been deleted.
    Graph *m_graph;

    //! We use a sparse data structure to record configured separations
    //! between Nodes. It is sparse not because we want to save space but
    //! because we don't want to have a fixed, rigid number of Nodes.
    //! Format: smaller Node ID points to map in which larger Node ID points
    //! to SepPair.
    //!
    //! In the SepPair, gap values always mean the gap from the Node of
    //! smaller ID to the node of larger ID.
    //!
    //! For example, suppose for Nodes 4 and 17 we have the SepPair,
    //!
    //!         xtype == SepType::INEQ
    //!         ytype == SepType::EQ
    //!         xgap == -200
    //!         ygap == 0
    //!
    //! This means that Node 17 is configured to lie due West of Node 4,
    //! at a distance of at least 200.
    SparseIdMatrix2d<SepPair_SP>::type m_sparseLookup;
};


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// SepCos, Projections, ProjSeqs


//! Simple struct to represent separation constraints in one dimension, in terms
//! of Nodes (rather than Rectangle indices).
//!
//! Also supports separation constraints with negative gaps (unlike VPSC).
//! Such constraints are perhaps better called "containment constraints". To illustrate,
//! in the figure below
//!
//!             ---
//!            | a |  ------|
//!             ---         |
//!                         |
//!             ---         |
//!            | b |        |
//!             ---         |
//!
//!
//! the separation constraint b.x - 100 <= a.x might mean that node b is constrained to
//! stay to the left of the dashed line (which moves with node a).
//!
struct SepCo {

    //! @brief  Standard constructor
    //! @param[in] dim  The dimension in which the constraint acts.
    //! @param[in] left  The lefthand Node (i.e. the one with smaller coordinate)
    //! @param[in] right  The righthand Node (i.e. the one with larger coordinate)
    //! @param[in] gap  The desired gap between the centres of the Nodes
    //! @param[in] exact  Says whether the gap is exact or a minimum.
    SepCo(vpsc::Dim dim, Node_SP left, Node_SP right, double gap, bool exact=false)
        : dim(dim), left(left), right(right), gap(gap), exact(exact) {}

    //! @brief  Allocate cola::CompoundConstraints to represent this SepCo.
    //! @param[in] cgr  A ColaGraphRep so that Node Id's can be mapped to Rectangle indices.
    //! @param[out] ccs  A CompoundConstraints vector to which to add pointers to the allocated
    //!                  constraints.
    void generateColaConstraints(const ColaGraphRep &cgr, cola::CompoundConstraints &ccs);

    //! @brief  Determine the constrained direction from one Node to the other.
    //! @param[in] baseNode  The Node relative to which the direction is understood.
    //! @return A CardinalDir giving the direction from baseNode to the other node.
    CardinalDir getDirecRelativeToNode(Node_SP baseNode) const;

    //! @brief  Add this constraint to a SepMatrix.
    void addToMatrix(SepMatrix &matrix) const;

    //! @brief  Determine the extent to which this separation constraint is currently violated.
    double violation(void) const;

    //! @brief  Write a string representation.
    std::string toString(void) const;

    vpsc::Dim dim;
    Node_SP left;
    Node_SP right;
    double gap;
    bool exact;
};

typedef std::shared_ptr<SepCo> SepCo_SP;
typedef std::set<SepCo_SP> SepCoSet;
typedef std::vector<SepCoSet> SepCoSets;

//! A Projection represents a set of constraints (given by SepCos), together
//! with a dimension in which to project.
struct Projection{

    //! @brief  Standard constructor.
    //! @param[in] s  The set of SepCos onto which to project.
    //! @param[in] d  The dimension in which to project.
    Projection(SepCoSet s, vpsc::Dim d) : sepCoSet(s), dim(d) {}

    //! @brief  Check how many SepCos are in the projection.
    size_t size(void) { return sepCoSet.size(); }

    //! @brief  Build a vector of cola CompoundConstraints representing the constraints
    //!         in this Projection.
    //! @param[in] cgr  A ColaGraphRep so that Node Id's can be mapped to Rectangle indices.
    cola::CompoundConstraints generateColaConstraints(const ColaGraphRep &cgr);

    //! @brief  Write a string representation.
    std::string toString(void) const;

    SepCoSet sepCoSet;
    vpsc::Dim dim;
};
typedef std::shared_ptr<Projection> Projection_SP;
typedef std::vector<Projection_SP> Projections;

//! Projection Sequence. Manages a sequence of VPSC projections onto a monotonially
//! increasing set of separation constraints.
class ProjSeq {
public:

    //! @brief  Default constructor.
    ProjSeq(void) {
        // Initialize an empty final set in each dimension.
        m_finalSets[vpsc::XDIM];
        m_finalSets[vpsc::YDIM];
    }

    //! @brief  Add a new set of SepCos, ensuring monotonicity by uniting with
    //!         the previous set of constraints in the same dimension, if any.
    //! @param[in] SepCos  The new set of SepCos to be added.
    //! @param[in] dim  The dimension in which the new set is to operate.
    void addProjection(SepCoSet sepCos, vpsc::Dim dim);

    //! @brief  Get the next Projection, if any.
    //! @return  The next Projection if there is another one, else nullptr.
    Projection_SP nextProjection(void);

    //! @brief  Note a stress change.
    void noteStresschange(double dS) { m_dSes.push_back(dS); }

    //! @brief  Write a string representation.
    std::string toString(void) const;

    //! @brief  When another ProjSeq is added to this one, we simply add each
    //!         Projection from the other one to this one, as usual. Thus is
    //!         monotonicity maintained, and this ProjSeq's pointer is left in place.
    ProjSeq &operator+=(const ProjSeq &rhs);

    //! @brief  Get the set of all constraints, in both dimensions.
    SepCoSet getAllConstraints(void) const;

    //! @brief  Reset to start of sequence.
    void reset(void) { m_ptr = 0; }

    //! @brief  Sum the violations of all SepCos in the final sets.
    double violation(void) const;

private:
    //! Sequence of Projections:
    //! The sets of constraints should be monotonic by dimension; i.e. each one
    //! should contain the last one that was in the same dimension.
    Projections m_projections;
    //! Sequence of stress changes:
    std::vector<double> m_dSes;
    //! Pointer to next constraint set to be applied:
    size_t m_ptr = 0;
    //! For maintaining monotonicity:
    std::map<vpsc::Dim, SepCoSet> m_finalSets;
};

} // namespace dialect

// Global Operators
dialect::ProjSeq operator+(const dialect::ProjSeq &lhs, const dialect::ProjSeq &rhs);

#endif // DIALECT_CONSTRAINTS_H
