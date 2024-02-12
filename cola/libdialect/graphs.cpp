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

#include <algorithm>
#include <vector>
#include <memory>
#include <deque>
#include <utility>
#include <string>
#include <sstream>
#include <iostream>
#include <map>
#include <stdexcept>
#include <cmath>
#include <iterator>
#include <functional>

#include "libavoid/geomtypes.h"
#include "libvpsc/rectangle.h"
#include "libcola/cluster.h"
#include "libcola/cola.h"

#include "libdialect/constraints.h"
#include "libdialect/io.h"
#include "libdialect/util.h"
#include "libdialect/ortho.h"
#include "libdialect/logging.h"
#include "libdialect/routing.h"
#include "libdialect/graphs.h"

using namespace dialect;

using std::max;
using std::min;
using std::vector;
using std::shared_ptr;
using std::make_shared;
using std::deque;
using std::pair;
using std::string;
using std::ostringstream;
using std::map;
using std::fabs;

using vpsc::Rectangle;
using vpsc::Rectangles;

using cola::CompoundConstraints;

using Avoid::Point;

id_type Node::nextID = 0;
id_type Edge::nextID = 0;

//! @brief  Adding two bounding boxes returns the bounding box of their union.
BoundingBox operator+(const BoundingBox &lhs, const BoundingBox &rhs) {
    BoundingBox sum = lhs;
    sum += rhs;
    return sum;
}

bool operator==(const BoundingBox &lhs, const BoundingBox &rhs) {
    return lhs.x==rhs.x && lhs.X==rhs.X && lhs.y==rhs.y && lhs.Y==rhs.Y;
}

bool operator!=(const BoundingBox &lhs, const BoundingBox &rhs) {
    return !(lhs==rhs);
}

BoundingBox &BoundingBox::operator+=(const BoundingBox &rhs) {
    x = min(x, rhs.x);
    X = max(X, rhs.X);
    y = min(y, rhs.y);
    Y = max(Y, rhs.Y);
    return *this;
}

string BoundingBox::repr(void) const {
    return string_format("[%.2f, %.2f] x [%.2f, %.2f]", x, X, y, Y);
}

LineSegment_SP BoundingBox::buildSideSegment(CardinalDir side) const {
    switch(side) {
    case CardinalDir::EAST:
        return std::make_shared<LineSegment>(Point(X, y), Point(X, Y));
    case CardinalDir::WEST:
        return std::make_shared<LineSegment>(Point(x, y), Point(x, Y));
    case CardinalDir::SOUTH:
        return std::make_shared<LineSegment>(Point(x, Y), Point(X, Y));
    case CardinalDir::NORTH:
        return std::make_shared<LineSegment>(Point(x, y), Point(X, y));
    default:
        COLA_ASSERT(false);

    }
}

Graph::Graph(const Graph &G)
    : m_debugOutputPath(G.m_debugOutputPath),
      m_projectionDebugLevel(G.m_projectionDebugLevel),
      m_sepMatrix(G.m_sepMatrix),
      m_iel(G.m_iel),
      m_needNewRectangles(true),
      m_nodes(G.m_nodes),
      m_edges(G.m_edges),
      m_maxDeg(G.m_maxDeg)
{
    // We deliberately do not copy any of the "cola stuff",
    // namely m_cgr, m_needNewRectangles, and m_cfdl.

    // As for the SepMatrix, we are happy to have a copy of that from
    // the existing Graph, except our copy must point back to this Graph.
    m_sepMatrix.setGraph(this);
}

Graph::~Graph(void) {
    delete m_cfdl;
    for (Rectangle *r : m_cgr.rs) delete r;
}

Graph &Graph::operator=(Graph other) {
    // Thanks to https://stackoverflow.com/a/3279550
    swap(*this, other);
    return *this;
}

unsigned Graph::getMaxDegree(void) const {
    return m_maxDeg;
}

void Graph::addNode(Node_SP node, bool takeOwnership) {
    // Note that, after this operation, it will be necessary to
    // recompute Rectangles.
    m_needNewRectangles = true;
    // Add a copy of the Node_SP to our lookup table.
    m_nodes.emplace(node->id(), node);
    if (takeOwnership) {
        // Tell the Node that this is its Graph.
        node->setGraph(*this);
    }
}

Node_SP Graph::addNode(void) {
    Node_SP u = Node::allocate();
    addNode(u);
    return u;
}

Node_SP Graph::addNode(double w, double h) {
    Node_SP u = Node::allocate(w, h);
    addNode(u);
    return u;
}

Node_SP Graph::addNode(double cx, double cy, double w, double h) {
    Node_SP u = Node::allocate(cx, cy, w, h);
    addNode(u);
    return u;
}

void Graph::addEdge(Edge_SP edge, bool takeOwnership) {
    // Add a copy of the Edge_SP to our lookup table.
    m_edges.emplace(edge->id(), edge);
    if (takeOwnership) {
        // Tell the Node that this is its Graph.
        edge->setGraph(*this);
    }
    // Update the Graph's max degree.
    m_maxDeg = max(m_maxDeg, edge->getSourceEnd()->getDegree());
    m_maxDeg = max(m_maxDeg, edge->getTargetEnd()->getDegree());
}

Edge_SP Graph::addEdge(Node_SP src, Node_SP tgt) {
    Edge_SP e = Edge::allocate(src, tgt);
    addEdge(e);
    return e;
}

Edge_SP Graph::addEdge(const id_type &srcID, const id_type &tgtID) {
    Node_SP src = getNode(srcID),
            tgt = getNode(tgtID);
    return addEdge(src, tgt);
}

bool Graph::hasNode(const id_type &id) const {
    return m_nodes.find(id) != m_nodes.end();
}

bool Graph::hasEdge(const id_type &id) const {
    return m_edges.find(id) != m_edges.end();
}

void Graph::severEdge(Edge &edge) {
    // Call the Edge's sever method in order to remove the Edge
    // from each of its endpoint Nodes.
    edge.sever();
    // And delete the Edge from our lookup.
    m_edges.erase(edge.id());
    // Maximum degree may have changed.
    recomputeMaxDegree();
}

void Graph::severNode(const Node &node) {
    EdgesById edges = node.getCopyOfEdgeLookup();
    for (auto p : edges) {
        severEdge(*p.second);
    }
}

