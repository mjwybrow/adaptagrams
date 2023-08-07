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

#include <deque>
#include <iterator>
#include <string>
#include <map>
#include <algorithm>
#include <memory>
#include <utility>
#include <sstream>
#include <limits>
#include <functional>

#include "libvpsc/assertions.h"
#include "libavoid/libavoid.h"

#include "libdialect/ortho.h"
#include "libdialect/graphs.h"
#include "libdialect/constraints.h"
#include "libdialect/commontypes.h"
#include "libdialect/trees.h"

using namespace dialect;

using std::map;
using std::vector;
using std::string;

using Avoid::Point;

Tree::Tree(Graph_SP G, Node_SP root)
    : m_graph(G),
      m_root(root),
      m_depth(0),
      m_breadth(0),
      m_isSymmetric(false),
      m_lb(0),
      m_ub(0),
      m_boundaryTight(true),
      m_growthDir(CardinalDir::NORTH)
{
    // As we go, we'll build a multimap from ranks to Nodes of that rank.
    // At the end, we'll use this to build our lookup, m_nodesByRank.
    std::multimap<unsigned, Node_SP> nodesByRank_mm;
    // Compute ranks, leaves, and breadth.
    // Start by setting rank 0 for the root node.
    nodesByRank_mm.emplace(0, m_root);
    m_rankByNodeID.emplace(m_root->id(), 0);
    m_depth = 1;
    // Now do BFS to identify all nodes of the tree.
    std::deque<Node_SP> bfs_queue{m_root};
    Node_SP node;
    while (!bfs_queue.empty()) {
        // Take from the front of the queue.
        node = bfs_queue.front();
        bfs_queue.pop_front();
        unsigned id = node->id();
        // Ensure we're not trying to construct a tree on
        // a graph that contains a cycle, by checking that we
        // have not yet encountered this node.
        COLA_ASSERT(m_nodes.count(id) == 0);
        m_nodes.insert({id, node});
        // The current node's rank should already be defined.
        unsigned rank = m_rankByNodeID[id];
        // Get children
        Nodes children = node->getChildren();
        // If there are no children, then the current Node is a leaf.
        if (children.empty()) {
            m_leafIDs.emplace(id);
        } else {
            // There are children, so update the depth.
            if (rank + 2 > m_depth) m_depth = rank + 2;
        }
        // Assign parent to any children, and add them to the queue.
        for (Node_SP c : children) {
            m_parents.emplace(c->id(), node);
            nodesByRank_mm.emplace(rank + 1, c);
            m_rankByNodeID.emplace(c->id(), rank + 1);
            bfs_queue.emplace_back(c);
        }
    }
    // After the search, the depth of the tree is already computed.
    // Now we can build our lookup for nodes by rank, and compute the
    // breadth as we go.
    m_nodesByRank.reserve(m_depth);
    m_leavesByRank.reserve(m_depth);
    m_nonleavesByRank.reserve(m_depth);
    // We will scan over our multimap, using two pointers.
    auto ptr1 = nodesByRank_mm.cbegin();
    auto ptr2 = ptr1;
    auto end = nodesByRank_mm.cend();
    // We work by rank. This works since the multimap is ordered.
    for (unsigned r = 0; r < m_depth; ++r) {
        size_t n = 0;
        // Find the range of nodes of rank r. Count them as we go.
        while (ptr2 != end && (*ptr2).first == r) {
            ptr2 = std::next(ptr2);
            ++n;
        }
        // Now initialize vectors and reserve space.
        // For the leaves and nonleaves it is more space than is needed, but
        // we guess this is the more efficient approach.
        Nodes rank_r, leaves_r, nonleaves_r;
        rank_r.reserve(n);
        leaves_r.reserve(n);
        nonleaves_r.reserve(n);
        // Copy Nodes into the vectors.
        while (ptr1 != ptr2) {
            Node_SP u = (*ptr1).second;
            rank_r.push_back(u);
            if (m_leafIDs.count(u->id()) > 0) {
                leaves_r.push_back(u);
            } else {
                nonleaves_r.push_back(u);
            }
            ptr1 = std::next(ptr1);
        }
        // Save the vectors
        m_nodesByRank.push_back(rank_r);
        m_leavesByRank.push_back(leaves_r);
        m_nonleavesByRank.push_back(nonleaves_r);
        // Update the breadth.
        if (n > m_breadth) m_breadth = n;
    }
    // Clear and initialize rank bounds.
    clearRankBounds();
}

