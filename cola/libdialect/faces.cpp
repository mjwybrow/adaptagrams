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

#include <list>
#include <utility>
#include <map>
#include <vector>
#include <memory>
#include <limits>
#include <string>
#include <algorithm>
#include <sstream>
#include <set>
#include <stdexcept>

#include "libavoid/geomtypes.h"
#include "libvpsc/assertions.h"
#include "libvpsc/rectangle.h"

#include "libdialect/ortho.h"
#include "libdialect/graphs.h"
#include "libdialect/constraints.h"
#include "libdialect/faces.h"
#include "libdialect/treeplacement.h"
#include "libdialect/expansion.h"

using namespace dialect;

using std::vector;

using Avoid::Point;

id_type Face::nextID = 0;

FaceSet::FaceSet(Graph_SP &G)
    : m_graph(G)
{
    // Compute the faces and identify the external one.
    computeFaces();
    identifyExternalFace();
    // Index the faces by the IDs of the Nodes that belong to them.
    for (Face_SP face : m_faces) {
        for (Node_SP node : face->getNodeSeq()) {
            m_facesByMemberNodeId[node->id()].insert(face);
        }
    }
    // Compute aligned sets.
    m_graph->getSepMatrix().getAlignedSets(m_hSets, m_vSets);
}

//! Face traversal code adapted from
//!   http://mathoverflow.net/questions/23811/reporting-all-faces-in-a-planar-graph
//! which in turn is an adaptation of a routine from SAGE:
//!   http://www.sagemath.org/doc/reference/graphs/sage/graphs/generic_graph.html#sage.graphs.generic_graph.GenericGraph.faces
//! 
//! As per discussion at the Mathoverflow page, it is a pretty simple idea:
//!
//!     "I'll assume the graph is connected, and that you have the clockwise or
//! counterclockwise ordering of the edges around each vertex. Then it's easy,
//! given a directed edge e, to walk around the face whose counterclockwise boundary
//! contains e. So make a list of all directed edges (i. e., two copies of each
//! undirected edge). Pick one directed edge, walk counterclockwise around its face,
//! and cross off all the directed edges you traverse. That's one face. Pick a
//! directed edge you haven't crossed off yet and walk around its face the same way.
//! Keep doing that until you've crossed off all of the edges. (Note that the
//! "counterclockwise" boundary of the exterior unbounded face actually goes clockwise
//! around the outside of the graph.)"
void FaceSet::computeFaces(void) {
    // Build a list of directed edges, represented as ordered pairs of node
    // IDs, with two directed edges per edge of the given graph G, one in
    // each direction.
    std::list<IdPair> edgeList;
    for (auto pair : m_graph->getEdgeLookup()) {
        Edge_SP &e = pair.second;
        id_type s = e->getSourceEnd()->id(),
                t = e->getTargetEnd()->id();
        edgeList.push_back({s, t});
        edgeList.push_back({t, s});

    }
    // Build the embedding, i.e. a map that looks like:
    //     v1:[v2,v3], v2:[v1], v3:[v1]
    // giving the clockwise ordering of neighbors at each vertex.
    // (Here the vi are IDs of nodes.)
    std::map<id_type, vector<id_type>> embedding;
    for (auto pair : m_graph->getNodeLookup()) {
        id_type uID = pair.first;
        Node_SP &u = pair.second;
        Nodes nbrs = u->getNeighboursCwCyclic();
        vector<id_type> nbrIds;
        nbrIds.reserve(nbrs.size());
        for (Node_SP v : nbrs) nbrIds.push_back(v->id());
        embedding[uID] = nbrIds;
    }
    // Initialise the first path.
    vector<IdPair> path;
    path.push_back(edgeList.front());
    edgeList.pop_front();
    // Trace the faces.
    while (!edgeList.empty()) {
        // Get the neighbour IDs of the target node of the final edge in the path.
        IdPair finalEdge = path.back();
        vector<id_type> nbrIds = embedding.at(finalEdge.second);
        // The source node of the final edge is among these neighbours.
        // Find its index in the clockwise order of neighbours, and decrement this
        // (mod number of neighbours) in order to traverse the face in clockwise order.
        size_t n = nbrIds.size(), i0 = n;
        // Check: every node in the face should have at least two neighbours.
        COLA_ASSERT(n >= 2);
        for (size_t i = 0; i < n; ++i) {
            if (nbrIds[i] == finalEdge.first) {
                i0 = i;
                break;
            }
        }
        COLA_ASSERT(i0 != n);
        // Since size_t can't be negative, we must add (n-1) rather that subtracting 1,
        // which would fail in the case i0 == 0. Then we can reduce mod n.
        size_t i1 = (i0 + n - 1) % n;
        id_type nextNodeId = nbrIds[i1];
        // For the next edge in the path.
        IdPair nextEdge{finalEdge.second, nextNodeId};
        // If it equals the first edge in the path, then we have finished traversing this face.
        if (nextEdge == path.front()) {
            // In this case it is time to build the Face, initialise it with the path,
            // and record it.
            Face_SP face = std::make_shared<Face>(m_graph);
            face->initWithEdgeSeq(path);
            m_faces.push_back(face);
            // Then get the next path started.
            path.clear();
            path.push_back(edgeList.front());
            edgeList.pop_front();
        }
        // Otherwise, add it to the current path, remove it from the list of remaining
        // edges, and continue.
        else {
            path.push_back(nextEdge);
            edgeList.remove(nextEdge);
        }
    }
    // After exiting the main loop, we may have a nonempty path. In that case, build
    // and record the last face based on it.
    Face_SP face = std::make_shared<Face>(m_graph);
    face->initWithEdgeSeq(path);
    m_faces.push_back(face);
}

