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

#ifndef DIALECT_TREES_H
#define DIALECT_TREES_H

#include <set>
#include <map>
#include <vector>
#include <string>

#include "libavoid/geomtypes.h"

#include "libdialect/graphs.h"
#include "libdialect/commontypes.h"
#include "libdialect/ortho.h"
#include "libdialect/opts.h"
#include "libdialect/util.h"
#include "libdialect/routing.h"

namespace dialect {

class Tree {
public:

    //! @brief  Standard constructor.
    //!
    //! @param[in]  G  the underlying Graph
    //! @param[in]  root  the root Node of the tree
    Tree(Graph_SP G, Node_SP root);

    //! @brief  Apply the Symmetric Layout algorithm of Manning & Atallah
    //!             See Manning, Joseph, and Mikhail J. Atallah.
    //!             "Fast detection and display of symmetry in trees." (1985).
    //!
    //! @param[in]  growthDir  The direction in which the tree should grow.
    //! @param[in]  nodeSep  Minimal gap between nodes on the same rank.
    //! @param[in]  rankSep  Minimal separation between ranks.
    //! @param[in]  convexOrdering  If true, the broadest and deepest subtrees are
    //!                             placed near the centre; if false, the opposite.
    void symmetricLayout(CardinalDir growthDir, double nodeSep, double rankSep,
                         bool convexOrdering = true);

    //! @brief  Flip the tree's layout over the axis running through the root node
    //!         and aligned with the growth direction.
    void flip(void);

    //! @brief  Translate the tree's layout by a given vector.
    //!
    //! @param[in]  vect  The vector by which to translate.
    void translate(Avoid::Point vect);

    //! @brief  Rotate the tree's layout to attain a desired growth direction.
    //!
    //! @param[in] dg  The desired growth direction.
    void rotate(CardinalDir dg);

    //! @brief  Do not rotate the layout, but simply update the Tree's growth direction by rotating clockwise.
    //! @param quarterTurns  How many quarter turns clockwise to rotate the growth direction.
    void rotateGrowthDirCW(unsigned quarterTurns);

    //! @brief  Access the Tree's underlying Graph.
    Graph_SP underlyingGraph(void) const { return m_graph; }

    //! @brief  Access the Tree's root Node.
    Node_SP getRootNode(void) const { return m_root; }

    //! @brief  Write a string representation of this Tree.
    //!         Useful for debugging.
    std::string repr(void) const;

    //! @brief  Check the ID of the root ndoe.
    id_type getRootNodeID(void) const { return m_root->id(); }

    //! @brief  Check whether the layout is symmetrical.
    //!
    //! @note  Just because you have called a Tree's symmetricLayout method, does not
    //!        mean it acutally got a symmetrical layout! That is only possible if the
    //!        Tree's structure is in fact symmetrical. This method informs you whether
    //!        the latter is the case.
    bool isSymmetrical(void) const { return m_isSymmetric; }

    //! @brief  Determine the size of the tree minus the root node, and the position
    //!         relative to the root node.
    //!
    //! @note  Before calling this method, the Tree should have been laid out, so the root
    //!        node should be centred at (0, 0), and the Tree's growth direction should be set.
    //!
    //! @param[in] growthDir  The growth direction with respect to which the box should be computed.
    //!
    //! @return  A Node whose dimensions w x h gives the dimensions of the bounding box
    //!          of the tree, minus the root node (but not minus the inter-rank space between
    //!          the root node and the first rank), and whose position (x, y) gives the
    //!          vector from the centre of the root node to the centre of this box.
    Node_SP buildRootlessBox(CardinalDir growthDir) const;

    //! @brief  Check how many nodes are in the tree.
    size_t size(void) const { return m_nodes.size(); }

    //! @brief  Add the nodes and edges of this Tree to a RoutingAdapter.
    //!         This is useful either for routing just the Tree itself, or for adding the internal part
    //!         of the tree (i.e. everything except the root node) to another graph where the tree is
    //!         rooted.
    //! @param[out] ra  The RoutingAdapter we are to augment.
    //! @param[in] trt  Which type of routing you want.
    //! @param[in] core  The core Graph in the case that this Tree has been peeled off a core, and is
    //!                  to be reattched.
    //! @note If a core Graph is given, then the root node of the tree will be omitted from the RoutingAdapter.
    void addNetworkToRoutingAdapter(RoutingAdapter &ra, TreeRoutingType trt, Graph_SP core = nullptr);