void Tree::symmetricLayout(CardinalDir growthDir, double nodeSep, double rankSep, bool convexOrdering) {
    m_growthDir = growthDir;
    // Initialise root position to zero.
    m_root->setCentre(0, 0);
    // Initialise rank bounds to zero.
    clearRankBounds();
    // Initialise rank-0 bounds for root node.
    dimensions rootDims = m_root->getDimensions();
    double half = Compass::isVertical((CompassDir) growthDir) ? rootDims.first/2.0 : rootDims.second/2.0;
    m_lb = -half;
    m_ub = half;
    m_boundsByRank[0][0] = m_lb;
    m_boundsByRank[0][1] = m_ub;
    // Leaves are simple:
    if (m_depth == 1) {
        m_isSymmetric = true;
        return;
    }
    // Proceed for nonleaves:
    Trees cTrees = getCTrees();
    // Layout the C-Trees recursively.
    for (Tree_SP c : cTrees) {
        c->symmetricLayout(growthDir, nodeSep, rankSep);
    }
    // Sort the C-Trees into isomorphism classes.
    map<string, Trees> classes;
    for (Tree_SP c : cTrees) {
        string isomstr = c->computeIsomString();
        classes[isomstr].push_back(c);
    }
    // Now form a deque of all isom. strings.
    // (Why a deque? We need to be able to sort, push front, and erase an element from the middle.
    //  If not for the sorting, we would prefer a list, but that does not have random access iterators,
    //  so cannot be sorted.)
    std::deque<string> isomStrings;
    isomStrings.resize(classes.size());
    std::transform(classes.cbegin(), classes.cend(), isomStrings.begin(),
        [](const std::pair<string, Trees> &p) -> string { return p.first; }
    );
    // Now sort the isom strings.
    // If using convex ordering, use the following rule.
    // (If not, use the opposite rule.)
    // Class A comes before (is "less than") class B if,
    // for any trees tA, tB belonging to these classes, resp.,
    // (i) tA is broader than tB, or
    // (ii) they are the same breadth but tA is deeper than tB, or
    // (iii) they are the same breadth and depth, but A's isom string
    //       comes before B's, lexicographically.
    // The first two options reflect our ideas about how C-trees should
    // be positioned in the layout; the third option is only there to
    // ensure the sorting is deterministic.
    std::sort(
        isomStrings.begin(), isomStrings.end(),
        [&classes, &convexOrdering](const string &a, const string &b) -> bool {
            Trees A = classes[a], B = classes[b];
            Tree_SP tA = A[0], tB = B[0];
            unsigned bA = tA->m_breadth, bB = tB->m_breadth,
                     dA = tA->m_depth,   dB = tB->m_depth;
            if (bA > bB) {
                return convexOrdering;
            } else if (bB > bA) {
                return !convexOrdering;
            } else if (dA > dB) {
                return convexOrdering;
            } else if (dB > dA) {
                return !convexOrdering;
            } else {
                return a < b;
            }
        }
    );
    // How many classes have odd order?
    // We also record the isom. string for the odd-order class most recently
    // noted. This works because we only need the string in case there is
    // exactly one odd-order class.
    string oddOrderIsomStr;
    size_t numOddOrder = 0;
    for (auto p : classes) {
        if (p.second.size() % 2 == 1) {
            ++numOddOrder;
            oddOrderIsomStr = p.first;
        }
    }
    // Now we can determine whether the layout is going to be symmetric or not,
    // and whether it's going to need to have a central c-tree.
    bool haveCentralTree = false;
    // If there are no odd-order classes, then we are symmetric.
    if (numOddOrder == 0) {
        m_isSymmetric = true;
    }
    // If there are two or more odd-order classes, then we are not symmetric.
    else if (numOddOrder > 1) {
        m_isSymmetric = false;
    }
    // Else there is exactly one odd-order class.
    // In this case we are symmetric if and only if (any representative of) the one
    // odd-order class is symmetric.
    else {
        m_isSymmetric = classes[oddOrderIsomStr][0]->m_isSymmetric;
        // For symmetric layout, the trees of odd-order class need to go in the centre.
        // So we put them first in the list, since we work our way outward from the centre
        // when placing the trees.
        haveCentralTree = true;
        auto it = isomStrings.begin();
        while (*it != oddOrderIsomStr) ++it;
        isomStrings.erase(it);
        isomStrings.push_front(oddOrderIsomStr);
    }
    // Now place the c-trees alternating around the centre.
    // We use a boolean to control the alternation.
    bool positiveNext = true;
    // Another boolean manages central tree placement.
    bool mustPlaceCentralTree = haveCentralTree;
    // We will need to do translations. We start with a base translation vector.
    Point baseTrans(0, 0);
    switch(growthDir) {
    case CardinalDir::EAST:
        baseTrans.x = rankSep;
        break;
    case CardinalDir::SOUTH:
        baseTrans.y = rankSep;
        break;
    case CardinalDir::WEST:
        baseTrans.x = -rankSep;
        break;
    case CardinalDir::NORTH:
        baseTrans.y = -rankSep;
        break;
    }
    // Now iterate over the isomorphism classes of C-Trees.
    for (string isom : isomStrings) {
        Trees C = classes[isom];
        for (Tree_SP t : C) {
            if (mustPlaceCentralTree) {
                // Tree t is to be placed centrally.
                t->translate(baseTrans);
                // Update own bounds based on Tree t's bounds.
                double lb = 0, ub = 0;
                for (unsigned r = 1; r <= t->m_depth; ++r) {
                    // Tree t's rank r - 1 lies on our own rank r.
                    auto tb = t->m_boundsByRank[r-1];
                    double tlb = tb[0], tub = tb[1];
                    m_boundsByRank[r][0] = tlb;
                    m_boundsByRank[r][1] = tub;
                    if (tlb < lb) lb = tlb;
                    if (tub > ub) ub = tub;
                }
                m_lb = lb;
                m_ub = ub;
                // Now central tree is placed, so clear the flag.
                mustPlaceCentralTree = false;
            } else {
                // Set up based on whether we're on the positive or the negative side.
                size_t a = positiveNext ? 1 : 0,
                       b = positiveNext ? 0 : 1;
                double rootPos = positiveNext ? std::numeric_limits<double>::min() : std::numeric_limits<double>::max(),
                       extreme = rootPos;
                if (!positiveNext) t->flip();
                // Consider the position where each rank would like the root to go.
                // Take the extreme, according to whether we're on the positive or negative side.
                for (unsigned r = 0; r < t->m_depth; ++r) {
                    double candidate = getBounds(r + 1, nodeSep)[a] - t->getBounds(r, nodeSep)[b];
                    rootPos = positiveNext ? std::max(rootPos, candidate) : std::min(rootPos, candidate);
                }
                // Translate the tree.
                Point trans = Compass::isVertical((CompassDir) growthDir) ?
                                Point(rootPos, baseTrans.y) : Point(baseTrans.x, rootPos);
                t->translate(trans);
                // Update upper bounds if on positive side, lower if on negative side.
                for (unsigned r = 0; r < m_depth; ++r) {
                    double tb;
                    if (1 <= r && r <= t->m_depth) {
                        // Again, Tree t's rank r - 1 lies on our own rank r.
                        tb = t->m_boundsByRank[r-1][a];
                        m_boundsByRank[r][a] = tb;
                    } else {
                        tb = m_boundsByRank[r][a];
                    }
                    extreme = positiveNext ? std::max(extreme, tb) : std::min(extreme, tb);
                }
                if (a == 1) {
                    m_ub = extreme;
                } else {
                    m_lb = extreme;
                }
                // Flip to other side for next iteration, if any.
                positiveNext = !positiveNext;
            }
        }
    }
}

