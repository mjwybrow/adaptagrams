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

#ifndef DIALECT_FACES_H
#define DIALECT_FACES_H

#include <vector>
#include <set>
#include <map>
#include <string>

#include "libvpsc/rectangle.h"

#include "libdialect/commontypes.h"
#include "libdialect/ortho.h"
#include "libdialect/opts.h"
#include "libdialect/constraints.h"

namespace dialect {

//! A side of a Face. E.g. a rectangular Face has four Sides: north, south, east, and west.
class Side {
public:

    //! @brief  Standard constructor.
    //! @param[in] nodeSeq  The Nodes belonging to this Side, in clockwise order
    //!                     w.r.t. the Face to which the Side belongs.
    //! @param[in] direc  The cardinal direction in which we move as we go forward
    //!                   through the sequnce of Nodes.
    //!
    //! @note  In theory the direc could be computed from the given nodeSeq; however,
    //!        wherever possible we want to promote the setting of directions logically,
    //!        based on existing constraints, rather than inferring directions geometrically.
    //!        When Faces construct Sides, there is such a constrained direction in place.
    Side(Nodes nodeSeq, CardinalDir direc);

    //! @brief  Get a copy of the node sequence.
    Nodes getNodeSeq(void) const { return m_nodeSeq; }

    //! @brief  Check whether this Side contains a Node of the given ID.
    bool containsNode(id_type id) const;

    //! @brief  Get the index of a Node in this Side's node sequence.
    //! @param[in] id  The ID of the Node to be found.
    //! @return  The index where the given ID occurs, or -1 if not found.
    size_t findNodeIndex(id_type id) const;

    //! @brief  Check the forward direction of this Side.
    CardinalDir getForwardDirec(void) const;

    //! @brief  Check the dimension in which this Side is aligned.
    vpsc::Dim getAlignmentDimension(void) const { return m_vardim; }

    //! @brief  Get a pointer to the first Node on this Side.
    Node_SP firstNode(void) const;

    //! @brief  Get a pointer to the last Node on this Side.
    Node_SP lastNode(void) const;

    //! @brief  Write a string representation.
    std::string toString(void) const;

    //! @brief  Check the centre coordinate of this Side.
    //! @note  We are working under the assumption that all Nodes belonging to a given
    //!        Side are centre-aligned!
    double getCentreCoord(void) const;

    //! @brief  Check how many of the Nodes on this Side are marked as root nodes.
    size_t getNumRootNodes(void) const;

    //! @brief  Record a TreePlacement as having been placed on this Side.
    void addTreePlacement(TreePlacement_SP tp);

    //! @brief  Compute a projection sequence to remove/prevent overlaps between the given
    //!         TreePlacement's tree box, and any existing tree boxes already on this Side,
    //!         as well as ordinary perimeter nodes on this Side.
    //! @param[in] tp  The TreePlacement to be added to this Side.
    //! @param[in] padding  Optional padding for the new tree box.
    //! @return  A ProjSeq representing the computed projection sequence.
    ProjSeq_SP computeCollateralProjSeq(TreePlacement_SP tp, double padding=0);

    //! @brief  Compute the closed interval [a, b], where a and b are the extreme coordinates
    //!         covered by this Side, up to the extremes of the boxes of the extreme Nodes.
    //! @return The closed interval covered by the Side.
    interval closedInterval(void) const;

    //! @brief  Compute the closed interval [a, b] that is the intersection of this Side's
    //!         closed interval with that of a given Segment.
    //! @param[in] seg  The LineSegment in question.
    //! @param[in] openInterval  Set true if you want to instead intersect this Side's closed
    //!                          interval with the LineSegment's open interval.
    //! @return  An interval representing the intersection.
    //! @note  Returns an empty interval (b < a) if the two intervals do not intersect, or
    //!        if the Side and the LineSegment are not aligned in the same dimension.
    interval getIntervalOppositeSegment(LineSegment &seg, bool openInterval=false) const;

    //! @brief  Check whether the closed interval spanned by this Side runs in the same
    //!         dimension as a given line segment, and overlaps it in projection onto
    //!         that dimension.
    //! @param[in] seg  The LineSegment in question.
    //! @param[in] openInterval  Set true if you want to instead intersect this Side's closed
    //!                          interval with the LineSegment's open interval.
    //! @return boolean
    bool liesOppositeSegment(LineSegment &seg, bool openInterval=false) const;

