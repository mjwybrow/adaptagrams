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

#ifndef DIALECT_TREEPLACEMENT_H
#define DIALECT_TREEPLACEMENT_H

#include <string>
#include <memory>
#include <map>
#include <set>

#include "libvpsc/rectangle.h"

#include "libdialect/commontypes.h"
#include "libdialect/faces.h"
#include "libdialect/graphs.h"
#include "libdialect/trees.h"
#include "libdialect/ortho.h"
#include "libdialect/opts.h"
#include "libdialect/constraints.h"
#include "libdialect/logging.h"

namespace dialect {

//! @brief  Given a planar orthogonal core, and the corresponding Trees (as resulting
//!         from the peeling process), choose Faces of the core in which to place the
//!         Trees, generate constraints to expand those Faces, and insert large Nodes
//!         to represent the bounding boxes of the Trees.
FaceSet_SP reattachTrees(Graph_SP core, Trees trees, HolaOpts opts, Logger *logger=nullptr);

//! @brief  Choose the best TreePlacement from among a list of alternatives.
TreePlacement_SP chooseBestPlacement(TreePlacements tps, HolaOpts opts);

//! A TreePlacement represents a way of placing a tree into the faces of
//! a 4-planar, orthogonal layout.
class TreePlacement
#ifndef SWIG
// We hide this from SWIG, lest it complain, "Nothing known about base class 'std::enable_shared_from_this..."
// http://swig.10945.n7.nabble.com/Nothing-known-about-base-class-std-enable-shared-from-this-tp14603p14604.html
        : public std::enable_shared_from_this<TreePlacement>
#endif
{
public:
    TreePlacement(Tree_SP tree, Face &face, Node_SP faceRoot,
                  CompassDir dp, CardinalDir dg, bool flip=false)
        : m_ID(nextID++),
          m_tree(tree),
          m_face(face),
          m_faceRoot(faceRoot),
          m_placementDir(dp),
          m_growthDir(dg),
          m_flip(flip),
          m_cost(0),
          m_boxNode(nullptr) {}

    //! @brief  Get the placement direction.
    CompassDir getPlacementDir(void) const { return m_placementDir; }

    //! @brief  Get the growth direction.
    CardinalDir getGrowthDir(void) const { return m_growthDir; }

    //! @brief  Check whether the placement is into the external face.
    bool isExternal(void) const { return m_face.isExternal(); }

    //! @brief  Get the unique ID of this instance.
    id_type id(void) const { return m_ID; }

    //! @brief  Check the number of "potential neighbours" of this tree, if placed
    //!         according to this placement. This is equal to the number of other
    //!         root nodes on the relevant Sides to which the root node of this
    //!         placement belongs.
    size_t getNumPotentialNbrs(void);

    //! @brief  Estimate the cost of this placement.
    double estimateCost(void);

    //! @brief  Get the Node at which the Tree would be rooted:
    Node_SP getRootNode(void) const { return m_faceRoot; }

    //! @brief  Access the Face to which this placement belongs.
    Face &getFace(void) { return m_face; }

    //! @brief  Insert a node representing the Tree into the Face to which this
    //!         placement belongs.
    //! @param[in] padding  Optional padding to be added to the tree box.
    //! @sa TreePlacement::getTreeBox for interpretation of the padding.
    void insertTreeNode(double padding=0) { m_face.insertTreeNode(shared_from_this(), padding); }

    //! @brief  Get a string representation.
    std::string toString(void) const;

    //! @brief  Determine the size of the tree minus the root node, and the position
    //!         relative to the root node.
    //!
    //! @param[in] padding  Optionally, you may request padding be added to the /outside/
    //!                     sides of the box, i.e. all those except the side facing the
    //!                     root node. For example, for a north-growing tree, the padding
    //!                     will be added only to the north, east, and west sides.
    //!                     Recommended value for padding is one quarter of the Graph's
    //!                     ideal edge length (iel/4.0).
    //!
    //! @return  A Node whose dimensions w x h gives the dimensions of the bounding box
    //!          of the tree, minus the root node, and whose position (x, y) gives the
    //!          vector from the centre of the root node to the centre of this box.
    //!
    //! @note  The dimensions are in respect of the growth direction of this TreePlacement,
    //!        and the position vector is in respect of its placement direction and flip bit.
    Node_SP buildTreeBox(double padding=0) const;

    //! @brief  Record the Node representing the box for the Tree.
    void recordBoxNode(Node_SP &boxNode) { m_boxNode = boxNode; }

    //! @brief  Check whether this TreePlacement has a box node yet.
    bool hasBoxNode(void) { return m_boxNode != nullptr; }

    //! @brief  Get the box node.
    Node_SP getBoxNode(void) { return m_boxNode; }

    //! @brief  Build the best projection sequence for this tree placement.
    //! @param[in] padding  Optional padding for the new tree box.
    //! @param[in] doCostlierDimensionFirst  Set true to do the more expensive dimension first,
    //!             in hopes that this will obviate expansion in the other dimension altogether.
    //! @param[in] estimateMethod  See defn of ExpansionEstimateMethod enum class.
    //! @return  A ProjSeq representing the computed projection sequence.
    ProjSeq_SP buildBestProjSeq(double padding=0, bool doCostlierDimensionFirst=false,
                                ExpansionEstimateMethod estimateMethod=ExpansionEstimateMethod::CONSTRAINTS) {
        return m_face.buildBestProjSeq(shared_from_this(), padding, doCostlierDimensionFirst, estimateMethod);
    }

    //! @brief  Compute some point belonging to the tree box and lying opposite a given
    //!         line segment.
    //! @param[in] seg  The LineSegment in question.
    //! @param[out] pt  The point whose coordinates are to be set.
    //! @param[in] padding  Padding for the tree box.
    //! @param[in] openInterval  Set true if you want to use the open interval of the line
    //!                          segment instead of closed (the default).
    //! @return boolean saying whether the tree box indeed lies opposite the given line segment.
    //!
    //! @note  We compute the interval I of the tree box in the dimension parallel to the segment.
    //! Let J be the interval of the segment -- open if the openInterval argument is true; closed
    //! otherwise. Let K be the intersection of I and J. If K is empty then we return false.
    //! Otherwise we pick a value w in K and return a point having w as one of its coordinates and
    //! a centre coordinate of the tree box as the other coordinate.
    bool somePointOppositeSegment(LineSegment &seg, Avoid::Point &pt, double padding=-1, bool openInterval=false);

    //! @brief  Rotate, flip, and translate the tree as necessary to match this placement.
    void applyGeometryToTree(void);

    //! @brief  Insert the tree into a given Graph.
    //! @param[out] G  The Graph into which the tree is to be inserted.
    //! @param[out] treeNodes  A place to record the tree nodes that are constructred.
    //! @param[out] bufferNodes  A place to record the buffer nodes that are constructred.
    //! @param[out] treeEdges  A place to record the tree edges that are added.
    void insertTreeIntoGraph(Graph &G, NodesById &treeNodes, NodesById &bufferNodes, EdgesById &treeEdges);

    //! @brief  Check the size (i.e. number of nodes in) the Tree.
    size_t size(void) const { return m_tree->size(); }

    //! @brief  Tell the TreePlacement which Nodes are aligned with its root node in a given dimension.
    //! @param[in] dim  The alignment dimension.
    //! @param[in] idSet  The set of IDs of nodes that are aligned in this dimension with the root node.
    void setRootAligns(vpsc::Dim dim, std::set<id_type> &idSet) { m_rootAligns[dim] = idSet; }

    //! @brief  Check whether the root node is aligned with a given node, in a given dimension.
    //! @param[in] dim  The dimension of interest.
    //! @param[in] id  The ID of the node in question.
    bool rootIsAlignedWith(vpsc::Dim dim, id_type id);

private:

    //! For class-specific generation of unique IDs:
    static id_type nextID;
    //! An instance's own unique ID:
    const id_type m_ID;

    //! The Tree to be placed:
    Tree_SP m_tree;
    //! The Face into which we consider placing it:
    Face &m_face;
    //! The Node in the Face, where the Tree would be rooted:
    Node_SP m_faceRoot;
    //! The placement direction considered:
    CompassDir m_placementDir;
    //! The growth direction of the Tree:
    CardinalDir m_growthDir;
    //! Whether the Tree's layout should be flipped over an axis parallel to its growth direction:
    bool m_flip;

    //! An estimated cost of this placement:
    double m_cost;
    //! A node to represent the bounding box of the Tree:
    Node_SP m_boxNode = nullptr;

    //! Want to know which other nodes are aligned with the root node in each dimension.
    std::map<vpsc::Dim, std::set<id_type>> m_rootAligns;

};


} // namespace dialect

#endif // DIALECT_TREEPLACEMENT_H