vector<Node_SP> Graph::severNodeNotingNeighbours(const Node &node) {
    vector<Node_SP> nbrs;
    EdgesById edges(node.getEdgeLookup());
    for (auto p : edges) {
        Edge_SP &e = p.second;
        nbrs.push_back(e->getOtherEnd(node));
        severEdge(*e);
    }
    return nbrs;
}

void Graph::removeNode(const Node &node) {
    // Note that, after this operation, it will be necessary to
    // recompute Rectangles.
    m_needNewRectangles = true;
    // Remove the Node from the SepMatrix.
    m_sepMatrix.removeNode(node.id());
    // Remove the Node from our own lookup.
    m_nodes.erase(node.id());
}

void Graph::removeNodes(const NodesById &nodes) {
    // Note that, after this operation, it will be necessary to
    // recompute Rectangles.
    m_needNewRectangles = true;
    // Remove the Nodes from the SepMatrix.
    m_sepMatrix.removeNodes(nodes);
    // Remove the Nodes from our own lookup.
    // We take advantage of the ordering of the maps in order to do
    // the removal in one pass.
    NodesById N;
    auto p = m_nodes.cbegin();
    auto q = nodes.cbegin();
    while (p != m_nodes.cend() && q != nodes.cend()) {
        id_type i = (*p).first, j = (*q).first;
        if (i > j) ++q;
        else {
            if (i < j) N.insert(*p);
            ++p;
        }
    }
    while (p != m_nodes.cend()) {
        N.insert(*p);
        ++p;
    }
    m_nodes = N;
}

void Graph::severAndRemoveNode(const Node &node) {
    severNode(node);
    removeNode(node);
}

void Graph::severAndRemoveNode(id_type nodeID) {
    Node_SP node = getNode(nodeID);
    if (node != nullptr) severAndRemoveNode(*node);
}

Nodes Graph::cloneNode(id_type id) {
    Node_SP baseNode = getNode(id);
    EdgesById edges = baseNode->getCopyOfEdgeLookup();
    Nodes clones;
    bool first = true;
    for (auto p : edges) {
        // Skip one edge.
        if (first) {
            first = false;
            continue;
        }
        // For every other edge, we make a clone.
        Node_SP clone = Node::allocate();
        clone->copyGeometry(*baseNode);
        clone->copyOtherGhostProperties(*baseNode);
        addNode(clone);
        clones.push_back(clone);
        // Get the existing node at the other end of the edge.
        Edge_SP e = p.second;
        Node_SP nbr = e->getOtherEnd(*baseNode);
        // Cut the old edge and add a new one.
        severEdge(*e);
        Edge_SP f = Edge::allocate(clone, nbr);
        addEdge(f);
    }
    return clones;
}

Node_SP Graph::getNode(const id_type &id) const {
    // Use .at() rather than [] so that we throw an exception
    // in the case of a nonexistent ID, rather than creating a new Node_SP.
    // (I.e. so this method can be const.)
    return m_nodes.at(id);
}

NodesById Graph::getNodeLookupWithIgnore(const Nodes &ignore) const {
    // Build set of IDs to be ignored.
    std::set<id_type> toIgnore;
    for (auto u : ignore) toIgnore.insert(u->id());
    // Build desired lookup.
    NodesById desired;
    std::set_difference(m_nodes.begin(), m_nodes.end(), toIgnore.begin(), toIgnore.end(),
                        std::inserter(desired, desired.end()), NodeIdCmp());
    return desired;
}

NodesById Graph::getNodeLookupWithIgnore(const NodesById &ignore) const {
    // Build desired lookup.
    NodesById desired;
    std::set_difference(m_nodes.begin(), m_nodes.end(), ignore.begin(), ignore.end(),
                        std::inserter(desired, desired.end()),
                        [](const IdNodePair &a, const IdNodePair &b)->bool {
        return a.first < b.first;
    });
    return desired;
}

double Graph::computeAvgNodeDim(void) const {
    double s = 0;
    size_t n = 0;
    dimensions d;
    for (auto p : m_nodes) {
        d = p.second->getDimensions();
        s += d.first + d.second;
        n += 2;
    }
    return s/n;
}

BoundingBox Graph::getBoundingBox(const NodesById &ignore, bool includeBends) const {
    // First filter out the Nodes that are to be ignored.
    vector<IdNodePair> keep(m_nodes.size());
    auto it = std::set_difference(
        m_nodes.cbegin(), m_nodes.cend(), ignore.cbegin(), ignore.cend(), keep.begin(),
        [](const IdNodePair &lhs, const IdNodePair &rhs){ return lhs.first < rhs.first; }
    );
    keep.resize(it - keep.begin());
    // Initialize an empty box.
    BoundingBox box;
    // Add each Node's box to it.
    for (auto p : keep) {
        box += p.second->getBoundingBox();
    }
    // If we want to include bend points, then also add each Edge's box.
    if (includeBends) {
        for (auto p : m_edges) {
            box += p.second->getBoundingBox();
        }
    }
    return box;
}

vector<Graph_SP> Graph::getConnComps(void) const {
    vector<Graph_SP> comps;
    // Make a copy of the Node lookup. This will keep track of any Nodes we have
    // not yet explored in our BFSes.
    NodesById remaining(m_nodes);
    while (!remaining.empty()) {
        // Initialize a new component Graph.
        Graph_SP new_comp = make_shared<Graph>();
        // Take a remaining Node...
        auto it = remaining.cbegin();
        Node_SP u0 = it->second;
        remaining.erase(it);
        // ...and add it to the new component.
        new_comp->addNode(u0, false);
        // We do BFS outward from this node.
        // We use a queue holding pairs: an Edge to follow, and the Node we reach
        // by following it.
        deque<pair<Edge_SP, Node_SP>> bfs_queue;
        // Initialize the queue with the edges incident to the initial node.
        EdgesById initialEdges = u0->getEdgeLookup();
        size_t n = initialEdges.size();
        bfs_queue.resize(n);
        std::transform(
            initialEdges.cbegin(), initialEdges.cend(), bfs_queue.begin(),
            [&u0](const IdEdgePair &p) -> pair<Edge_SP, Node_SP> {
                return {p.second, p.second->getOtherEnd(*u0)};
            }
        );
        // Now do BFS.
        Edge_SP e;
        Node_SP v;
        while (!bfs_queue.empty()) {
            // Take from the front of the queue.
            e = bfs_queue.front().first;
            v = bfs_queue.front().second;
            bfs_queue.pop_front();
            // Add the Edge if not already in the new component.
            if (!new_comp->hasEdge(e->id())) new_comp->addEdge(e, false);
            // If the Node is already in the new component then continue to next case.
            if (new_comp->hasNode(v->id())) continue;
            // Otherwise add the Node to the new component...
            new_comp->addNode(v, false);
            // ...remove it from the remaining set...
            remaining.erase(v->id());
            // ...and extend the queue using the new node's incident edges.
            EdgesById newEdges = v->getEdgeLookup();
            n = newEdges.size();
            size_t m = bfs_queue.size();
            bfs_queue.resize(m+n);
            std::transform(
                newEdges.cbegin(), newEdges.cend(), bfs_queue.end() - n,
                [&v](const IdEdgePair &p) -> pair<Edge_SP, Node_SP> {
                    return {p.second, p.second->getOtherEnd(*v)};
                }
            );
        }
        // When the queue is empty we should have explored the entire
        // connected component. Add it to the list.
        comps.push_back(new_comp);
    }
    // Return the list of components.
    return comps;
}

