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

#ifndef DIALECT_CHAINS_H
#define DIALECT_CHAINS_H

#include <deque>
#include <memory>
#include <vector>
#include <map>
#include <utility>

#include "libdialect/commontypes.h"
#include "libdialect/ortho.h"

//! This header defines tools for working with the "chains", i.e. maximal subgraphs
//! composed entirely of "links" (nodes of degree 2), in 4-planar orthogonal layouts.

namespace dialect {

//! In a 4-planar orthogonal layout, a link has one of six possible shapes, depending
//! on which two of its four sides are the ones where its edges meet it.
//! For example, if one edge enters from the south, and the other from the east, then
//! this link is shaped like the top-left corner of a box.
//! This enum names the six possible configurations.
enum class LinkShape {
    TLC, // top-left corner
    H, // horizontal
    BLC, // bottom-left corner
    TRC, // top-right corner
    V, // vertical
    BRC // bottom-right corner
};

typedef std::vector<LinkShape> LinkShapes;

//! The bent LinkShapes, in clockwise order:
extern const LinkShapes bentLinkShapeCw;

//! Get the bent LinkShapes, in clockwise order, starting from a given one.
LinkShapes bentLinkShapeCwFromStartingPt(LinkShape start);

//! Lookup table:
//! Given one of the four bent LinkShapes b, and a CardinalDir d, return the CardinalDir you
//! would be going if you came into bend b going in direction d, and then followed the bend.
extern const std::map<LinkShape, std::map<CardinalDir, CardinalDir>> applyBendToDir;

//! Lookup table:
//! For any of the four bent LinkShapes, return its clockwise incoming CardinalDir.
//! In other words, if you were going clockwise around a box, which direction would
//! you be going when you entered this bend?
extern const std::map<LinkShape, CardinalDir> cwIncomingDirForBend;

//! Lookup table:
//! Suppose you have a chain starting at node A and ending at node Z. Let c be the CompassDir
//! from Z to A (which will be cardinal if A and Z are aligned, else ordinal). Suppose as we
//! traverse the chain from A toward Z we must depart A going in CardinalDir d0, and must enter
//! Z going in CardinalDir d1. Then by looking up (c, d0, d1) in this table, we get a vector of
//! vectors of LinkShapes, giving all possible minimal bend sequences for this chain.
//!
//! Note: Whereas d0 gives both the direction from which we depart node A AND the side of A from which
//! we depart, d1 is the direction we travel as we enter node Z (and therefore is the OPPOSITE of
//! the side of Z at which we enter). For example d1 = EAST means we are traveling east as we enter
//! node Z (but we enter it on its west side).
extern const std::map<CompassDir, std::map<CardinalDir, std::map<CardinalDir, std::vector<std::vector<LinkShape>>>>> minimalBendSeqs;

//! @brief  Look up the minimal bend sequences for a Chain.
//! @param[in] A  The left anchor Node of the Chain.
//! @param[in] d0  The CardinalDir in which the chain departs from Node A.
//! @param[in] Z  The right anchor Node of the Chain.
//! @param[in] d1  The CardinalDir in which the chain enters Node Z (i.e. the CardinalDir from the
//!                last Node of the Chain toward the right anchor Node).
//! @return Vector of vectors of LinkShapes, giving all possible minimal bend sequences for this Chain.
//! @note This does work correctly even for the case of a chain in which the left and right anchor nodes are the
//!       same (as, e.g., in a figure-8 network).
std::vector<std::vector<LinkShape>> lookupMinimalBendSeqs(Node_SP A, CardinalDir d0, Node_SP Z, CardinalDir d1);


//! @brief  List the possible cardinal directions from node1 to node2, if they were to be
//!         aligned non-aggressively.
CardinalDirs possibleCardinalDirections(Node_SP node1, Node_SP node2);


//! @brief  Determine the LinkShape for a given Node of degree 2.
//! @param[in] link  The link whose shape is to be determined.
//! @return  The LinkShape for the given node.
//! @throws  Runtime error if the given node is not of degree 2.
LinkShape shapeOfLink(Node_SP link);


//! @brief A data structure for managing sequences of bend types, points at which these
//! bends should occur (in a given Chain), cost of such a sequence of bends
//! (for a given Chain), and incoming and outgoing Compass directions, for non-cycles.
struct BendSequence {

    //! @brief  Basic constructor.
    //! @param[in] shapes  The sequence of LinkShapes describing the desired bends.
    BendSequence(LinkShapes &shapes) : bendTypes(shapes) {}

    //! @brief  Augmented constructor in which incoming and outgoing directions are also set.
    //! @param[in] shapes  The sequence of LinkShapes describing the desired bends.
    //! @param[in] inDir  The incoming direction.
    //! @param[in] outDir  The outgoing direction.
    BendSequence(LinkShapes &shapes, CardinalDir inDir, CardinalDir outDir)
        : bendTypes(shapes), incomingDirec(inDir), outgoingDirec(outDir) {}