    //! @brief  Compute the first point of the interval of this Side that lies opposite
    //!         a given line segment.
    //! @param[in] seg  The LineSegment in question.
    //! @return  The Point.
    //! @throws  Runtime error if this Side does not lie opposite the given segment.
    //! @sa Side::liesOppositeSegment
    Avoid::Point getFirstPtOppositeSegment(LineSegment &seg) const;

    //! @brief  Given a LineSegment, find that portion of this Side that lies opposite it,
    //!         (if any) and report the maximum half-width of the near half.
    //! @param[in] seg  The LineSegment in question.
    //! @return  The desired half-width. Will be equal to -1 if the Side's interval does not intersect
    //!          that of the segment, or if it does but the two are in-line with one another.
    //! @note  The Edges of the Side are given the thickness value set for aligned edges in the underlying Graph.
    double halfWidthOppositeSegment(LineSegment &seg) const;

    //! @brief  Read-only access to the set of TreePlacements that have been attached to this Side.
    const std::set<TreePlacement_SP> &getTreePlacements(void) const { return m_treePlacements; }

private:

    //! The Nodes belonging to this Side, in clockwise order w.r.t. the Face to which
    //! the Side belongs:
    Nodes m_nodeSeq;
    //! The cardinal direction in which we move as we go forward through the sequnce of Nodes:
    CardinalDir m_forward;
    //! The cardinal direction pointing into the interior of the Face:
    CardinalDir m_inward;
    //! The varying dimension:
    vpsc::Dim m_vardim;
    //! The constant dimension:
    vpsc::Dim m_constdim;
    //! Set of TreePlacements that have been placed on this Side:
    std::set<TreePlacement_SP> m_treePlacements;

};

enum class NexusPolarity {
    ENTER_FROM,
    EXIT_TO
};

//! Regarded as a member of a Face F, a Node u belongs to certain Sides si
//! of F. As we traverse the face in the clockwise direction (i.e. so that the
//! interior of the face is always to the /right/), each Side si gets a direction,
//! and therefore may stand in one of eight relations to Node u: it may be /entering/
//! or /exiting/, and this may be from or to any of the four cardinal compass directions.
//!
//! A single Side may stand in two such relations, as when the Node lies along
//! the middle of the Side, or else in just one such relation, as when a Node
//! lies at one end or the other.
//!
//! A Nexus represents a Node in this capacity as a "joining point" of several
//! Sides of a Face. It stores eight "slots" that are either empty or else occupied by
//! a Side object.
class Nexus {
public:

    //! @brief  Standard constructor.
    //! @param[in] u  The Node that this Nexus is to represent.
    Nexus(Node_SP u) : m_node(u), m_slots(8) {}

    //! @brief  Add a Side to the Nexus.
    void addSide(Side_SP side);

    //! @brief  Find out what are the first objects you hit as you try sweeping both
    //!         clockwise and anticlockwise, starting from a given direction.
    //! @param[in] direc  The given direction.
    //! @return  Set of Sides.
    //! @note  The size of the returned set will be either 0, 1, or 2, according to whether
    //!        the number of filled slots in this Nexus is 0, 1, or more, respectively.
    std::set<Side_SP> getNeighboursOfADirection(CompassDir direc);

    //! @brief  Write a string representation.
    std::string toString(void) const;

private:

    //! @brief  Record a Side in the proper slot.
    //! @param[in] direc  The side of the Nexus concerned.
    //! @param[in] polarity  The polarity of the connection.
    //! @param[in] side  The Side instance that is connecting.
    //!
    //! For example, consider the node labeled x in this picture:
    //!
    //!                 etc.
    //!                  |
    //!                a |
    //!                  |
    //!             b    |
    //!     etc. ------- x    e
    //!             c    |
    //!                  |
    //!                d |
    //!                  |
    //!                 etc.
    //!
    //! and regarded as a member of the external face. As we trace the external face in its
    //! clockwise direction (which looks like anticlockwise except from the point at infinity)
    //! we travel south down side a, west along side b, east along side c, south along side d,
    //! and north along side e. (This example also demonstrates that two different sides, such
    //! as b and c, can be made up of the same Nodes, only traversed in different directions.)
    //!
    //! For the Nexus at x, we would then use this method to record the following six facts:
    //!     Side a enters from the north
    //!     Side b exits to the west
    //!     Side c enters from the west
    //!     Side d exits to the south
    //!     Side e enters from the south
    //!     Side e also exits to the north
    //!
    void writeSlot(CardinalDir direc, NexusPolarity polarity, Side_SP side);

    //! When searching for the neighbours of a given direction, we need to start
    //! the search at the right index. We store a lookup for this.
    static const std::map<CompassDir, size_t> DIREC_TO_INITIAL_SEARCH_INDEX;