    //! @brief  Add the Nodes and Edges of this tree to a Graph.
    //! @param[out] G  The Graph to which the Nodes and Edges are to be added.
    //! @param[out] treeNodes  A place to record the tree nodes that are constructred.
    //! @param[out] treeEdges  A place to record the tree edges that are constructred.
    //! @note  Before adding each Node to the given Graph G we check whether G already has a Node that reports
    //!        the same ID; if it does, we do not add the Node. This is useful so that root nodes are not
    //!        overwritten when reattaching peeled trees to a core graph.
    void addNetwork(Graph &G, NodesById &treeNodes, EdgesById &treeEdges);

    //! @brief  Set constraints in a given Graph's SepMatrix in order to maintain the layout of the tree.
    //! @param[out] G  The Graph to which the constraints are to be added.
    //! @param[in] alignRoot  Say whether an alignment constraint should be set between the root node and
    //!                       a centre node (if present) in the first rank. For example, this should be set
    //!                       false in the case of a peeled tree being reattached with ordinal placement direction.
    //! @note  The given Graph G must have Nodes matching the IDs of those in this Tree.
    void addConstraints(Graph &G, bool alignRoot);

    //! @brief  Add buffer nodes to help keep separation between the nodes of the tree and the remainder of
    //!         the given graph G. Set constraints to keep the buffer nodes in place.
    //! @param[out] G  The Graph to which the Nodes and constraints are to be added.
    //! @param[out] bufferNodes  A place to record the buffer nodes that are constructred.
    //! @note  This Tree's addNetworkAndConstraints should have been called first, for the same Graph G.
    void addBufferNodesAndConstraints(Graph &G, NodesById &bufferNodes);

    //! @brief  Add padding to nodes of another graph, corresponding to (same ID) non-root nodes in this tree.
    void padCorrespNonRootNodes(Graph &H, double dw, double dh);

private:

    void clearRankBounds(void);
    std::vector<double> getBounds(unsigned rank, double nodeSep) const;
    Trees getCTrees(void) const;
    std::string computeIsomString(void) const;

    // the underlying Graph
    Graph_SP m_graph;
    // the root Node
    Node_SP m_root;
    // Depth of the tree (number of ranks)
    unsigned m_depth;
    // Breadth of the tree (width of widest rank)
    unsigned m_breadth;
    // Is the layout symmetric?
    bool m_isSymmetric;
    // All nodes lying at or below the root.
    // This can differ from the set of nodes in the underlying Graph m_graph when we are working
    // with C-trees, a special type of subtree used in the symmetric layout process.
    NodesById m_nodes;
    // IDs of those nodes that are leaves
    std::set<unsigned> m_leafIDs;
    // map rank numbers (0-based) to the Nodes belonging to that rank
    std::vector<Nodes> m_nodesByRank;
    // We also want the Nodes of each rank partitioned according to whether they are leaves.
    std::vector<Nodes> m_leavesByRank;
    std::vector<Nodes> m_nonleavesByRank;
    // map Node IDs to the ranks to which those Nodes belong
    std::map<id_type, unsigned> m_rankByNodeID;
    // map ID of child to parent Node
    NodesById m_parents;
    // For layout, we keep track of a pair <lb, ub>, i.e.
    // the lower and upper bounds on the lateral coordinates
    // of the tree, for each rank (e.g. for NORTH growth direction the
    // bounds are on x-coordinates).
    std::vector<std::vector<double>> m_boundsByRank;
    // We also keep global lower and upper bounds on rank coords.
    double m_lb;
    double m_ub;
    /* 
     * If the boundary is tight then the bounds for each rank cover just the
     * nodes on that rank plus half nodeSep on each end; else the bounds for
     * every rank are equal to the tight bounds for the widest rank.
     */
    bool m_boundaryTight;
    // Growth direction
    CardinalDir m_growthDir;
};

} // namespace dialect

#endif // DIALECT_TREES_H