void Tree::flip(void) {
    // We filp the tree around the y-axis for NORTH and SOUTH growth,
    // around the x-axis for EAST and WEST growth.
    if (Compass::isVertical((CompassDir) m_growthDir)) {
        for (auto p : m_nodes) {
            Point q = p.second->getCentre();
            p.second->setCentre(-q.x, q.y);
        }
    } else {
        for (auto p : m_nodes) {
            Point q = p.second->getCentre();
            p.second->setCentre(q.x, -q.y);
        }
    }
    // Bounds are swapped and negated.
    double l = -m_ub, u = -m_lb;
    m_lb = l;
    m_ub = u;
    // Flip per-rank bounds
    for (unsigned r = 0; r < m_depth; ++r) {
        l = -m_boundsByRank[r][1];
        u = -m_boundsByRank[r][0];
        m_boundsByRank[r][0] = l;
        m_boundsByRank[r][1] = u;
    }
}

void Tree::translate(Avoid::Point vect) {
    // Translate Nodes.
    for (auto p : m_nodes) {
        Point q = p.second->getCentre();
        p.second->setCentre(q.x + vect.x, q.y + vect.y);
    }
    // Adjust rank bounds
    double disp = Compass::isVertical((CompassDir) m_growthDir) ? vect.x : vect.y;
    m_lb += disp;
    m_ub += disp;
    for (unsigned r = 0; r < m_depth; ++r) {
        m_boundsByRank[r][0] += disp;
        m_boundsByRank[r][1] += disp;
    }
}