    Node_SP m_node;
    std::vector<Side_SP> m_slots;
    bool m_isEmpty = true;
};

//! Holds all the Face objects for a given 4-planar, orthogonal layout,
//! and provides methods to use and manage them.
class FaceSet {
public:
    FaceSet(Graph_SP &G);

    //! @brief  Check how many faces there are.
    size_t getNumFaces(void) { return m_faces.size(); }

    //! @brief  Compute all the possible ways of placing a given Tree into the Faces,
    //!         given that it must connect at a particular root node in the underlying Graph.
    //! @param[in] tree  The Tree to be placed.
    //! @sa Face::listAllPossibleTreePlacements.
    TreePlacements listAllPossibleTreePlacements(Tree_SP tree);

    //! @brief  After tree placements have actually been chosen and performed (i.e. trees have
    //!         been placed into faces), obtain a vector of all those TreePlacements that were
    //!         actually chosen.
    TreePlacements getAllTreePlacements(void);

    //! @brief  Get a copy of the vector of Faces_SP's.
    Faces getFaces(void) { return m_faces; }

    //! @brief  Get the external Face.
    Face_SP getExternalFace(void) { return m_externalFace; }

    //! @brief  After tree placements have been chosen and performed, get a count of trees
    //!         by growth direction.
    //! @param[in] scaleBySize  Set true if for each tree you actually want to count its number
    //!                         of nodes. In other words, the final counts you get actually indicate
    //!                         the number of nodes in trees that grow in the given directions.
    std::map<CardinalDir, size_t> getNumTreesByGrowthDir(bool scaleBySize=false) const;

private:

    //! Part of construction; compute and store all the Faces of the given Graph.
    void computeFaces(void);
    //! Part of construction; determine which is the external Face.
    void identifyExternalFace(void);

    //! The Graph to which the Face belongs:
    Graph_SP m_graph;
    Faces m_faces;
    Face_SP m_externalFace;
    //! Let each Node ID map to the vector of Faces to which it belongs.
    std::map<id_type, std::set<Face_SP>> m_facesByMemberNodeId;
    //! We also want lookups for aligned sets of nodes, i.e. want to be able to look up
    //! for any given node, what is the set of all other nodes with which it is aligned
    //! in either dimension.
    std::map<id_type, std::set<id_type>> m_hSets;
    std::map<id_type, std::set<id_type>> m_vSets;
};


//! Represents a single face of a 4-planar, orthogonal layout.
class Face {
    friend class FaceSet;
public:
    //! @brief  Standard constructor.
    //! @param[in] G  The Graph to which the Face belongs.
    Face(Graph_SP &G) : m_ID(nextID++), m_graph(G) {}

    //! @brief  A method to build the Face, based on the clockwise sequence of "edges"
    //!         of which it is made up; here an "edge" is a mere IdPair.
    void initWithEdgeSeq(const std::vector<IdPair> &edges);

    //! @brief  Access the sequence (vector) of Nodes belonging to the Face,
    //!         in clockwise order.
    Nodes getNodeSeq(void) { return m_nodeSeq; }

    //! @brief  Get a copy of the vector of Sides.
    Sides getSides(void) { return m_sides; }

    //! @brief  Get a copy of the Nexus lookup.
    NexesById getNexusLookup(void) { return m_nexesByNodeIds; }

    //! @brief  Utility method, to test whether the Face contains a given sequence
    //!         of Node IDs, in clockwise cyclic order.
    bool containsNodeIdSeq(std::vector<id_type> idSeq) const;

    //! @brief  Access the underlying Graph.
    Graph_SP getGraph(void) { return m_graph; }

    //! @brief  Access the unique ID of a given instance.
    //!
    //! @return  The ID.
    id_type id(void) const { return m_ID; }

    //! @brief  Check whether this is the external face or not.
    bool isExternal(void) const { return m_isExternal; }

    //! @brief  Get a string representation.
    std::string toString(void) const;

    //! @brief  Access the neighbour pairs.
    std::map<id_type, std::vector<std::pair<Node_SP, Node_SP>>> getNbrPairs(void) {return m_nbrPairs;}

    //! @brief  Get a vector of all Sides that are relevant to a given TreePlacement.
    Sides getRelevantSidesForPlacement(TreePlacement_SP tp) const;