void Graph::getChainsAndCycles(std::vector<std::deque<Node_SP> > &chains, std::vector<std::deque<Node_SP> > &cycles) {
    // First identify all links in the Graph, i.e. Nodes of degree 2.
    std::set<Node_SP> allLinks;
    for (auto p : m_nodes) {
        Node_SP u = p.second;
        if (u->getDegree() == 2) allLinks.insert(u);
    }
    // Now we explore all the links, building chains and cycles.
    while (!allLinks.empty()) {
        // Take any link still in the set.
        auto it = allLinks.begin();
        Node_SP L0 = *it;
        allLinks.erase(it);
        // Initialise a deque to hold all links in the chain to which L0 belongs.
        std::deque<Node_SP> links{L0};
        // Get the two Edges incident to L0, and prepare to explore in both directions.
        const EdgesById &E0 = L0->getEdgeLookup();
        COLA_ASSERT(E0.size() == 2);
        int direc = -1;
        bool isPolygon = false;
        for (auto p : E0) {
            Edge_SP e = p.second;
            if (isPolygon) break;
            // Explore from link L0 in one direction.
            direc *= -1;
            Node_SP last = L0;
            bool done = false;
            while (!done) {
                // Consider the next node in the current direction.
                Node_SP next = e->getOtherEnd(*last);
                if (next == L0) {
                    // In this case the entire connected component to which L0 belongs is
                    // a mere polygon.
                    isPolygon = true;
                    cycles.push_back(links);
                    links.clear();
                    done = true;
                } else if (next->getDegree() == 2) {
                    // This must be a link which we have not encountered before.
                    allLinks.erase(next);
                    // Add this link to the correct side of the deque, according to the
                    // direction in which we are currently exploring.
                    if (direc == 1) {
                        links.push_back(next);
                    } else {
                        links.push_front(next);
                    }
                    // Get that one of the two incident edges to node 'next' that leads onward, away from 'last'.
                    const EdgesById &E1 = next->getEdgeLookup();
                    for (auto q : E1) {
                        if (q.second != e) {
                            e = q.second;
                            break;
                        }
                    }
                    last = next;
                } else {
                    // We've reached the "anchor node" at one end of the chain.
                    done = true;
                }
            }
        }
        // Now we have explored from link L0 in both directions, or else found that
        // it belonged to a polygon. The case is the former iff links is nonempty.
        if (!links.empty()) chains.push_back(links);
    }
}

string Graph::writeTglf(bool useExternalIds) const {
    map<id_type, unsigned> id2ext;
    int base_id = 0;
    ostringstream ss, edges_ss;
    // Nodes
    if (useExternalIds) {
        /* We will write external IDs for all Nodes for which they have been set.
         * However, in case there are some nodes that do not have external IDs (which happens,
         * for example, when new nodes, such as bend nodes, have been generated), we first
         * determine the maximum external ID, and then generate new IDs based on that.
        */
        int max_ext_id = -1;
        int first_int_id_lacking_ext = -1;
        for (auto pair : m_nodes) {
            id_type id = pair.first;
            Node_SP &u = pair.second;
            int ext_id = u->getExternalId();
            max_ext_id = max(max_ext_id, ext_id);
            if (ext_id == -1 && first_int_id_lacking_ext == -1) first_int_id_lacking_ext = id;
        }
        base_id = max_ext_id + 1;
        // However, to make debugging easier, we also want to avoid shifting any internal IDs
        // unless necessary in order to avoid collisions.
        if (first_int_id_lacking_ext > max_ext_id) base_id = 0;
    }
    for (auto pair : m_nodes) {
        id_type id = pair.first;
        Node_SP node = pair.second;
        Point c = node->getCentre();
        dimensions d = node->getDimensions();
        if (useExternalIds) {
            int ext_id = node->getExternalId();
            if (ext_id < 0) ext_id = base_id + id;
            id2ext.insert({id, ext_id});
            id = (id_type) ext_id;
        }
        ss << id << " " << c.x << " " << c.y << " " << d.first << " " << d.second << "\n";
    }
    // Edges
    for (auto p : m_edges) {
        id_type sid = p.second->getSourceEnd()->id(),
                tid = p.second->getTargetEnd()->id();
        if (useExternalIds) {
            sid = id2ext.at(sid);
            tid = id2ext.at(tid);
        }
        edges_ss << sid << " " << tid << p.second->writeRouteTglf() << "\n";
    }
    string edges_tglf = edges_ss.str();
    // SepCos
    string sepcos_tglf = m_sepMatrix.writeTglf(id2ext);
    // Put it all together.
    bool have_edges  = !edges_tglf.empty(),
         have_sepcos = !sepcos_tglf.empty();
    if (have_edges || have_sepcos) {
        ss << "#\n" << edges_tglf;
        if (have_sepcos) {
            ss << "#\n" << sepcos_tglf;
        }
    }
    return ss.str();
}

string Graph::writeSvg(bool useExternalIds) const {
    NodesById ignore;
    bool includeBends = true;
    BoundingBox b = getBoundingBox(ignore, includeBends);
    double pad = 8;
    b.x -= pad; b.X += pad; b.y -= pad; b.Y += pad;
    ostringstream ss;
    ss << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
    ss << "<svg xmlns:inkscape=\"http://www.inkscape.org/namespaces/inkscape\" xmlns=\"http://www.w3.org/2000/svg\" width=\"100%%\" height=\"100%%\" ";
    ss << string_format("viewBox=\"%g %g %g %g\">\n", b.x, b.y, b.w(), b.h());
    // Edges
    for (auto p : m_edges) {
        Edge_SP e = p.second;
        ss << e->writeSvg();
    }
    // Nodes
    for (auto p : m_nodes) {
        Node_SP u = p.second;
        ss << u->writeSvg(useExternalIds);
    }
    ss << "</svg>\n";
    return ss.str();
}