void FaceSet::identifyExternalFace(void) {
    // We identify the external face and record it. We also set a flag in the external face
    // to mark it as such.
    // NB: We assume the graph is 4-planar orthogonal.
    // Begin by grabbing any Node u of maximal x-coord.
    Node_SP u = nullptr;
    double max_x = std::numeric_limits<double>::min();
    for (auto pair : m_graph->getNodeLookup()) {
        Node_SP &v = pair.second;
        Point p = v->getCentre();
        if (p.x > max_x) {
            u = v;
            max_x = p.x;
        }
    }
    COLA_ASSERT(u != nullptr);
    // The node u cannot have any neighbour to the east. Therefore the node v following
    // u in the clockwise traversal of the exterior face (which, remember, looks anticlockwise,
    // except from the point of view of the point at infinity on the Riemann sphere) must be
    // that which comes first in the anticlockwise ordering of u's neighbours starting with north.
    // Since the degree of u is at least two, and it can have at most one south neighbour, it
    // must have at least one neighbour among north and west. Therefore v is u's north neighbour
    // if it has one, else u's west neighbour.
    Node_SP west_nbr = nullptr,
            north_nbr = nullptr;
    for (Node_SP w : u->getNeighbours()) {
        CardinalDir d = Compass::cardinalDirection(u, w);
        if (d == CardinalDir::NORTH) {
            north_nbr = w;
            break;
        } else if (d == CardinalDir::WEST) {
            west_nbr = w;
        }
    }
    Node_SP v = north_nbr == nullptr ? west_nbr : north_nbr;
    COLA_ASSERT(v != nullptr);
    // The external face is then the unique one containing the "signature" of u's ID followed
    // by v's ID, in the clockwise traversal of its Nodes (considered cyclically).
    vector<id_type> signature{u->id(), v->id()};
    for (Face_SP F : m_faces) {
        if (F->containsNodeIdSeq(signature)) {
            F->m_isExternal = true;
            m_externalFace = F;
            break;
        }
    }
}

TreePlacements FaceSet::listAllPossibleTreePlacements(Tree_SP tree) {
    // First find the "core root", i.e. the Node in the underlying Graph where
    // the tree is supposed to reattach.
    Node_SP coreRoot = m_graph->getNode(tree->getRootNodeID());
    // Now get the vector of possible Faces, i.e. those Faces that contain this Node.
    std::set<Face_SP> possibleFaces = m_facesByMemberNodeId.at(coreRoot->id());
    // We can then ask each Face to generate all possible placements into it, and compile
    // the results.
    TreePlacements tps;
    for (Face_SP F : possibleFaces) F->listAllPossibleTreePlacements(tps, tree, coreRoot);
    // Inform each TreePlacement about the nodes that are aligned with its root node.
    for (TreePlacement_SP tp : tps) {
        Node_SP r = tp->getRootNode();
        tp->setRootAligns(vpsc::XDIM, m_hSets.at(r->id()));
        tp->setRootAligns(vpsc::YDIM, m_vSets.at(r->id()));
    }
    return tps;
}

TreePlacements FaceSet::getAllTreePlacements(void) {
    TreePlacements tps;
    for (Face_SP F : m_faces) F->getAllTreePlacements(tps);
    return tps;
}