void Tree::rotate(CardinalDir dg) {
    // If we already have the desired growth direction, there's nothing to do.
    if (dg == m_growthDir) return;
    // Otherwise, rotate.
    PlaneMap rot = Compass::getRotationFunction(m_growthDir, dg);
    for (auto p : m_nodes) {
        Node_SP u = p.second;
        u->applyPlaneMap(rot);
    }
    // And record the new growth direction.
    m_growthDir = dg;
}

void Tree::rotateGrowthDirCW(unsigned quarterTurns) {
    unsigned q = quarterTurns % 4;
    for (unsigned i = 0; i < q; ++i) m_growthDir = Compass::cardRotateCw90(m_growthDir);
}

void Tree::clearRankBounds(void) {
    m_boundsByRank.clear();
    m_boundsByRank.reserve(m_depth);
    for (unsigned r = 0; r < m_depth; ++r) {
        m_boundsByRank.push_back(std::vector<double>{0, 0});
    }
}

std::vector<double> Tree::getBounds(unsigned rank, double nodeSep) const {
    std::vector<double> bounds(2);
    // Start with basic bounds, based on whether we're using the tight boundary setting.
    if (m_boundaryTight) {
        bounds = m_boundsByRank[rank];
    } else {
        bounds[0] = m_lb;
        bounds[1] = m_ub;
    }
    // Now pad with the given nodeSep value.
    bounds[0] -= nodeSep;
    bounds[1] += nodeSep;
    return bounds;
}

Trees Tree::getCTrees(void) const {
    // There's one C-Tree for each child of the root node.
    // (It is the subtree rooted there.)
    Trees ctrees;
    Nodes children = m_root->getChildren();
    ctrees.reserve(children.size());
    for (Node_SP c : children) {
        Tree_SP ct = std::make_shared<Tree>(m_graph, c);
        ctrees.push_back(ct);
    }
    return ctrees;
}

std::string Tree::computeIsomString(void) const {
    // We compute a string which uniquely represents the isomorphism class
    // of this Tree. This is the core idea from Manning & Atallah 1985.
    map<id_type, unsigned> isomNumber;
    map<id_type, vector<unsigned>> isomTuple;
    map<id_type, string> isomTupleString;
    vector<string> levelIsomStrings;
    // Assign isomNumber 0 to all leaves.
    for (id_type id : m_leafIDs) isomNumber[id] = 0;
    // Initialise L to be a list of the deepest leaves.
    Nodes L = m_leavesByRank[m_depth-1];
    // Compute the isomstring for each level.
    for (unsigned j = 2; j <= m_depth; ++j) {
        unsigned r = m_depth - j;
        Nodes N = m_nonleavesByRank[r];
        for (Node_SP u : N) {
            isomTuple[u->id()] = vector<unsigned>();
        }
        // For each leaf v in L, insert its number into
        // the tuple of its parent node.
        for (Node_SP v : L) {
            id_type v_id = v->id();
            Node_SP p = m_parents.at(v_id);
            unsigned n = isomNumber[v_id];
            isomTuple[p->id()].push_back(n);
        }
        // Create strings for isomTuples.
        for (Node_SP u : N) {
            id_type u_id = u->id();
            vector<unsigned> tuple = isomTuple[u_id];
            std::sort(tuple.begin(), tuple.end());
            // Thanks to https://stackoverflow.com/a/1430774
            std::stringstream ss;
            for (size_t i = 0; i < tuple.size(); ++i) {
                if (i != 0) ss << ",";
                ss << tuple[i];
            }
            isomTupleString[u_id] = ss.str();
        }
        // Sort the nonleaves of rank r by tuple.
        std::sort(N.begin(), N.end(),
            [&isomTupleString](const Node_SP &a, const Node_SP &b) -> bool {
                return isomTupleString[a->id()] < isomTupleString[b->id()];
            }
        );
        // Now we can write the isom string for this level.
        std::stringstream ss;
        for (size_t i = 0; i < N.size(); ++i) {
            if (i != 0) ss << ";";
            ss << isomTupleString[N[i]->id()];
        }
        levelIsomStrings.push_back(ss.str());
        // Compute next L.
        // Start with the leaves of rank r.
        L = m_leavesByRank[r];
        // Sort nodes in N by tuple.
        map<string, Nodes> nodesByTuple;
        for (Node_SP u : N) {
            string t = isomTupleString[u->id()];
            nodesByTuple[t].push_back(u);
        }
        // Iterate over the classes, in lexicographic order on the tuple strings.
        unsigned k = 0;
        for (auto p : nodesByTuple) {
            Nodes M = p.second;
            // The Nodes in each class get isomNumber according to the ordering.
            // They are also added to the next list L.
            for (Node_SP u : M) {
                isomNumber[u->id()] = k + 1;
                L.push_back(u);
            }
        }
    }
    // Join the level strings for the full tree string.
    std::stringstream ss;
    for (size_t i = 0; i < levelIsomStrings.size(); ++i) {
        if (i != 0) ss << ":";
        ss << levelIsomStrings[i];
    }
    string treeIsomString = ss.str();
    return treeIsomString;
}