    //! @brief  Compute all the possible ways of placing a given Tree into this Face,
    //!         at a given root Node belonging to this Face.
    //!
    //! @param[out] tps  A vector of TreePlacements to which all those allocated by this
    //!                  method are to be added.
    //! @param[in] tree  The Tree to be placed.
    //! @param[in] root  The Node, belonging to this Face, where the Tree is to attach.
    void listAllPossibleTreePlacements(TreePlacements &tps, Tree_SP tree, Node_SP root);

    //! @brief  List the compass directions in which an edge could point if it were
    //!         anchored at the given Node, and pointed inward, into the Face.
    CompassDirs inwardDirsAvailable(Node_SP node);

    //! @brief  To be used after the face has been expanded to make room for the tree.
    //!         This method adds a large node to the graph, representing the bounding
    //!         box of the tree, with padding.
    //!         The treenode is constrained to lie beside its root node.
    //! @param[in] tp  A TreePlacement representing the Tree to be placed.
    //! @param[in] padding  Optional padding to be added to the tree box.
    //! @sa TreePlacement::getTreeBox for interpretation of the padding.
    void insertTreeNode(TreePlacement_SP tp, double padding=0);

    //! @brief  Compute a projection sequence to remove/prevent overlaps between the given
    //!         TreePlacement's tree box, and any existing tree boxes or ordinary perimeter
    //!         Nodes on relevant Sides of this Face.
    //! @param[in] tp  The TreePlacement to be added to this Face.
    //! @param[in] padding  Optional padding for the new tree box.
    //! @return  A ProjSeq representing the computed projection sequence.
    //!
    //! @note  In the case where tp's root lies on two sides of the face, these two
    //! sides must be aligned in complementary dimensions (i.e. one in x, and one
    //! in y), so the sepcos generated here are always independent.
    //!
    //! That is, if each side has an existing treenode that must be pushed away,
    //! the sepco for one tree will never achieve the push for the other, since
    //! these pushes /must/ be done in complementary dimensions.
    //!
    //! Therefore there is no need to wait until after the first removal has been
    //! actually performed to compute the sepco for the second removal; it cannot
    //! be affected by the first one. This means we can safely compute them simultaneously.
    ProjSeq_SP computeCollateralProjSeq(TreePlacement_SP tp, double padding=0);

    //! @brief  Convenience function for applying a ProjSeq with all the appropriate options.
    //! @param[in] ps  The ProjSeq to be applied.
    //! @return boolean, saying whether all the projections were successful.
    bool applyProjSeq(ProjSeq &ps);

    //! @brief  Perform collateral expansion for a given TreePlacement. This means
    //!         pushing Nodes and tree boxes on relevant Sides out of the way, to make
    //!         room for the new tree box.
    //! @param[in] tp  The TreePlacement requiring space.
    //! @param[in] padding  Optional padding to be added to the new tree box. Leave
    //!                     negative if you want padding equal to iel/4 to be automatically
    //!                     applied, where iel is the underlying graph's current ideal edge
    //!                     length.
    //! @return  The ProjSeq that was used to achieve the required expansion. This
    //!          ProjSeq will have been already evaluated and applied when returned.
    //! @throws  Runtime exception if any of the collateral expansion projections
    //!          are infeasible. In theory this should never happen.
    ProjSeq_SP doCollateralExpansion(TreePlacement_SP tp, double padding=-1);

    //! @brief  Build LineSegments to represent all those segments of the internal boundary
    //!         of this Face that face in a given direction. These segments can represent
    //!         aligned Edges, or parts of boundaries of Nodes or tree boxes.
    //! @param[in] facingDir  CardinalDir telling which side the segments should face.
    //! @param[out] closedSegs  Vector in which to store all constructed closed segments.
    //! @param[out] openSegs  Vector in which to store all constructed open segments.
    //! @param[in] ignoreTP  Optional TreePlacement whose collateral tree boxes should be ignored.
    //!
    //! @note  Whether a boundary segment is closed or open means whether one crosses the interior
    //! boundary of the face at the endpoints (yes for closed, no for open).
    //!
    //! For example, if facingDir is SOUTH, then closedSegs will get one LineSegment for every 
    //! horizontal Edge in the Face, while openSegs will get a LineSegment representing the 
    //! southern boundary of (a) each Node in the Face, and (b) each Tree box in the Face.
    void buildBdrySegsFacingOneDir(CardinalDir facingDir,
                                   LineSegments &closedSegs, LineSegments &openSegs,
                                   TreePlacement_SP ignoreTP = nullptr);