void Graph::recomputeMaxDegree(void) {
    unsigned m = 0;
    for (auto p : m_nodes) m = max(m, p.second->getDegree());
    m_maxDeg = m;
}

double Graph::autoInferIEL(void) {
    m_iel = 2*computeAvgNodeDim();
    return m_iel;
}

double Graph::getIEL(void) {
    if (m_iel == 0) autoInferIEL();
    return m_iel;
}

double Graph::recomputeIEL(void) {
    autoInferIEL();
    return m_iel;
}

double Graph::computeStress(void) {
    // Update the positions in our CFDL object.
    for (auto p : m_nodes) {
        Node_SP &u = p.second;
        id_type id = u->id();
        Point c = u->getCentre();
        m_cfdl->X[m_cgr.id2ix.at(id)] = c.x;
        m_cfdl->Y[m_cgr.id2ix.at(id)] = c.y;
    }
    // Then ask it to compute the stress.
    return m_cfdl->computeStress();
}

ColaGraphRep &Graph::updateColaGraphRep(void) {
    // Only redo the Rectangles and index maps if necessary.
    if (m_needNewRectangles) {
        for (auto r : m_cgr.rs) delete r;
        m_cgr.rs.clear();
        m_cgr.id2ix.clear();
        m_cgr.ix2id.clear();
        size_t i = 0;
        for (auto p : m_nodes) {
            BoundingBox b = p.second->getBoundingBox();
            m_cgr.rs.push_back(new Rectangle(b.x, b.X, b.y, b.Y));
            m_cgr.id2ix.emplace(p.first, i);
            m_cgr.ix2id.emplace(i, p.first);
            ++i;
        }
        // Note that it is now unnecessary to recompute Rectangles.
        m_needNewRectangles = false;
    }
    // We always redo the cola::Edges. They are simple pairs of IDs,
    // and we pass copies around, rather than pointers.
    m_cgr.es.clear();
    for (auto p : m_edges) {
        m_cgr.es.emplace_back(
            m_cgr.id2ix[p.second->getSourceEnd()->id()],
            m_cgr.id2ix[p.second->getTargetEnd()->id()]
        );
    }
    // Update the CFDL too.
    delete m_cfdl;
    m_cfdl = new cola::ConstrainedFDLayout(
        m_cgr.rs, m_cgr.es, m_iel
    );
    // Return
    return m_cgr;
}

cola::RootCluster *Graph::buildRootCluster(const ColaOptions &opts) {
    // Delete the old cluster, if any.
    delete m_cgr.rc;
    // Allocate a new one.
    cola::RootCluster *rc = new cola::RootCluster();
    // Add clusters.
    for (NodesById nodes : opts.nodeClusters) {
        cola::RectangularCluster *c = new cola::RectangularCluster();
        for (auto p : nodes) {
            id_type id = p.first;
            c->addChildNode(m_cgr.id2ix.at(id));
        }
        rc->addChildCluster(c);
    }
    // Store and return.
    m_cgr.rc = rc;
    return rc;
}

void Graph::updateNodesFromRects(bool xAxis, bool yAxis) {
    for (auto p : m_nodes) {
        Rectangle *r = m_cgr.rs.at(m_cgr.id2ix[p.second->id()]);
        Node_SP u = p.second;
        if (xAxis) u->updateXCoordFromRect(r);
        if (yAxis) u->updateYCoordFromRect(r);
    }
}

void Graph::rotate90cw(ColaOptions *opts) {
    // Prepare node map, edge map, sep transform, and pass to generic method.
    auto nodeMap = Compass::getRotationFunction(CardinalDir::EAST, CardinalDir::SOUTH);
    std::function<void(Edge_SP)> edgeMap = [](Edge_SP e)->void{e->rotate90cw();};
    SepTransform st = SepTransform::ROTATE90CW;
    rotate90(nodeMap, edgeMap, st, opts);
}

void Graph::rotate90acw(ColaOptions *opts) {
    // Prepare node map, edge map, sep transform, and pass to generic method.
    auto nodeMap = Compass::getRotationFunction(CardinalDir::EAST, CardinalDir::NORTH);
    std::function<void(Edge_SP)> edgeMap = [](Edge_SP e)->void{e->rotate90acw();};
    SepTransform st = SepTransform::ROTATE90ACW;
    rotate90(nodeMap, edgeMap, st, opts);
}

void Graph::rotate90(PlaneMap nodeMap, std::function<void (Edge_SP)> edgeMap, SepTransform st, ColaOptions *opts) {

    // Set up for logging.
    Logger *logger = opts != nullptr ? opts->logger : nullptr;
    unsigned ln = logger != nullptr ? logger->nextLoggingIndex : 0;
    unsigned lns = 0;
    std::function<void(string)> log = [this, logger](string name)->void{
        if (logger!=nullptr) logger->log(*this, name);
    };

    // Rotate nodes.
    // Note: This just means rotating their coordinates, not flipping their dimensions.
    // The latter would be an operation quite outside what layout is intended to do.
    for (auto p : m_nodes) {
        Node_SP u = p.second;
        Point q = u->getCentre(),
              r = nodeMap(q);
        u->setCentre(r.x, r.y);
    }
    log(string_format("%02d_%02d_rotated_nodes", ln, lns++));
    // Rotate edges (i.e. ask them to rotate their connector routes).
    for (auto p : m_edges) edgeMap(p.second);
    log(string_format("%02d_%02d_rotated_edges", ln, lns++));
    // Rotate constraints.
    m_sepMatrix.transform(st);
    log(string_format("%02d_%02d_rotated_constraints", ln, lns++));
    // And finally destress if requested.
    if (opts != nullptr) {
        if (logger != nullptr) logger->nextLoggingIndex = ln + 1;
        destress(*opts);
    }
}

void Graph::rotate180(void) {
    // Rotate nodes.
    auto f = Compass::getRotationFunction(CardinalDir::EAST, CardinalDir::WEST);
    for (auto p : m_nodes) {
        Node_SP u = p.second;
        Point q = u->getCentre(),
              r = f(q);
        u->setCentre(r.x, r.y);
    }
    // Rotate edges.
    for (auto p : m_edges) p.second->rotate180();
    // Rotate constraints.
    m_sepMatrix.transform(SepTransform::ROTATE180);
}