std::string Tree::repr(void) const {
    std::string s = "Tree:";
    // All nodes:
    for (auto p : m_nodes) {
        s += string_format(" %d", p.first);
    }
    s += "\n";
    // Ranks:
    for (unsigned r = 0; r < m_depth; ++r) {
        s += string_format("Rank %d:", r);
        Nodes rank = m_nodesByRank[r];
        std::sort(rank.begin(), rank.end(),
            [](const Node_SP &a, const Node_SP &b) -> bool {
                return a->id() < b->id();
            }
        );
        for (Node_SP u : rank) {
            s += string_format(" %d", u->id());
            if (m_leafIDs.count(u->id()) > 0) s += "^";
        }
        s += "\n";
    }
    s += "Isom: " + computeIsomString() + "\n";
    /*
    // Leaves:
    for (unsigned r = 0; r < m_depth; ++r) {
        s += string_format("Leaves %d:", r);
        Nodes rank = m_leavesByRank[r];
        std::sort(rank.begin(), rank.end(),
            [](const Node_SP &a, const Node_SP &b) -> bool {
                return a->id() < b->id();
            }
        );
        for (Node_SP u : rank) {
            s += string_format(" %d", u->id());
        }
        s += "\n";
    }
    // Nonleaves:
    for (unsigned r = 0; r < m_depth; ++r) {
        s += string_format("Nonleaves %d:", r);
        Nodes rank = m_nonleavesByRank[r];
        std::sort(rank.begin(), rank.end(),
            [](const Node_SP &a, const Node_SP &b) -> bool {
                return a->id() < b->id();
            }
        );
        for (Node_SP u : rank) {
            s += string_format(" %d", u->id());
        }
        s += "\n";
    }
    */
    return s;
}

Node_SP Tree::buildRootlessBox(CardinalDir growthDir) const {
    // Tree should already have a layout.
    // So we begin by computing the box for the Tree as laid out.
    BoundingBox bb = m_graph->getBoundingBox();
    dimensions rootDims = m_root->getDimensions();
    double rootW = rootDims.first,
           rootH = rootDims.second,
           rootHalfOblongness = (rootW - rootH)/2.0,
           x = bb.x,
           y = bb.y,
           w = bb.w(),
           h = bb.h();
    switch(m_growthDir) {
    // If growing East/West then the root node's width is
    // subracted from that of the box. In East case only,
    // ULC of box shifts right by root node's width.
    case CardinalDir::EAST:
        x += rootW;
        // Do not break. Flow through to WEST case...
    case CardinalDir::WEST:
        w -= rootW;
        break;
    // If growing South/North then the root node's height is
    // subracted from that of the box. In South case only,
    // ULC of box shifts down by root node's height.
    case CardinalDir::SOUTH:
        y += rootH;
        // Do not break. Flow through to NORTH case...
    case CardinalDir::NORTH:
        h -= rootH;
        break;
    }
    // Now we can compute the centre of the box.
    double u = x + w/2.0,
           v = y + h/2.0;
    // So far everything has been based on this Tree's own growth
    // direction. Now adjust for the desired growth direction, in
    // case it is different.
    if (growthDir != m_growthDir) {
        // Rotate centre.
        Point c = Compass::getRotationFunction(m_growthDir, growthDir)(Point(u, v));
        u = c.x;
        v = c.y;
        // If the two growth directions are not in the same dimension...
        if (!Compass::sameDimension(m_growthDir, growthDir)) {
            // ...the dimensions swap...
            std::swap(w, h);
            // ...and if the root node is oblong, then the box needs to
            // be shifted in the growth direction.
            switch(growthDir) {
            case CardinalDir::EAST:
                u += rootHalfOblongness;
                break;
            case CardinalDir::WEST:
                u -= rootHalfOblongness;
                break;
            case CardinalDir::SOUTH:
                v -= rootHalfOblongness;
                break;
            case CardinalDir::NORTH:
                v += rootHalfOblongness;
            }
        }
    }
    // Build Node and return it.
    Node_SP boxNode = Node::allocate();
    boxNode->setCentre(u, v);
    boxNode->setDims(w, h);
    return boxNode;
}