std::map<CardinalDir, size_t> FaceSet::getNumTreesByGrowthDir(bool scaleBySize) const {
    std::map<CardinalDir, size_t> counts;
    for (Face_SP F : m_faces) F->getNumTreesByGrowthDir(counts, scaleBySize);
    return counts;
}

std::string Face::toString(void) const {
    std::ostringstream ss;
    ss << "Face (" << (m_isExternal ? "ext" : "int") << "):";
    for (Node_SP u : m_nodeSeq) ss << " " << u->id();
    return ss.str();
}

void Face::listAllPossibleTreePlacements(TreePlacements &tps, Tree_SP tree, Node_SP root) {
    CompassDirs dirs = inwardDirsAvailable(root);
    for (CompassDir dp : dirs) {
        // If it is a cardinal placement direction, then the growth direction can only be the same.
        // On the other hand, if it is an ordinal placement direction, then there are two growth directions
        // to be considered; namely, the two cardinal components of the ordinal direction.
        // Thus, for all directions, the Compass::cardinalComponents function returns exactly the vector
        // of growth directions we wish to consider.
        CardinalDirs dgs = Compass::cardinalComponents(dp);
        for (CardinalDir dg : dgs) {
            tps.push_back(std::make_shared<TreePlacement>(tree, *this, root, dp, dg));
            // If the tree is not symmetrical, then we also consider the flipped version.
            if (!tree->isSymmetrical()) tps.push_back(std::make_shared<TreePlacement>(tree, *this, root, dp, dg, true));
        }
    }
}

CompassDirs Face::inwardDirsAvailable(Node_SP node) {
    // Prepare return value.
    CompassDirs available;
    // We will make use of the doubled anticlockwise compass.
    const CompassDirs &allDirs = Compass::acwAllDoubled;
    // The inward directions are identified by examining the "neighbour pairs" for the given Node.
    // For each pair, we find the inward directions between them.
    vector<std::pair<Node_SP, Node_SP>> nbrPairs = m_nbrPairs.at(node->id());
    for (auto p : nbrPairs) {
        Node_SP prev = p.first,
                next = p.second;
        /**
         * Because of the way the faces are traced out, swinging from the previous
         * node to the next one sweeps out an area of the /inside/ of this face
         * when you go /anticlockwise/.
         *
         * Therefore if d0 is the compass direction from node to prev, and if d1
         * is that from node to next, then the available directions are all those
         * strictly between d0 and d1 /in the anticlockwise direction/.
         *
         * For example, if node --> prev points South while node --> next points
         * West, then both East and North are available.
         * But if node --> prev points East while node --> next points North, then
         * only NE is available.
         */
        CompassDir d0 = (CompassDir) Compass::cardinalDirection(node, prev),
                   d1 = (CompassDir) Compass::cardinalDirection(node, next);
        size_t i = 0;
        // Advance to direction d0:
        while (allDirs[i] != d0) ++i;
        // Advance one step beyond it:
        ++i;
        // Record the intervening directions.
        while (allDirs[i] != d1) {
            available.push_back(allDirs[i]);
            ++i;
        }
    }
    return available;
}


void Face::initWithEdgeSeq(const vector<IdPair> &edges) {
    // Record the number of Nodes.
    m_n = edges.size();
    // Get the Nodes from the Graph.
    m_nodeSeq.reserve(m_n);
    for (IdPair p : edges) {
        m_nodeSeq.push_back(m_graph->getNode(p.first));
    }
    computeNbrPairs();
    computeSides();
    buildNexes();
}

void Face::computeNbrPairs(void) {
    // First pass: identify the indices at which each node occurs in the nodeseq.
    std::map<id_type, vector<size_t>> indices;
    for (size_t i = 0; i < m_n; ++i) {
        Node_SP &u = m_nodeSeq[i];
        indices[u->id()].push_back(i);
    }
    // Second pass: assemble the neighbour pairs.
    Nodes &ns = m_nodeSeq;
    size_t n = m_n;
    for (Node_SP u : ns) {
        vector<std::pair<Node_SP, Node_SP>> pairs;
        vector<size_t> indices_for_node_u = indices[u->id()];
        pairs.reserve(indices_for_node_u.size());
        for (size_t i : indices_for_node_u) {
            pairs.push_back({ns[(i+n-1)%n], ns[(i+1)%n]});
        }
        m_nbrPairs.insert({u->id(), pairs});
    }
}