void Graph::translate(double dx, double dy) {
    // Translate nodes.
    for (auto p : m_nodes) p.second->translate(dx, dy);
    // Rotate edges.
    for (auto p : m_edges) p.second->translate(dx, dy);
}

void Graph::putInBasePosition(void) {
    clearAllRoutes();
    clearAllConstraints();
    size_t z = 0;
    for (auto p : m_nodes) {
        Node_SP u = p.second;
        u->setCentre(z, z);
        ++z;
    }
}

bool Graph::hasSameLayoutAs(const Graph &other, double tol, id_map *idMap) const {
    try {
        // We start by building a lookup of the other Graph's Nodes by the IDs of
        // this Graph's corresponding Nodes.
        NodesById yourNodesByMyIds;
        if (idMap == nullptr) {
            // Build an ID map from the external IDs of this Graph's Nodes
            // to their internal IDs.
            id_map g;
            for (auto p : m_nodes) {
                Node_SP u = p.second;
                g.insert({u->getExternalId(), u->id()});
            }
            // Now we can build the lookup we want.
            for (auto p : other.getNodeLookup()) {
                Node_SP u = p.second;
                yourNodesByMyIds.insert({g.at(u->getExternalId()), u});
            }
        } else {
            for (auto p : other.getNodeLookup()) {
                Node_SP u = p.second;
                yourNodesByMyIds.insert({idMap->at(u->id()), u});
            }
        }
        // Next we need a lookup for the Edges of the other Graph.
        SparseIdMatrix2d<Edge_SP>::type yourEdges = other.getEdgeBySrcIdTgtIdLookup();
        // Check node positions.
        for (auto p : m_nodes) {
            Node_SP u = p.second,
                    v = yourNodesByMyIds.at(u->id());
            Point cu = u->getCentre(),
                  cv = v->getCentre(),
                  dc = cv - cu;
            if (fabs(dc.x) > tol || fabs(dc.y) > tol) return false;
        }
        // Check edge routes.
        for (auto p : m_edges) {
            Edge_SP e = p.second;
            Node_SP s = yourNodesByMyIds.at(e->getSourceEnd()->id()),
                    t = yourNodesByMyIds.at(e->getTargetEnd()->id());
            Edge_SP f = yourEdges.at(s->id()).at(t->id());
            vector<Point> eRoute = e->getRoute(),
                          fRoute = f->getRoute();
            if (eRoute.size() != fRoute.size()) return false;
            auto it = eRoute.begin();
            auto jt = fRoute.begin();
            while (it != eRoute.end()) {
                Point dc = *it - *jt;
                if (fabs(dc.x) > tol || fabs(dc.y) > tol) return false;
                ++it; ++jt;
            }
        }
    } catch (std::out_of_range const&) {
        // We invoke the .at() method of various maps, above.
        // If any of these fails, the two Graphs do not have the same layout.
        return false;
    }
    // If we make it to the end, all tests are passed.
    return true;
}

SparseIdMatrix2d<Edge_SP>::type Graph::getEdgeBySrcIdTgtIdLookup(void) const {
    SparseIdMatrix2d<Edge_SP>::type lookup;
    for (auto p : m_edges) {
        Edge_SP e = p.second;
        Node_SP s = e->getSourceEnd(),
                t = e->getTargetEnd();
        lookup[s->id()][t->id()] = e;
    }
    return lookup;
}

void Graph::destress(void) {
    ColaOptions opts;
    destress(opts);
}

void Graph::destress(const ColaOptions &opts) {

    // Set up for logging.
    Logger *logger = opts.logger;
    unsigned ln = logger != nullptr ? logger->nextLoggingIndex : 0;
    unsigned lns = 0;
    std::function<void(Graph&, string)> log = [logger](Graph &H, string name)->void{
        if (logger!=nullptr) logger->log(H, name);
    };

    // Handle the version with solidified edges separately:
    if (opts.solidifyAlignedEdges) {
        // Copy the opts and deactivate the aligned edges option to avoid infinite loop.
        ColaOptions opts2(opts);
        opts2.solidifyAlignedEdges = false;
        // Also make sure the options request makeFeasible, which should always be done first,
        // so that any overlaps between edgenodes are resolved in the right way.
        // Don't use any extra border.
        opts2.makeFeasible = true;
        opts2.makeFeasible_xBorder = 0;
        opts2.makeFeasible_yBorder = 0;
        // Make two copies of this graph, with solidified edges in each dimension, and destress there.
        // Since the new Graphs point to the same Nodes, the layout is successfully updated automatically.
        Graph Hx(*this), Hy(*this);

        // Work in x-dimension, with solid vertical edges:
        Hy.solidifyAlignedEdges(vpsc::YDIM, opts2);
        log(Hy, string_format("%02d_%02d_with_solid_V_edges", ln, lns++));
        opts2.xAxis = true;
        opts2.yAxis = false;
        Hy.destress(opts2);
        log(Hy, string_format("%02d_%02d_destressed_with_solid_V_edges", ln, lns++));

        // Work in y-dimension, with solid horizontal edges:
        Hx.solidifyAlignedEdges(vpsc::XDIM, opts2);
        log(Hx, string_format("%02d_%02d_with_solid_H_edges", ln, lns++));
        opts2.xAxis = false;
        opts2.yAxis = true;
        Hx.destress(opts2);
        log(Hx, string_format("%02d_%02d_destressed_with_solid_H_edges", ln, lns++));

    } else {
        // Make sure our rectangles etc. are up to date.
        updateColaGraphRep();
        // Set up the clustering, if any.
        buildRootCluster(opts);
        // Set up the ideal edge length.
        double iel = opts.idealEdgeLength;
        if (iel == 0) iel = getIEL();
        // If using neighbour stress, apply the scalar.
        if (opts.useNeighbourStress) iel *= opts.nbrStressIELScalar;
        // Set up the constraints.
        // Start with a copy of those given.
        cola::CompoundConstraints ccs(opts.ccs);
        // And add the SepMatrix.
        // (We cannot simply add the SepMatrix to the given vector, since this would lead to redundant
        //  constraints if the same ColaOptions object was used again.)
        ccs.push_back(&m_sepMatrix);
        // Construct and run the layout object.
        if (opts.useMajorization) {
            // We use ConstrainedMajorizationLayout.
            cola::ConstrainedMajorizationLayout alg(
                        m_cgr.rs, m_cgr.es, m_cgr.rc, iel,
                        opts.eLengths, opts.doneTest, opts.preIteration, opts.useNeighbourStress
            );

            alg.setAvoidOverlaps(opts.preventOverlaps);
            alg.setScaling(opts.useScaling);
            alg.setConstraints(&ccs);
            alg.run(opts.xAxis, opts.yAxis);
        } else {
            // We use ConstrainedFDLayout.
            cola::ConstrainedFDLayout alg(
                        m_cgr.rs, m_cgr.es, iel, opts.eLengths, opts.doneTest, opts.preIteration
            );
            alg.setAvoidNodeOverlaps(opts.preventOverlaps);
            alg.setUseNeighbourStress(opts.useNeighbourStress);
            alg.setConstraints(ccs);
            alg.setClusterHierarchy(m_cgr.rc);
            if (opts.makeFeasible) alg.makeFeasible(opts.makeFeasible_xBorder, opts.makeFeasible_yBorder);
            alg.run(opts.xAxis, opts.yAxis);
        }
        // Update node positions.
        updateNodesFromRects(opts.xAxis, opts.yAxis);
    }
}