void Tree::addNetworkToRoutingAdapter(RoutingAdapter &ra, TreeRoutingType trt, Graph_SP core) {
    // Add nodes.
    NodesById nodes(m_nodes);
    // If a core graph is given, omit the root node, since in this case the client is interested in
    // reattaching this tree to the core, where there is already a root node.
    if (core != nullptr) nodes.erase(m_root->id());
    ra.addNodes(nodes);
    // Prepare to set connection directions.
    // Although the Tree could be growing in any cardinal direction, we envision the case of a north-growing
    // or upward-growing Tree. Our variable names are based on this image.
    CardinalDir up = m_growthDir,
                down = Compass::cardFlip(up);
    Avoid::ConnDirFlags upperDirs = Compass::libavoidConnDirs.at(down),
                        ordinalRootDirs = Avoid::ConnDirAll ^ upperDirs,
                        lowerDirs = trt >= TreeRoutingType::MONOTONIC ? ordinalRootDirs : Compass::libavoidConnDirs.at(up);
    EdgeConnDirs dirs{lowerDirs, upperDirs};
    EdgeConnDirs rev{upperDirs, lowerDirs};
    EdgeConnDirs rootDirs{ordinalRootDirs, upperDirs};
    EdgeConnDirs rootRev{upperDirs, ordinalRootDirs};
    // In the case of a peeled tree which is to be reattached to a core graph, if the tree has been
    // given ordinal placement direction then its root node is in a different position than the root
    // node belonging to the core. So in order for the connectors in the router to get the proper endpts,
    // we temporarily move the root node to the position of the core root.
    Node_SP altRoot = core == nullptr ? m_root : core->getNode(m_root->id());
    Point altRootPt = altRoot->getCentre(),
          rootPt = m_root->getCentre();
    m_root->setCentre(altRootPt.x, altRootPt.y);
    // Determine whether we use permissive directions for connection to root node.
    // There are two cases in which that is so: (1) we are using the MONOTONIC routing type, or
    // (2) it is the CORE_ATTACHMENT type, and the root node has a single child and is transversely displaced.
    bool permissiveRootDirs = (
        trt >= TreeRoutingType::MONOTONIC ||
        (trt == TreeRoutingType::CORE_ATTACHMENT && rootPt != altRootPt && m_nodesByRank[1].size() == 1)
    );
    // Now we can iterate over the Edges and set up the ConnDirs.
    EdgesById edges = m_graph->getEdgeLookup();
    EdgeConnDirsById connDirs;
    id_type rid = m_root->id();
    for (auto p : edges) {
        // Determine the ranks of the source and target Nodes of the Edge, so that we can set the
        // conndirs in the right order.
        Edge_SP e = p.second;
        id_type sid = e->getSourceEnd()->id(),
                tid = e->getTargetEnd()->id();
        unsigned rs = m_rankByNodeID[sid],
                 rt = m_rankByNodeID[tid];
        EdgeConnDirs d;
        if ( permissiveRootDirs && (rid == sid || rid == tid) ) {
            d = rs < rt ? rootDirs : rootRev;
        } else {
            d = rs < rt ? dirs : rev;
        }
        connDirs.insert({p.first, d});
    }
    // Add edges to router with conn dirs.
    ra.addEdges(edges, &connDirs);
    // Retore root position.
    m_root->setCentre(rootPt.x, rootPt.y);
}