void Face::computeSides(void) {
    // Get index of first bend.
    size_t i0 = findIndexOfFirstBend();
    // Prepare a node sequence starting from node i0, then
    // cycling back around to i0 and including node i0 /again/.
    // Then we know that a Side begins at the start of this sequence,
    // and a Side ends at the end of it, and there is no Side between
    // the last and first one.
    Nodes ns;
    ns.reserve(m_n + 1);
    for (size_t i = i0; i < m_n; ++i) ns.push_back(m_nodeSeq[i]);
    for (size_t i = 0; i < i0+1; ++i) ns.push_back(m_nodeSeq[i]);
    // Initialise the loop. We will always be checking the direction from
    // a Node u to a Node v. We start with the first two.
    Node_SP u = ns[0], v = ns[1];
    CardinalDir d0 = direc(u, v);
    // We will be building the sequence of Nodes on a given Side.
    Nodes nodesOnASide{u, v};
    // Now consider all remaining Nodes.
    for (size_t i = 2; i < m_n + 1; ++i) {
        v = ns[i];
        u = nodesOnASide.back();
        CardinalDir d1 = direc(u, v);
        if (d1 == d0) {
            // Still on the same Side.
            nodesOnASide.push_back(v);
        } else {
            // We're beginning a new Side.
            // First construct and record the current one.
            m_sides.push_back(std::make_shared<Side>(nodesOnASide, d0));
            // Now begin the new one.
            nodesOnASide = {u, v};
            d0 = d1;
        }
    }
    // Construct and record the final side.
    m_sides.push_back(std::make_shared<Side>(nodesOnASide, d0));
}

void Face::buildNexes(void) {
    // Build a Nexus for each Node.
    for (Node_SP u : m_nodeSeq) m_nexesByNodeIds.insert({u->id(), std::make_shared<Nexus>(u)});
    // Now add each Side to the Nexus for each Node it contains.
    for (Side_SP S : m_sides) {
        for (Node_SP v : S->getNodeSeq()) {
            m_nexesByNodeIds.at(v->id())->addSide(S);
        }
    }
}

bool Face::containsNodeIdSeq(std::vector<id_type> idSeq) const {
    if (idSeq.empty()) return true;  // vacuously true
    id_type firstId = idSeq.front();
    // Search for first ID.
    size_t n = m_nodeSeq.size(), i0 = n;
    for (size_t i = 0; i < n; ++i) {
        const Node_SP &u = m_nodeSeq[i];
        if (u->id() == firstId) {
            i0 = i; break;
        }
    }
    // If we didn't find the first Id, then we certainly do not have the whole sequence.
    if (i0 == n) return false;
    // Otherwise we check whether we have the remainder of the sequence.
    size_t m = idSeq.size();
    for (size_t j = 1; j < m; ++j) {
        if (m_nodeSeq[(i0+j)%n]->id() != idSeq[j]) return false;
    }
    return true;
}

CardinalDir Face::direc(const Node_SP &u, const Node_SP &v) {
    return m_graph->getSepMatrix().getCardinalDir(u->id(), v->id());
}

size_t Face::findIndexOfFirstBend(void) {
    // Prepare a vector of Nodes equal to our nodeSeq plus a "wraparound" node at each end.
    Nodes loop(m_n + 2);
    loop[0] = m_nodeSeq[m_n - 1];
    loop[m_n+1] = m_nodeSeq[0];
    auto it = ++loop.begin();
    std::copy(m_nodeSeq.begin(), m_nodeSeq.end(), it);
    // Now we can check for the first bend, using a sliding window of length 3.
    for (size_t i = 0; i < m_n; ++i) {
        Node_SP u = loop[i], v = loop[i+1], w = loop[i+2];
        CardinalDir duv = direc(u, v),
                    dvw = direc(v, w);
        if (dvw != duv) return i;
    }
    // If we reach this point, we didn't find a bend. This should never happen, since every
    // Face should have at least one bend.
    COLA_ASSERT(false);
}

Sides Face::getRelevantSidesForPlacement(TreePlacement_SP tp) const {
    // Get the root node of the placement.
    Node_SP root = tp->getRootNode();
    // Get the Nexus representing that node.
    Nexus_SP nexus = m_nexesByNodeIds.at(root->id());
    // Get the intended placement direction.
    CompassDir placementDir = tp->getPlacementDir();
    // Ask the Nexus for the set of relevant Sides.
    std::set<Side_SP> relevantSideSet = nexus->getNeighboursOfADirection(placementDir);
    // We return a vector instead of a set.
    Sides relevantSides(relevantSideSet.size());
    std::copy(relevantSideSet.begin(), relevantSideSet.end(), relevantSides.begin());
    return relevantSides;
}

