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
#include <vector>

#include "libvpsc/assertions.h"

#include "libdialect/commontypes.h"
#include "libdialect/peeling.h"
#include "libdialect/trees.h"

using namespace dialect;

using std::dynamic_pointer_cast;
using std::vector;
using std::make_shared;

id_type PeeledNode::nextTreeSerialNumber = 0;

void Stem::addSelfToGraph(Graph &H) {
    // First make sure the Graph has PeeledNodes representing the
    // plain Nodes at the ends of this Stem.
    Node_SP plain_leaf(m_plain_leaf);
    Node_SP plain_root(m_plain_root);
    PeeledNode_SP tree_leaf, tree_root;
    // Leaf:
    if (H.hasNode(plain_leaf->id())) {
        tree_leaf = dynamic_pointer_cast<PeeledNode>(H.getNode(plain_leaf->id()));
    } else {
        tree_leaf = PeeledNode::allocate(*plain_leaf);
        tree_leaf->setMasquerade(true);
        H.addNode(tree_leaf);
    }
    // Root:
    if (H.hasNode(plain_root->id())) {
        tree_root = dynamic_pointer_cast<PeeledNode>(H.getNode(plain_root->id()));
    } else {
        tree_root = PeeledNode::allocate(*plain_root);
        tree_root->setMasquerade(true);
        H.addNode(tree_root);
    }
    // Ensure that the root node's serial number is largest.
    tree_root->updateSerialNumber();
    // Now add an Edge to the Graph connecting root and leaf.
    // We always make the root end the source, so that, when regarded as
    // made up of directed Edges the tree will flow from root to leaves.
    H.addEdge(Edge::allocate(tree_root, tree_leaf));
}

vector<Stem_SP> dialect::makeStemsFromLeaves(const NodesById &leaves) {
    // Prepare the return vector.
    vector<Stem_SP> stems;
    // Iterate over the leaves.
    for (auto p : leaves) {
        // Get the leaf and its incident edges.
        Node_SP &leaf = p.second;
        const EdgesById &edges = leaf->getEdgeLookup();
        // leaf is supposed to actually be a leaf in the graph, i.e. should
        // have degree 1.
        COLA_ASSERT(edges.size() == 1);
        // This means we can get the one incident edge, and the node at the other end.
        Edge_SP edge = edges.cbegin()->second;
        Node_SP root = edge->getOtherEnd(*leaf);
        // Create the Stem.
        stems.push_back(make_shared<Stem>(leaf, root));
    }
    return stems;
}

PeeledNode_SP dialect::identifyRootNode(const Graph &graph) {
    // We want to find the PeeledNode of maximal serial number.
    id_type candidate_id;
    id_type max_serial_no = 0;
    const NodesById &nodes = graph.getNodeLookup();
    // Iterate over all nodes in the graph.
    for (auto p : nodes) {
        Node_SP &node = p.second;
        // Cast as PeeledNode.
        PeeledNode_SP pn = dynamic_pointer_cast<PeeledNode>(node);
        // If serial number is larger, make this node the candidate.
        if (pn->m_treeSerialNumber >= max_serial_no) {
            max_serial_no = pn->m_treeSerialNumber;
            candidate_id = pn->id();
        }
    }
    // Mark the root node.
    PeeledNode_SP root = dynamic_pointer_cast<PeeledNode>(graph.getNode(candidate_id));
    root->setIsRoot(true);
    // And return a shared pointer to it.
    return root;
}

NodeBuckets::NodeBuckets(Graph &graph) :
    m_graph(graph),
    m_maxDegree(graph.getMaxDegree()),
    m_buckets(m_maxDegree + 1)
{
    // Fill the buckets.
    for (auto p : graph.getNodeLookup()) {
        // Each node gets placed in a bucket according to its degree.
        unsigned degree = p.second->getDegree();
        m_buckets[degree].insert(p);
    }
}

NodesById NodeBuckets::takeLeaves(void) {
    // Make a copy of the bucket of leaves, i.e. nodes of degree 1.
    NodesById leaves(m_buckets[1]);
    // Now can clear the leaf bucket...
    m_buckets[1].clear();
    // ...and return the copy.
    return leaves;
}

bool NodeBuckets::moveNode(id_type id, unsigned oldDegree, unsigned newDegree) {
    // Make sure neither degree is too large.
    if (oldDegree > m_maxDegree || newDegree > m_maxDegree) return false;
    // Look for the node in the old bucket.
    NodesById &oldBucket = m_buckets[oldDegree];
    auto it = oldBucket.find(id);
    // If it's not in there, then fail.
    if (it == oldBucket.end()) return false;
    // Otherwise go ahead and move it.
    NodesById &newBucket = m_buckets[newDegree];
    newBucket.insert(*it);
    oldBucket.erase(it);
    return true;
}

void NodeBuckets::severNodes(const NodesById &nodes) {
    // Iterate over the given Nodes.
    for (auto p : nodes) {
        Node_SP &u = p.second;
        // Sever incident edges and get a vector of erstwhile neighbouring nodes.
        vector<Node_SP> nbrs = m_graph.severNodeNotingNeighbours(*u);
        // Move each former neighbour up one bucket.
        for (Node_SP v : nbrs) {
            unsigned degree = v->getDegree();
            moveNode(v->id(), degree + 1, degree);
        }
    }
    // And remove the Nodes fully from the Graph.
    m_graph.removeNodes(nodes);
}

Trees dialect::peel(Graph &G) {
    // Initialise a fresh Graph, essentially a workspace where we'll build the trees.
    Graph H;
    // The NodeBuckets object keeps the nodes in G sorted by degree.
    NodeBuckets buckets(G);
    // Start by getting the initial leaves.
    NodesById leaves = buckets.takeLeaves();
    while (!leaves.empty()) {
        // Make stems from leaves.
        vector<Stem_SP> stems = makeStemsFromLeaves(leaves);
        // Cut the leaves out of the graph.
        buckets.severNodes(leaves);
        if (G.isEmpty()) {
            // This case only arises when G was a tree with a double centre U--V
            // At this point we must have:
            //     stems == [Stem(U,V), Stem(V,U)]
            // and we must only add one (doesn't matter which) of those stems to H.
            COLA_ASSERT(stems.size() == 2);
            stems.pop_back();
        }
        // Add the stems to the new graph.
        for (Stem_SP stem : stems) stem->addSelfToGraph(H);
        // And now see if there are any newly-created leaves.
        leaves = buckets.takeLeaves();
    }
    // We have now repeatedly cut off all leaves until none remain, reconnecting
    // the removed leaves in the new graph H.
    // The connected components of H are the trees.
    Graphs comps = H.getConnComps();
    // Build and return Trees.
    Trees trees;
    bool G_empty = G.isEmpty();
    for (Graph_SP comp : comps) {
        PeeledNode_SP root = identifyRootNode(*comp);
        // Except in the case of a pure tree with a double centre, in which case the
        // original graph G is now empty, the node that is the root of each tree is
        // a GhostNode of a Node still belonging to the original Graph.
        // This is the "attachment point". For use later on, we mark that original
        // node as "root" as well (unless G is empty).
        if (!G_empty) {
            Node_SP originalRoot = G.getNode(root->id());
            originalRoot->setIsRoot(true);
        }
        // Finally, construct and record the Tree.
        Tree_SP tree = make_shared<Tree>(comp, root);
        trees.push_back(tree);
    }
    // And return.
    return trees;
}
