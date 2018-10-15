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

#ifndef DIALECT_PEELING_H
#define DIALECT_PEELING_H

#include <memory>
#include <vector>

#include "libdialect/commontypes.h"
#include "libdialect/graphs.h"

namespace dialect {

class PeeledNode;
struct Stem;

typedef std::shared_ptr<PeeledNode> PeeledNode_SP;
typedef std::shared_ptr<Stem> Stem_SP;

//! @brief  Mark as "root" the PeeledNode having largest serial number.
//!
//! @note  The Graph must contain only PeeledNodes.
//!
//! @return  The PeeledNode identified as root.
PeeledNode_SP identifyRootNode(const Graph &graph);

//! @brief  A PeeledNode is a type of GhostNode, used in the peeling process.
class PeeledNode : public GhostNode {
    friend struct Stem;
    friend PeeledNode_SP identifyRootNode(const Graph &graph);
public:
    //! @brief  Factory function.
    //!
    //! @return  A shared_ptr to PeeledNode (PeeledNode_SP).
    static PeeledNode_SP allocate(const Node &node) { return PeeledNode_SP(new PeeledNode(node)); }

    //! @brief  Copy-assignment operator
    PeeledNode &operator=(const PeeledNode&) = default;

    //! @brief  Destructor
    virtual ~PeeledNode(void) = default;

private:
    //! @brief  Default constructor is deleted.
    PeeledNode(void) = delete;

    //! @brief  Copy constructor is deleted.
    PeeledNode(const PeeledNode&) = delete;

    //! @brief  We always make a PeeledNode as a copy of a plain Node.
    PeeledNode(const Node &node)
      : GhostNode(node),
        m_treeSerialNumber(nextTreeSerialNumber++) {}

    //! PeeledNodes keep a "tree serial number" that is
    //! useful in figuring out which PeeledNode is the root Node of the peeled tree.
    static id_type nextTreeSerialNumber;
    id_type m_treeSerialNumber;
    void updateSerialNumber(void) { m_treeSerialNumber = nextTreeSerialNumber++; }

};


//! Represents a leaf node, along with its one edge and neighbour.
struct Stem {
    Stem(const Node_SP &leaf, const Node_SP &root) : m_plain_leaf(leaf), m_plain_root(root) {}
    Node_WP m_plain_leaf;
    Node_WP m_plain_root;
    void addSelfToGraph(Graph &H);
};

//! @brief  Make a Stem object to represent each leaf.
std::vector<Stem_SP> makeStemsFromLeaves(const NodesById &leaves);

//! For sorting all the nodes of a graph into "buckets" according
//! to their degree.
//! You can take all the leaves (degree-1 nodes) and you can move
//! a node from one bucket to another.
struct NodeBuckets {
    //! @brief  Initialize a set of node buckets for the given Graph.
    //!
    //! Creates buckets for all degrees up to the Graph's max degree,
    //! and populates these. Each bucket contains a NodesById map.
    NodeBuckets(Graph &graph);

    //! @brief  Return a copy of the bucket of leaves, and clear the latter.
    NodesById takeLeaves(void);

    //! @brief  Move a node from one bucket to another.
    //!
    //! @param[in]  id  The ID of the Node to be moved.
    //! @param[in]  oldDegree  The bucket in which the Node should be found.
    //! @param[in]  newDegree  The bucket to which the Node should be moved.
    //!
    //! @return  boolean saying whether the Node was found and moved.
    bool moveNode(id_type id, unsigned oldDegree, unsigned newDegree);

    //! @brief  Sever the given Nodes from our Graph.
    //!
    //! @note  This method does modify the Graph.
    //!        For each given Node, all incident Edges are severed, and the Node
    //!        is removed from the Graph.
    //!        The neighbouring Nodes are moved to new buckets as appropriate.
    //! @note  The given Nodes themselves are NOT moved to new buckets.
    //!        This preserves the freedom to handle that operation separately.
    void severNodes(const NodesById &nodes);

    Graph &m_graph;
    unsigned m_maxDegree;
    std::vector<NodesById> m_buckets;
};

//! @brief  Perform the "peeling" process, in which the exterior trees are
//!         removed from the given Graph.
//!
//! @note  See Abello, James, Frank Van Ham, and Neeraj Krishnan.
//!         "Ask-graphview: A large scale graph visualization system."
//!         IEEE transactions on visualization and computer graphics 12
//!         no. 5 (2006): 669-676.
//!
//! @note  The given Graph is modiifed in place. It will be pared down
//!        to its own /core/ -- i.e. all that remains after the trees have
//!        been peeled away.
//!
//!        Each tree includes a root node which is a copy of a node that
//!        remains in the core.
//!
//!        The underlying Graphs of the created Trees consist of PeeledNodes.
//!
//!        In the case that the given Graph is itself a tree, the remaining core
//!        will consist only of the tree's root node. Meanwhile the one Tree will
//!        be a copy of the entire original graph.
//!
//! @return  The trees.
Trees peel(Graph &G);

} // namespace dialect

#endif // DIALECT_PEELING_H