string Graph::writeId2Ix(void) const {
    ostringstream ss;
    for (auto p : m_cgr.id2ix) ss << p.first << ": " << p.second << std::endl;
    return ss.str();
}

string Graph::writeIx2Id(void) const {
    ostringstream ss;
    for (auto p : m_cgr.ix2id) ss << p.first << ": " << p.second << std::endl;
    return ss.str();
}

void Graph::makeFeasible(const ColaOptions &opts) {

    // Set up for logging.
    Logger *logger = opts.logger;
    unsigned ln = logger != nullptr ? logger->nextLoggingIndex : 0;
    unsigned lns = 0;
    std::function<void(Graph&, string)> log = [logger](Graph &H, string name)->void{
        if (logger!=nullptr) logger->log(H, name);
    };

    // Handle the version with solidified edges separately:
    if (opts.solidifyAlignedEdges) {
        // Copy the opts and deactivate the aligned edges option to avoid infinite loop.
        ColaOptions opts2(opts);
        opts2.solidifyAlignedEdges = false;
        // Make a copy of this graph, with solidified edges in both dimensions, and make feasible there.
        Graph H(*this);
        H.solidifyAlignedEdges(vpsc::XDIM, opts2);
        //log(H, string_format("%02d_%02d_solid_H_edges", ln, lns++));
        H.solidifyAlignedEdges(vpsc::YDIM, opts2);
        log(H, string_format("%02d_%02d_solid_H_and_V_edges", ln, lns++));
        H.makeFeasible(opts2);
    } else {
        // Make sure our rectangles etc. are up to date.
        updateColaGraphRep();
        // Set up the clustering, if any.
        buildRootCluster(opts);
        // Set up the ideal edge length.
        // While makeFeasible only does projection (so IEL is irrelevant), we do need to
        // construct a CFDL object to do it, which requires an IEL.
        double iel = opts.idealEdgeLength;
        if (iel == 0) iel = getIEL();
        // Set up the constraints.
        // Start with a copy of those given.
        cola::CompoundConstraints ccs(opts.ccs);
        // And add the SepMatrix.
        ccs.push_back(&m_sepMatrix);
        // Construct the layout object and ask it to make feasible.
        cola::ConstrainedFDLayout alg(
            m_cgr.rs, m_cgr.es, iel, opts.eLengths, opts.doneTest, opts.preIteration
        );
        alg.setAvoidNodeOverlaps(opts.preventOverlaps);
        alg.setConstraints(ccs);
        alg.setClusterHierarchy(m_cgr.rc);
        alg.makeFeasible(opts.makeFeasible_xBorder, opts.makeFeasible_yBorder);
        // Update node positions.
        updateNodesFromRects(opts.xAxis, opts.yAxis);
    }
}

int Graph::project(const ColaOptions &opts, vpsc::Dim dim, int accept) {
    // Handle the version with solidified edges separately:
    if (opts.solidifyAlignedEdges) {
        // Copy the opts and deactivate the aligned edges option to avoid infinite loop.
        ColaOptions opts2(opts);
        opts2.solidifyAlignedEdges = false;
        // Make a copy of this graph, with solidified edges, and project there.
        // Since the new Graph points to the same Nodes, the layout is successfully updated automatically.
        Graph H(*this);
        H.solidifyAlignedEdges(vpsc::conjugate(dim), opts2);
        return H.project(opts2, dim, accept);
    } else {
        // Make sure our rectangles etc. are up to date.
        updateColaGraphRep();
        // Set up the constraints.
        // Start with a copy of those given.
        cola::CompoundConstraints ccs(opts.ccs);
        // And add the SepMatrix.
        ccs.push_back(&m_sepMatrix);
        // Project.
        unsigned debugLevel = m_projectionDebugLevel;
        cola::ProjectionResult result = cola::projectOntoCCs(dim, m_cgr.rs, ccs, opts.preventOverlaps, accept, debugLevel);
        if (debugLevel > 0) {
            std::string usi = result.unsatinfo;
            size_t n = usi.length();
            if (n > 77) { // Header is 77 chars. String is longer iff has actual unsat constraints.
                // Show ix2id map.
                std::cout << "VPSC var indices to Node Ids:\n" << std::endl;
                for (auto p : m_cgr.ix2id) std::cout << p.first << " --> " << p.second << "\n";
                // Show graph.
                writeStringToFile(writeTglf(), m_debugOutputPath+"failed_projection.tglf");
            }
            std::cout << result.unsatinfo << std::endl;
        }
        int errorLevel = result.errorLevel;
        // Accept new node positions conditionally:
        if (errorLevel <= accept) updateNodesFromRects();
        // Return the error level.
        return errorLevel;
    }
}

int Graph::projectOntoSepCo(const ColaOptions &opts, SepCo_SP sepco, int accept) {
    updateColaGraphRep();
    ColaOptions opts2(opts);
    sepco->generateColaConstraints(m_cgr, opts2.ccs);
    return project(opts2, sepco->dim, accept);
}