    //! @brief  Report the number of bends in the sequence.
    size_t size(void) const { return bendTypes.size(); }

    //! Under bendTypes we record what shape bend we want.
    LinkShapes bendTypes;

    //! Under bendPoints we can record those places in a Chain where we want the bends to occur.
    //! The "places" are indices 0, 1, 2, 3, ... which refer to the first node in the chain,
    //! then the first edge, next node, next edge, and so on, with even numbers meaning nodes
    //! and odd numbers meaning edges.
    std::vector<size_t> bendPoints;

    double cost = 0;
    CardinalDir incomingDirec;
    CardinalDir outgoingDirec;
};

typedef std::shared_ptr<BendSequence> BendSequence_SP;
typedef std::vector<BendSequence_SP> BendSequences;

//! @brief A bend point deliberately added to a connector route, for aesthetic reasons.
struct AestheticBend {

    AestheticBend(Edge_SP edge, Node_SP bendNode, Node_SP nbrNode1, Node_SP nbrNode2)
        : edge(edge), bendNode(bendNode), nbrNode1(nbrNode1), nbrNode2(nbrNode2) {}

    //! @brief  Add the bend node to the edge.
    void addBendToEdge(void);

    Edge_SP edge;
    Node_SP bendNode;
    Node_SP nbrNode1;
    Node_SP nbrNode2;

};

typedef std::shared_ptr<AestheticBend> AestheticBend_SP;
typedef std::vector<AestheticBend_SP> AestheticBends;

typedef std::vector<std::pair<CardinalDir, CardinalDir>> ChainConfigSeq;

//! @brief A Chain is a sequence of degree-2 Nodes, possibly forming a cycle.
//!
//! This class, like everything defined in this header, is intended for use only with
//! 4-planar orthogonal layouts.
class Chain {
public:

    //! @brief  Standard constructor
    //! @param[in] G  The Graph to which the Chain belongs.
    //! @param[in] nodes  The Nodes belonging to the Chain, in order
    //! @param[in] isCycle  Set true if these Nodes form a closed cycle. Default false.
    //! @throws  Runtime error if nodes is empty.
    //!
    //! @note  We accept a deque of Nodes (rather than a vector) not because it is needed,
    //!        but because the Graph method that computes the lists of nodes forming chains uses deques.
    //! @sa Graph::getChainsAndCycles
    Chain(Graph_SP G, std::deque<Node_SP> nodes, bool isCycle=false);

    //! @brief  Get a Node according to its "place" in the Chain.
    //!
    //! Together with the getEdge function, this function allows us to have the indices
    //! 0, 1, 2, 3, ... refer to the first node in the chain, then the first edge, next node,
    //! next edge, and so on. Negative integers refer to left anchors.
    //! @param[in] i  An even integer from -2 to 2n, where n is the number of nodes in this chain.
    //! @return  Left anchor node for i == -2, self.nodes[i/2] for i from 0 to 2n-2, and right
    //!          anchor node for i == 2n.
    Node_SP getNode(int i) const;

    //! @brief  Get an Edge according to its "place" in the Chain.
    //!
    //! Together with the getNode function, this function allows us to have the indices
    //! 0, 1, 2, 3, ... refer to the first node in the chain, then the first edge, next node,
    //! next edge, and so on. Negative integers refer to left anchors.
    //! @param[in] i  An odd integer from -1 to 2n-1, where n is the number of nodes in this chain.
    //! @return  Left anchor edge for i == -1, self.edges[(i-1)/2] for i from 1 to 2n-3, and right
    //!          anchor edge for i == 2n-1.
    Edge_SP getEdge(int i) const;

    //! @brief  Compute the cost of making a given bend shape at a given position in the chain,
    //!         given the current geometry.
    //!
    //!         If this chain is a cycle, the cost takes into account that the nodes are in
    //!         clockwise order.
    //!
    //! @param[in] bendType  A bent LinkShape.
    //! @param[in] i0  A position in the chain from 0 to 2n-2 -- evens for nodes, odds for edges --
    //!                where n is the number of nodes in this chain.
    double bendCost(LinkShape bendType, size_t i0) const;

    //! @brief  Check how many nodes are in the chain.
    size_t size(void) const { return m_n; }

    //! @brief  Compute the possible bend sequences that this chain could have.
    //!
    //!         If "no bends" is a possibility, then we return a single BendSequence with
    //!         empty list of bend types.
    //! @return Vector of BendSequences indicating all the possibilites.
    BendSequences computePossibleBendSequences(void) const;