void Face::insertTreeNode(TreePlacement_SP tp, double padding) {
    // First ask the relevant Side object(s) to note the placement.
    for (Side_SP S : getRelevantSidesForPlacement(tp)) S->addTreePlacement(tp);
    // Now insert the tree node.
    Node_SP treeNode = tp->buildTreeBox(padding),
            rootNode = tp->getRootNode();
    // The dimensions of the treeNode are already as we want them.
    // However, its centre indicates only its displacement from the root node.
    // We now need to translate this to absolute coordinates by adding this
    // displacement to the coordinates of the root node.
    Point ct = treeNode->getCentre(),
          cr = rootNode->getCentre();
    treeNode->setCentre(ct.x + cr.x, ct.y + cr.y);
    // Now add the tree node to the graph.
    m_graph->addNode(treeNode);
    // Keep records.
    id_type tid = treeNode->id(),
            rid = rootNode->id();
    m_treeNodes.insert({tid, treeNode});
    m_treePlacementsByNodeIds[rid] = tp;
    tp->recordBoxNode(treeNode);
    // Constrain the tree node to sit beside the root node.
    m_graph->getSepMatrix().addFixedRelativeSep(rid, tid, ct.x, ct.y);
}

ProjSeq_SP Face::computeCollateralProjSeq(TreePlacement_SP tp, double padding) {
    ProjSeq_SP ps = std::make_shared<ProjSeq>();
    Sides sides = getRelevantSidesForPlacement(tp);
    // Sanity check: should be exactly one or two relevant Sides:
    size_t n = sides.size();
    COLA_ASSERT(n == 1 || n == 2);
    // Now simply extend the ProjSeq for each Side, and return the result.
    for (Side_SP S : sides) *ps += *(S->computeCollateralProjSeq(tp, padding));
    return ps;
}

bool Face::applyProjSeq(ProjSeq &ps) {
    // When applying a projection sequence in order to expand a Face,
    // we always want overlap prevention, including with aligned edges.
    ColaOptions opts;
    opts.preventOverlaps = true;
    opts.solidifyAlignedEdges = true;
    return m_graph->applyProjSeq(opts, ps);
}

ProjSeq_SP Face::doCollateralExpansion(TreePlacement_SP tp, double padding) {
    // Auto-infer padding if requested.
    if (padding < 0) {
        padding = m_graph->getIEL()/4.0;
    }
    ProjSeq_SP ps = computeCollateralProjSeq(tp, padding);
    bool okay = applyProjSeq(*ps);
    if (!okay) throw std::runtime_error("Infeasible collateral tree sep");
    return ps;
}

void Face::buildBdrySegsFacingOneDir(CardinalDir facingDir,
                                     LineSegments &closedSegs, LineSegments &openSegs,
                                     TreePlacement_SP ignoreTP) {

    // Since a node may occur more than once in the traversal of a face, we keep
    // a set of visited IDs.
    std::set<id_type> visitedNodeIds;
    // Are we going to be ignoring any tree boxes?
    std::set<id_type> ignoreTreeBoxNodeIds;
    if (ignoreTP != nullptr) {
        Sides sides = getRelevantSidesForPlacement(ignoreTP);
        for (Side_SP S : sides) {
            for (Node_SP u : S->getNodeSeq()) {
                ignoreTreeBoxNodeIds.insert(u->id());
            }
        }
    }
    // Begin building segments.
    // Iterate over all Nodes in the Face.
    for (size_t i = 0; i < m_n; ++i) {
        // Grab next node, and the one following it, cyclically.
        Node_SP u = m_nodeSeq[i],
                v = m_nodeSeq[(i+1)%m_n];
        // Consider the Edge from u to v.
        // If it's perpendicular to the facing direction...
        CardinalDir outdir = direc(u, v);
        if (Compass::arePerpendicular(outdir, facingDir)) {
            // ...then build a closed interval for it.
            Point p0 = u->getBoundaryCompassPt((CompassDir)outdir),
                  p1 = v->getBoundaryCompassPt((CompassDir)Compass::cardFlip(outdir));
            closedSegs.push_back(std::make_shared<LineSegment>(p0, p1));
        }
        // Has node u been visited yet?
        id_type uid = u->id();
        if (visitedNodeIds.find(uid) == visitedNodeIds.end()) {
            // If not then build an open interval for the facing side of the node box.
            BoundingBox b = u->getBoundingBox();
            openSegs.push_back(b.buildSideSegment(facingDir));
            // If a tree box has been placed here, build an open interval for its facing side
            // (unless we're supposed to ignore this one).
            TreePlacement_SP tp = m_treePlacementsByNodeIds[uid];
            if (tp != nullptr && ignoreTreeBoxNodeIds.find(uid) == ignoreTreeBoxNodeIds.end()) {
                // We use *unpadded* tree boxes when considering where the boundary lies.
                double padding = 0;
                Node_SP treeBox = tp->buildTreeBox(padding);
                // Translate relative to root node.
                Point c = u->getCentre();
                treeBox->translate(c.x, c.y);
                // Get box and build segment.
                BoundingBox b2 = treeBox->getBoundingBox();
                openSegs.push_back(b2.buildSideSegment(facingDir));
            }
            // Note node u as visited.
            visitedNodeIds.insert(uid);
        }
    }
}