bool Graph::applyProjSeq(const ColaOptions &opts, ProjSeq &ps, int accept) {
    updateColaGraphRep();
    // Set up the ideal edge length.
    double iel = opts.idealEdgeLength;
    if (iel == 0) iel = getIEL();
    // Do stress computations iff iel >= 0.
    double lastStress = 0.0;
    if (iel >= 0) lastStress = computeStress();
    // We will check whether all projections work.
    bool allOK = true;
    // Get first projection.
    Projection_SP proj = ps.nextProjection();
    // Keep going until there are no more.
    while (proj != nullptr) {
        // Act only if projection is nonempty.
        if (proj->size() > 0) {
            // Copy the options and set the constraints in the copy.
            ColaOptions projOpts(opts);
            projOpts.ccs = proj->generateColaConstraints(m_cgr);
            // Do the projection.
            int result = project(projOpts, proj->dim, accept);
            // Clean up.
            for (cola::CompoundConstraint *cc : projOpts.ccs) delete cc;
            // If result not acceptable, note this.
            if (result > accept) allOK = false;
            // Do stress computation if desired.
            if (iel >= 0) {
                double stress = computeStress();
                double dS = stress - lastStress;
                ps.noteStresschange(dS);
                lastStress = stress;
            }
            // Now break if the projection didn't work.
            if (!allOK) break;
        }
        // Get the next projection.
        proj = ps.nextProjection();
    }
    // Report whether all projections worked or not.
    return allOK;
}

void Graph::solidifyAlignedEdges(vpsc::Dim dim, const ColaOptions &opts) {
    // For the nodes representing aligned Edges, we avoid making rectangles that would be shorter
    // than a certain minimal length. (If they are zero length, VPSC is unhappy.)
    // Constants have been experimnetally determined.
    const double MIN_LENGTH = 0.01;
    const double GAP = 1;
    // Exemptions may be passed in the ColaOptions, in the form of an EdgesById lookup.
    // Thus have two ordered lookups of Edges to compare, so iterate properly for linear time complexity.
    auto it = m_edges.begin();
    auto jt = opts.solidEdgeExemptions.cbegin();
    auto exemptionsEnd = opts.solidEdgeExemptions.cend();
    while (it != m_edges.end()) {
        auto p = *it;
        if (jt != exemptionsEnd) {
            auto q = *jt;
            id_type id = p.first,
                    jd = q.first;
            if (id > jd) {
                ++jt;
                continue;
            } else if (id == jd) {
                // This edge is exempt.
                ++it;
                continue;
            }
        }
        ++it;
        // Otherwise the edge is not exempt.
        Edge_SP &e = p.second;
        Node_SP src = e->getSourceEnd(),
                tgt = e->getTargetEnd();
        id_type sid = src->id(),
                tid = tgt->id();
        bool edgeIsAligned = dim == vpsc::HORIZONTAL ?
                    m_sepMatrix.areHAligned(sid, tid) :
                    m_sepMatrix.areVAligned(sid, tid);
        if (edgeIsAligned) {
            // Form the "edgenode".
            Node_SP edgeNode = Node::allocate();
            // Get local nodes of matching IDs.
            // This is important so that we get the latest coordinates.
            // (When working with GhostNodes, there may be different, older Nodes,
            // at the ends of the Edge itself.)
            src = m_nodes.at(sid);
            tgt = m_nodes.at(tid);
            // Get bounding boxes and centres.
            BoundingBox bsrc = src->getBoundingBox(),
                        btgt = tgt->getBoundingBox();
            Point csrc = src->getCentre(),
                  ctgt = tgt->getCentre();
            // We'll need to set the direction and gap for the constraints.
            SepDir sd;
            double srcGap, tgtGap;
            // Set position and dimensions for edgenode, and determine the cardinal direction.
            if (dim == vpsc::VERTICAL) {
                double h = min(
                    fabs(btgt.y - bsrc.Y), fabs(bsrc.y - btgt.Y)
                ) - 2*GAP;
                if (h < MIN_LENGTH) continue;
                edgeNode->setDims(m_edge_thickness, h);
                double cx = (csrc.x + ctgt.x) / 2.0,
                       cy = min(bsrc.Y, btgt.Y) + h/2.0 + GAP;
                edgeNode->setCentre(cx, cy);
                if (csrc.y < ctgt.y) {
                    sd = SepDir::SOUTH;
                    srcGap = cy - csrc.y;
                    tgtGap = ctgt.y - cy;
                } else {
                    sd = SepDir::NORTH;
                    srcGap = csrc.y - cy;
                    tgtGap = cy - ctgt.y;
                }
            } else {
                double w = min(
                    fabs(btgt.x - bsrc.X), fabs(bsrc.x - btgt.X)
                ) - 2*GAP;
                if (w < MIN_LENGTH) continue;
                edgeNode->setDims(w, m_edge_thickness);
                double cy = (csrc.y + ctgt.y) / 2.0,
                       cx = min(bsrc.X, btgt.X) + w/2.0 + GAP;
                edgeNode->setCentre(cx, cy);
                if (csrc.x < ctgt.x) {
                    sd = SepDir::EAST;
                    srcGap = cx - csrc.x;
                    tgtGap = ctgt.x - cx;
                } else {
                    sd = SepDir::WEST;
                    srcGap = csrc.x - cx;
                    tgtGap = cx - ctgt.x;
                }
            }
            // Add the node to the graph.
            addNode(edgeNode);
            // Set constraints.
            // NB: It is very important that we free the constraint on src and tgt in addition
            // to setting the new constraints on src and the edgeNode, and on tgt and edgeNode.
            // Otherwise we will pass redundant equality constraints to VPSC, which will report
            // an unsatisfiability.
            id_type sid = src->id(),
                    tid = tgt->id(),
                    eid = edgeNode->id();
            m_sepMatrix.free(sid, tid);
            // NB: Another very important implementation detail is that we use CENTRE gaps
            // here, not BDRY gaps. For if the user has set an extra boundary gap in the SepMatrix,
            // then the gaps will be made much larger than we want.
            // Objection: But the gap is irrelevant because when using solid vertical edges
            // we only project in the x-dimension, and vice versa.
            // Reply: That is /one/ way in which solid edges are used, in particular when they are
            // used by the Graph's destress method. However there are other times when both vertical and
            // horizontal edges are used in conjunction, while projecting in both dimensions; for example,
            // this happens in the Graph's makeFeasible method.
            m_sepMatrix.addSep(sid, eid, GapType::CENTRE, sd, SepType::INEQ, srcGap);
            m_sepMatrix.addSep(eid, tid, GapType::CENTRE, sd, SepType::INEQ, tgtGap);
        }
    }
}