    //! @brief  For a given BendSequence, determine the best places for those bends to occur
    //!         in this Chain, and the costs of applying those constraints.
    //! @param[in, out] bendSeq  The BendSequence to be considered.
    //!                          Sequence of best places to make the bends is recorded in here,
    //!                          along with the total cost.
    void evaluateBendSequence(BendSequence_SP bendSeq) const;

    //! @brief  Determine the direction in which each edge in this chain should be configured,
    //!         in order to enforce a given bend sequence.
    //! @param[in] bendSeq  The desired bend sequence. Its bendpoints are indices into this
    //! Chain's sequence of nodes AND edges -- thus even indices for nodes and odd indices for
    //! edges. Its corresponding bendtypes are the types of bends that should occur at those indices.
    //! @return  A "chain configuration sequence," which looks like
    //!                                 [ c0, c1, ..., cm-1 ]
    //! where m is the number of edges to be configured, which is n - 1 if this is not a cycle,
    //! and n if it is -- n the number of nodes in the chain -- and each ci is a pair of CardinalDirs.
    //!
    //! When ci == [ d, d ], then edge i is simply to be configured in direction d.
    //! When ci == [d1, d2] with d1 != d2, then edge i is to be replaced by a bend point, which we go
    //! into in direction d1, and come out of in direction d2.
    ChainConfigSeq writeConfigSeq(BendSequence_SP bendSeq) const;

    //! @brief  Give this chain an orthogonal configuration best fitting its present geometric shape.
    //!
    //!         This means we put the bend points in the most natural places, including the possibility
    //!         that they go where edges are (meaning a new bend point is constructed).
    //!
    //!         Thus, constraints at least are added to the underlying Graph's SepMatrix.
    //!         New Nodes (bend points) may also be added.
    void takeShapeBasedConfiguration(void);

    //! @brief  Add any aesthetic bends that were chosen during shape-based configuration, to the
    //!         Edges to which they belong.
    void addAestheticBendsToEdges(void);

    //! For our search procedures we need a size_t that effectively means "no position".
    //! For this we follow the practice of std::string::find, using npos = -1, the largest
    //! possible size_t. We can certainly not work on networks with that many nodes!
    static const size_t npos;

private:

    //! @brief  Choose a locally optimal point for creating a bend in the chain.
    //! @param[in] i0  A position in the Chain.
    //! @param[in] bendType  The bent LinkShape to be formed.
    //! @param[in] remaining  How many more points we must choose /after/ this one.
    //! @param[out] i1  The position to be chosen.
    //! @return The cost of the chosen position.
    //!
    //! We choose a locally optimal point i1 /at or after/ position i0, at which to create
    //! the given bend type. Optimality is understood in terms of cost returned by the bendCost
    //! function. However, the "locality" of the search means that once we have chosen a candidate
    //! point, we will accept it as soon as the next tested point costs more.
    //!
    //! Thus, we do not necessarily explore all points. For that see Chain::globalOptimalPoint.
    //!
    //! If remaining == r and there are at least r positions left after i0 in the chain,
    //! then we return an i1 which has at least r points left after it; if not, then we just
    //! return i1 = i0.
    double nextLocalOptimalPoint(size_t i0, LinkShape bendType, size_t remaining, size_t &i1) const;

    //! @brief  Choose an optimal point for creating a bend in the chain.
    //! @param[in] bendType  The bent LinkShape to be formed.
    //! @param[out] i1  The position to be chosen.
    //! @param[in] beginAt  A position in the chain at which to begin search.
    //! @return The cost of the chosen position.
    //!
    //! We choose an optimal point /at or after/ position beginAt, at which to create
    //! the given bend type. Optimality means minimal cost, from the bendCost function.
    //!
    //! If there are no points left after beginAt, we return None.
    double globalOptimalPoint(LinkShape bendType, size_t &i1, size_t beginAt = 0) const;

    //! @brief  Check how many potential bend points we have. This is equal to the number of
    //!         nodes in the chain, plus the number of internal edges.
    size_t getNumPotentialBendPlaces(void) const { size_t M = 2*m_n - 1; if (m_isCycle) ++M; return M; }

    Graph_SP m_graph;
    size_t m_n;
    Nodes m_nodes;
    bool m_isCycle;
    LinkShapes m_linkShapes;
    Edges m_edges;
    Node_SP m_anchorNodeLeft = nullptr;
    Edge_SP m_anchorEdgeLeft = nullptr;
    Node_SP m_anchorNodeRight = nullptr;
    Edge_SP m_anchorEdgeRight = nullptr;
    Edge_SP m_returnEdge = nullptr;
    AestheticBends m_aestheticBends;

};

typedef std::shared_ptr<Chain> Chain_SP;
typedef std::vector<Chain_SP> Chains;

//! @brief  Convenience method to build all the chains and cycles in a graph.
Chains buildAllChainsInGraph(std::shared_ptr<dialect::Graph> graph);



} // namespace dialect

#endif // DIALECT_CHAINS_H