void Tree::addNetwork(Graph &G, NodesById &treeNodes, EdgesById &treeEdges) {
    // Add Nodes.
    NodesById G_nodes = G.getNodeLookup();
    auto it = m_nodes.begin();
    auto jt = G_nodes.begin();
    while (it != m_nodes.end() && jt != G_nodes.end()) {
        auto p = *it;
        auto q = *jt;
        id_type i = p.first,
                j = q.first;
        if (i > j) ++jt;
        else {
            if (i < j) {
                // In this case the Tree has a Node that does not have a matching ID in the graph G.
                // So we can add this Node to G.
                G.addNode(p.second);
                treeNodes.insert(p);
            }
            ++it;
        }
    }
    // In case we fell off the end of G_nodes first:
    while (it != m_nodes.end()) {
        auto p = *it;
        G.addNode(p.second);
        ++it;
    }
    // Add Edges.
    for (auto p : m_graph->getEdgeLookup()) {
        Edge_SP e = p.second;
        G.addEdge(e, false);
        treeEdges.insert(p);
    }
}

void Tree::addConstraints(Graph &G, bool alignRoot) {

    // Prepare functions and constants based on our growth direction.
    bool isVertical = Compass::isVerticalCard(m_growthDir);
    // Function to compare Nodes by their transverse coordinate:
    std::function<bool(Node_SP, Node_SP)> transCoordCmp = isVertical ?
                [](const Node_SP &a, const Node_SP &b)->bool{ return a->getCentre().x < b->getCentre().x; } :
                [](const Node_SP &a, const Node_SP &b)->bool{ return a->getCentre().y < b->getCentre().y; } ;
    // Funciton to compare Nodes by their axial dimension:
    std::function<bool(Node_SP, Node_SP)> axialMeasureCmp = isVertical ?
                [](const Node_SP &a, const Node_SP &b)->bool{ return a->getDimensions().second < b->getDimensions().second; } :
                [](const Node_SP &a, const Node_SP &b)->bool{ return a->getDimensions().first < b->getDimensions().first; } ;
    // The direction in which to separate Nodes belonging to a common rank:
    CardinalDir intraRankSepDir = isVertical ? CardinalDir::EAST : CardinalDir::SOUTH;
    // The direction in which to separate Nodes belonging to neighbouring ranks:
    SepDir interRankSepDir = lateralWeakening(cardinalDirToSepDir(m_growthDir));
    // The coordinate to be equated when aligning Nodes with centre child Nodes:
    vpsc::Dim centreAlignCoord = isVertical ? vpsc::XDIM : vpsc::YDIM;

    SepMatrix &matrix = G.getSepMatrix();

    // Align and separate all nodes within a rank.
    // As we work, for each rank we want to note a "tallest node," i.e. one of maximal axial dimension.
    Nodes tallestNodes;
    tallestNodes.reserve(m_depth);
    for (Nodes rank : m_nodesByRank) {
        // Record a tallest node.
        tallestNodes.push_back(*std::max_element(rank.begin(), rank.end(), axialMeasureCmp));
        // Separate nodes within the rank.
        // First sort by transverse coordinate.
        std::sort(rank.begin(), rank.end(), transCoordCmp);
        // Now we can set the separations.
        for (auto it = rank.begin(); it != std::prev(rank.end()); ++it) {
            auto jt = std::next(it);
            Node_SP u = *it,
                    v = *jt;
            id_type id1 = u->id(),
                    id2 = v->id();
            matrix.setCardinalOP(id1, id2, intraRankSepDir);
        }
    }

    // Rank separation.
    // For each pair of neighbouring ranks, we separate the tallest nodes in those ranks.
    for (auto it = tallestNodes.begin(); it != std::prev(tallestNodes.end()); ++it) {
        auto jt = std::next(it);
        Node_SP u = *it,
                v = *jt;
        id_type id1 = u->id(),
                id2 = v->id();
        matrix.addSep(id1, id2, GapType::BDRY, interRankSepDir, SepType::INEQ, 0.0);
    }

    // Alignments of parents with central children.
    for (auto p : m_nodes) {
        Node_SP u = p.second;
        // Skip the root node if we're not aligning it with its central child (if any).
        if (!alignRoot && u == m_root) continue;
        // Otherwise find the central child, if any.
        Nodes children = u->getChildren();
        size_t n = children.size();
        // If even number of children, then none is central.
        if (n % 2 == 0) continue;
        // Otherwise odd, so sort by transverse coord and take middle one.
        std::sort(children.begin(), children.end(), transCoordCmp);
        size_t m = (n - 1)/2;
        Node_SP c = children[m];
        id_type id1 = u->id(),
                id2 = c->id();
        matrix.alignByEquatedCoord(id1, id2, centreAlignCoord);
    }
}