    //! @brief  Build the best projection sequence for a given tree placement.
    //! @param[in] tp  The TreePlacement to be added to this Face.
    //! @param[in] padding  Optional padding for the new tree box.
    //! @param[in] doCostlierDimensionFirst  See TreePlacement::buildBestProjSeq.
    //! @param[in] estimateMethod  See defn of ExpansionEstimateMethod enum class.
    //! @return  A ProjSeq representing the computed projection sequence.
    ProjSeq_SP buildBestProjSeq(TreePlacement_SP tp, double padding=0,
                                bool doCostlierDimensionFirst=false,
                                ExpansionEstimateMethod estimateMethod=ExpansionEstimateMethod::CONSTRAINTS);

    //! @brief  Get all the Sides of this Face that lie opposite a given LineSegment.
    //! @param[in] seg  The LineSegment in question.
    //! @param[in] openInterval  Set true if you want to consider the overlap with the LineSegment's
    //!                          open interval instead of closed (the default).
    //! @return  Vector of Sides.
    Sides getAllSidesOppositeSegment(LineSegment &seg, bool openInterval=false) const;

    //! @brief  Get all TreePlacements that have been added to this Face.
    //! @return  Vector of TreePlacements.
    TreePlacements getAllTreePlacements(void) const;

    //! @brief  Get the set of all TreePlacements that have been added to this Face.
    //! @return  Set of TreePlacements.
    std::set<TreePlacement_SP> getSetOfAllTreePlacements(void) const;

    //! @brief  Get all TreePlacements that have been added to this Face.
    //! @param[out] tps  Vector of TreePlacements to which those belonging to this
    //!                  Face should be added.
    void getAllTreePlacements(TreePlacements &tps) const;

    //! @brief  After tree placements have been chosen and performed, get a count of trees
    //!         by growth direction.
    //! @param[out] counts  Map into which the counts should be added.
    //! @param[in] scaleBySize  Set true if for each tree you actually want to count its number
    //!                         of nodes. In other words, the final counts you get actually indicate
    //!                         the number of nodes in trees that grow in the given directions.
    void getNumTreesByGrowthDir(std::map<CardinalDir, size_t> &counts, bool scaleBySize=false) const;

private:

    //! Construction steps:
    void computeNbrPairs(void);
    void computeSides(void);
    void buildNexes(void);

    //! @brief Scanning through this Face's nodeseq, look for the first place where
    //!        a bend occurs, i.e. where the incoming and outgoing directions are different.
    //! @return  The index where the first bend occurs.
    //! @note  We say a bend occurs at an index, not at a Node, since a Node may
    //! be encountered more than once during traversal of the Face. On one encounter
    //! a bend may happen there, while on another it may not. Consider for example
    //! the external face and node x here:
    //!
    //!                 etc.
    //!                  |
    //!            etc.--x
    //!                  |
    //!                 etc.
    //!
    //! In this example node x is encountered three times: twice a bend occurs
    //! there; once it does not.
    size_t findIndexOfFirstBend(void);

    //! Check the constrained cardinal direction from one node to another.
    //! Since we only work with constrained 4-planar orthogonal graphs, this should always be defined.
    //! @throws  Runtime error if no cardinal constraint has been defined for these nodes.
    CardinalDir direc(const Node_SP &u, const Node_SP &v);

    //! For class-specific generation of unique IDs:
    static id_type nextID;
    //! An instance's own unique ID:
    const id_type m_ID;

    //! The Graph to which the Face belongs:
    Graph_SP m_graph;
    //! All Nodes belonging to the face, in clockwise order:
    Nodes m_nodeSeq;
    //! The number of Nodes in the Face:
    size_t m_n;
    //! Flag indicating whether this Face is the external one:
    bool m_isExternal = false;
    //! For any given Node u belonging to the Face, we want to be able to look up
    //! its "neighbour pairs", by which we mean those pairs of Nodes (v, w) such that
    //! the sequence v -> u -> w is encountered in clockwise cyclic traversal of the Face.
    //! So we keep a lookup of the form,
    //!             u.id --> [{v0, w0}, {v1, w1}, ...]
    std::map<id_type, std::vector<std::pair<Node_SP, Node_SP>>> m_nbrPairs;
    //! The Sides of the Face:
    Sides m_sides;
    //! The Nexes, indexed by the IDs of the Nodes they represent:
    NexesById m_nexesByNodeIds;
    //! Nodes representing Trees placed into this Face:
    NodesById m_treeNodes;
    //! We keep a record of any TreePlacements placed at any Nodes of this Face.
    //! Since at most one Tree can be rooted at any given Node, we need record at most
    //! one TreePlacement per Node.
    std::map<id_type, TreePlacement_SP> m_treePlacementsByNodeIds;

};


} // namespace dialect

#endif // DIALECT_FACES_H