ProjSeq_SP Face::buildBestProjSeq(TreePlacement_SP tp, double padding,
                                  bool doCostlierDimensionFirst, ExpansionEstimateMethod estimateMethod) {
    // Save initial node positions.
    m_graph->pushNodePositions();
    // Start by removing any overlaps with collateral nodes.
    ProjSeq_SP ps0 = doCollateralExpansion(tp, padding);
    // Choose primary expansion dimension.
    ExpansionManager em(tp, vpsc::XDIM, padding);
    std::map<vpsc::Dim, double> costs;
    switch(estimateMethod) {
    case ExpansionEstimateMethod::SPACE:
        costs = em.estimateCostByDimension();
        break;
    case ExpansionEstimateMethod::CONSTRAINTS:
        costs = em.estimateCostByDimension2();
        break;
    }
    double xEst = costs[vpsc::XDIM],
           yEst = costs[vpsc::YDIM];
    vpsc::Dim dim;
    if (doCostlierDimensionFirst) {
        dim = xEst > yEst ? vpsc::XDIM : vpsc::YDIM;
    } else {
        dim = xEst > yEst ? vpsc::YDIM : vpsc::XDIM;
    }
    // Expand.
    ExpansionManager em2(tp, dim, padding);
    ProjSeq_SP ps1 = em2.extendProjSeq(ps0);
    if (ps1 == nullptr) {
        // First try failed. Try again, with the other dimension as primary.
        ExpansionManager em3(tp, vpsc::conjugate(dim), padding);
        m_graph->popNodePositions();
        m_graph->pushNodePositions();
        // Can reuse `ps0`, since `ExpansionGoal::tryExpansionRec()` works
        // with a fresh copy of it, leaving it unaltered.
        ps1 = em3.extendProjSeq(ps0);
    }
    // Restore node positions and return.
    m_graph->popNodePositions();
    return ps1;
}

Sides Face::getAllSidesOppositeSegment(LineSegment &seg, bool openInterval) const {
    Sides sidesOpp(m_sides.size());
    auto it = std::copy_if(m_sides.begin(), m_sides.end(), sidesOpp.begin(), [&seg, openInterval](const Side_SP &S)->bool {
        return S->liesOppositeSegment(seg, openInterval);
    });
    sidesOpp.resize(it-sidesOpp.begin());
    return sidesOpp;
}

TreePlacements Face::getAllTreePlacements(void) const {
    TreePlacements tps;
    tps.reserve(m_treePlacementsByNodeIds.size());
    for (auto p : m_treePlacementsByNodeIds) {
        TreePlacement_SP tp = p.second;
        if (tp != nullptr) tps.push_back(tp);
    }
    return tps;
}

std::set<TreePlacement_SP> Face::getSetOfAllTreePlacements(void) const {
    std::set<TreePlacement_SP> tps;
    for (auto p : m_treePlacementsByNodeIds) {
        TreePlacement_SP tp = p.second;
        if (tp != nullptr) tps.insert(tp);
    }
    return tps;
}

void Face::getAllTreePlacements(TreePlacements &tps) const {
    for (auto p : m_treePlacementsByNodeIds) {
        TreePlacement_SP tp = p.second;
        if (tp != nullptr) tps.push_back(tp);
    }
}

void Face::getNumTreesByGrowthDir(std::map<CardinalDir, size_t> &counts, bool scaleBySize) const {
    TreePlacements tps = getAllTreePlacements();
    for (auto tp : tps) {
        CardinalDir dg = tp->getGrowthDir();
        counts[dg] += scaleBySize ? tp->size() : 1;
    }
}