void Tree::addBufferNodesAndConstraints(Graph &G, NodesById &bufferNodes) {
    SepMatrix &matrix = G.getSepMatrix();
    // We need a function to take a new buffer node bn, and a neighbour of that node nbr, and (a) add bn to Graph G,
    // (b) add bn to our running lookup of bufferNodes, and (c) set a fixed relative constraint between bn and nbr.
    std::function<void(Node_SP&, Node_SP&)> addBufferNode = [&bufferNodes, &G, &matrix](Node_SP &bn, Node_SP &nbr)->void{
        G.addNode(bn);
        id_type bnID = bn->id(),
                nbrID = nbr->id();
        bufferNodes.insert({bnID, bn});
        Point c0 = bn->getCentre(),
              c1 = nbr->getCentre();
        double dx = c1.x - c0.x,
               dy = c1.y - c0.y;
        matrix.addFixedRelativeSep(bnID, nbrID, dx, dy);
    };
    // Set pad amount.
    double pad = G.getIEL()/4.0;
    // Pads on "top" of leaves (picturing NORTH growth):
    auto it = m_nodes.begin();
    auto jt = m_leafIDs.begin();
    while (it != m_nodes.end() && jt != m_leafIDs.end()) {
        auto p = *it;
        id_type i = p.first,
                j = *jt;
        if (i > j) ++jt;
        else {
            if (i==j) {
                Node_SP leaf = p.second;
                BoundingBox b = leaf->getBoundingBox();
                Node_SP bn = Node::allocate();
                switch(m_growthDir) {
                    case CardinalDir::EAST: bn->setBoundingBox(b.X, b.X + pad, b.y, b.Y); break;
                    case CardinalDir::SOUTH: bn->setBoundingBox(b.x, b.X, b.Y, b.Y + pad); break;
                    case CardinalDir::WEST: bn->setBoundingBox(b.x - pad, b.x, b.y, b.Y); break;
                    case CardinalDir::NORTH: bn->setBoundingBox(b.x, b.X, b.y - pad, b.y); break;
                }
                addBufferNode(bn, leaf);
            }
            ++it;
        }
    }
    // Pads on outsides of ranks:
    bool isVertical = Compass::isVerticalCard(m_growthDir);
    // Function to compare Nodes by their transverse coordinate:
    std::function<bool(Node_SP, Node_SP)> transCoordCmp = isVertical ?
                [](const Node_SP &a, const Node_SP &b)->bool{ return a->getCentre().x < b->getCentre().x; } :
                [](const Node_SP &a, const Node_SP &b)->bool{ return a->getCentre().y < b->getCentre().y; } ;
    for (auto it = std::next(m_nodesByRank.begin()); it != m_nodesByRank.end(); ++it) {
        Nodes rank = *it;
        std::sort(rank.begin(), rank.end(), transCoordCmp);
        Node_SP first = rank.front(),
                last  = rank.back();
        BoundingBox a = first->getBoundingBox(),
                    b = last->getBoundingBox();
        // We make two buffer nodes, one for each end of the rank.
        Node_SP bn0 = Node::allocate(),
                bn1 = Node::allocate();
        if (isVertical) {
            bn0->setBoundingBox(a.x - pad, a.x, a.y, a.Y);
            bn1->setBoundingBox(b.X, b.X + pad, b.y, b.Y);
        } else {
            bn0->setBoundingBox(a.x, a.X, a.y - pad, a.y);
            bn1->setBoundingBox(b.x, b.X, b.Y, b.Y + pad);
        }
        addBufferNode(bn0, first);
        addBufferNode(bn1, last);
    }
}

void Tree::padCorrespNonRootNodes(Graph &H, double dw, double dh) {
    NodesById rootIgnore = {{m_root->id(), m_root}};
    m_graph->padCorrespNodes(H, dw, dh, rootIgnore);
}