NodesById Graph::buildUniqueBendPoints(void) {
    // Initialise return value.
    NodesById bendNodes;
    // We want to be able to find nearby nodes, since we'll consider any within
    // half-integer distance to be the same.
    NearbyObjectFinder<Node_SP> nof(0.5);
    // Consider all Edges in the Graph:
    for (auto pair : m_edges) {
        Edge_SP &e = pair.second;
        vector<Point> route = e->getRoute();
        // If the route does not contain at least three points, then it has no bends,
        // so we skip this Edge.
        size_t N = route.size();
        if (N < 3) continue;
        // Otherwise we need to prepare the vector of bend Nodes on this Edge's route.
        Nodes routeBends;
        // For display/testing purposes, we want the bend nodes we create to
        // look smaller than the average node in the graph.
        double bendNodeSize = getIEL()/8.0;
        // Consider the interior points of the route.
        for (size_t j = 1; j < N - 1; ++j) {
            Point p = route[j];
            // Do we already have a bend node near enough to this point?
            Node_SP bend = nof.findObject(p.x, p.y);
            // If not, we will create a new bend node.
            if (bend == nullptr) {
                bend = Node::allocate();
                // Set its position and dimensions.
                bend->setCentre(p.x, p.y);
                bend->setDims(bendNodeSize, bendNodeSize);
                // Record it in the lookup.
                bendNodes[bend->id()] = bend;
                // And put it in the object finder.
                nof.addObject(p.x, p.y, bend);
            }
            // Whether we created a new bend node, or found an existing one near enough to the
            // given route point, add the bend to the Edge's route.
            routeBends.push_back(bend);
        }
        // Set the route bends in the Edge.
        e->setBendNodes(routeBends);
    }
    // Return.
    return bendNodes;
}

void Graph::pushNodePositions(void) {
    std::map<id_type, Point> positions;
    for (auto p : m_nodes) {
        Node_SP &u = p.second;
        id_type id = u->id();
        Point c = u->getCentre();
        positions.insert({id, c});
    }
    m_posStack.push(positions);
}

bool Graph::popNodePositions(void) {
    // If stack is empty, return false.
    if (m_posStack.empty()) return false;
    // Otherwise restore Node positions based on top element on stack.
    std::map<id_type, Point> &positions = m_posStack.top();
    for (auto p : m_nodes) {
        Node_SP &u = p.second;
        id_type id = u->id();
        Point c = positions.at(id);
        u->setCentre(c.x, c.y);
    }
    // Pop the top element and return true.
    m_posStack.pop();
    return true;
}

void Graph::padAllNodes(double dw, double dh) {
    for (auto p : m_nodes) {
        Node_SP u = p.second;
        u->addPadding(dw, dh);
    }
}

void Graph::setPosesInCorrespNodes(Graph &H) {
    NodesById nodes_H = H.getNodeLookup();
    auto it = m_nodes.begin();
    auto jt = nodes_H.begin();
    while (it != m_nodes.end() && jt != nodes_H.end()) {
        auto p = *it;
        auto q = *jt;
        id_type i = p.first,
                j = q.first;
        if (i > j) ++jt;
        else {
            if (i == j) {
                Point c = p.second->getCentre();
                q.second->setCentre(c.x, c.y);
            }
            ++it;
        }
    }
}

void Graph::padCorrespNodes(Graph &H, double dw, double dh, const NodesById &ignore) {
    NodesById nodes_G = getNodeLookupWithIgnore(ignore);
    NodesById nodes_H = H.getNodeLookup();
    auto it = nodes_G.begin();
    auto jt = nodes_H.begin();
    while (it != nodes_G.end() && jt != nodes_H.end()) {
        auto p = *it;
        auto q = *jt;
        id_type i = p.first,
                j = q.first;
        if (i > j) ++jt;
        else {
            if (i == j) {
                q.second->addPadding(dw, dh);
            }
            ++it;
        }
    }
}

void Graph::setRoutesInCorrespEdges(Graph &H, bool directed) {
    // First build a lookup for Edges in H by [srcID][tgtID].
    SparseIdMatrix2d<Edge_SP>::type lookup_H;
    EdgesById edges_H = H.getEdgeLookup();
    for (auto p : edges_H) {
        Edge_SP f = p.second;
        id_type sid = f->getSourceEnd()->id(),
                tid = f->getTargetEnd()->id();
        lookup_H[sid][tid] = f;
    }
    // Now iterate over the Edges of this Graph, setting routes where
    // a match is found.
    for (auto p : m_edges) {
        Edge_SP e = p.second;
        id_type sid = e->getSourceEnd()->id(),
                tid = e->getTargetEnd()->id();
        // See if H has an Edge with same src and tgt.
        Edge_SP f = lookup_H[sid][tid];
        bool reversed = false;
        // If not, and if we're considering edges to be undirected, then
        // we get to try again, with the IDs swapped.
        if (f == nullptr && !directed) {
            f = lookup_H[tid][sid];
            // In this case we'll have to reverse the order of the route points.
            reversed = true;
        }
        // If we managed to find an Edge, set its route.
        if (f != nullptr) {
            vector<Point> e_route = e->getRoute();
            if (reversed) std::reverse(e_route.begin(), e_route.end());
            f->setRoute(e_route);
        }
    }
}

void Graph::clearAllRoutes(void) {
    for (auto p : m_edges) {
        Edge_SP e = p.second;
        e->clearRouteAndBends();
    }
}

void Graph::buildRoutes(void) {
    for (auto p : m_edges) {
        Edge_SP e = p.second;
        e->buildRouteFromBends();
    }
}

void Graph::route(Avoid::RouterFlag routingType) {
    clearAllRoutes();
    RoutingAdapter ra(routingType);
    ra.addNodes(m_nodes);
    ra.addEdges(m_edges);
    ra.route();
}

void Graph::addBendlessSubnetworkToRoutingAdapter(RoutingAdapter &ra) {
    ra.addNodes(m_nodes);
    EdgesById edges;
    for (auto p : m_edges) if (!p.second->hasBendNodes()) edges.insert(p);
    ra.addEdges(edges);
}

void Graph::clearAllConstraints(void) {
    m_sepMatrix.clear();
}

void Graph::setCorrespondingConstraints(Graph &H) {
    m_sepMatrix.setCorrespondingConstraints(H.getSepMatrix());
}

void Graph::transformClosedSubset(SepTransform tf, const std::set<id_type> &ids) {
    m_sepMatrix.transformClosedSubset(tf, ids);
}

void Graph::transformOpenSubset(SepTransform tf, const std::set<id_type> &ids) {
    m_sepMatrix.transformOpenSubset(tf, ids);
}
