/*
 * vim: ts=4 sw=4 et tw=0 wm=0
 *
 * libavoid - Fast, Incremental, Object-avoiding Line Router
 *
 * Copyright (C) 2011-2013  Monash University
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * See the file LICENSE.LGPL distributed with the library.
 *
 * Licensees holding a valid commercial license may use this file in
 * accordance with the commercial license agreement provided with the 
 * library.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  
 *
 * Author(s):   Michael Wybrow <mjwybrow@users.sourceforge.net>
*/

#ifndef AVOID_HYPEREDGETREE_H
#define AVOID_HYPEREDGETREE_H

#include <cstdio>
#include <list>
#include <map>
#include <set>
#include <utility>

#include "libavoid/geomtypes.h"

namespace Avoid {

// These classes are not intended for public use.
// They are used to represent a hyperedge as a tree that certain
// transformations can be easily performed on to improve the routing
// of the hyperedge.

class JunctionRef;
class ConnRef;
class HyperEdgeShiftSegment;
class VertInf;
class Router;

struct HyperEdgeTreeEdge;
struct HyperEdgeTreeNode;

typedef std::map<JunctionRef *, HyperEdgeTreeNode *>
        JunctionHyperEdgeTreeNodeMap;
typedef std::set<JunctionRef *> JunctionSet;
typedef std::list<JunctionRef *> JunctionRefList;
typedef std::list<ConnRef *> ConnRefList;

class CmpNodesInDim;

typedef std::set<HyperEdgeTreeNode *, CmpNodesInDim> OrderedHENodeSet;

struct HyperEdgeTreeNode
{
    HyperEdgeTreeNode();
    ~HyperEdgeTreeNode();

    void deleteEdgesExcept(HyperEdgeTreeEdge *ignored);
    void removeOtherJunctionsFrom(HyperEdgeTreeEdge *ignored, 
            JunctionSet &treeRoots);
    void outputEdgesExcept(FILE *fp, HyperEdgeTreeEdge *ignored);
    static HyperEdgeTreeNode *moveJunctionAlongCommonEdge(
            HyperEdgeTreeNode *self);
    void disconnectEdge(HyperEdgeTreeEdge *edge);
    void spliceEdgesFrom(HyperEdgeTreeNode *oldNode);
    void removeZeroLengthEdges(HyperEdgeTreeEdge *ignored);
    void writeEdgesToConns(HyperEdgeTreeEdge *ignored, size_t pass);
    void addConns(HyperEdgeTreeEdge *ignored, Router *router, 
            ConnRefList& oldConns, ConnRef *conn);
    void listJunctionsAndConnectors(HyperEdgeTreeEdge *ignored,
            JunctionRefList& junctions, ConnRefList& connectors);

    std::list<HyperEdgeTreeEdge *> edges;
    JunctionRef *junction;
    Point point;
    OrderedHENodeSet *shiftSegmentNodeSet;
    VertInf *finalVertex;
    bool isConnectorSource;
};

struct HyperEdgeTreeEdge
{
    HyperEdgeTreeEdge(HyperEdgeTreeNode *node1, HyperEdgeTreeNode *node2,
            ConnRef *conn);

    HyperEdgeTreeNode *followFrom(HyperEdgeTreeNode *from) const;
    bool zeroLength(void) const;
    void splitFromNodeAtPoint(HyperEdgeTreeNode *source, const Point& point);
    bool hasOrientation(const size_t dimension) const;
    void outputNodesExcept(FILE *file, HyperEdgeTreeNode *ignored);
    void removeZeroLengthEdges(HyperEdgeTreeNode *ignored);
    void deleteNodesExcept(HyperEdgeTreeNode *ignored);
    void removeOtherJunctionsFrom(HyperEdgeTreeNode *ignored, 
            JunctionSet &treeRoots);
    void writeEdgesToConns(HyperEdgeTreeNode *ignored, size_t pass);
    void addConns(HyperEdgeTreeNode *ignored, Router *router,
            ConnRefList& oldConns);
    void disconnectEdge(void);
    void replaceNode(HyperEdgeTreeNode *oldNode,
            HyperEdgeTreeNode *newNode);
    void listJunctionsAndConnectors(HyperEdgeTreeNode *ignored,
            JunctionRefList& junctions, ConnRefList& connectors);

    std::pair<HyperEdgeTreeNode *, HyperEdgeTreeNode *> ends;
    ConnRef *conn;
};


typedef std::map<VertInf *, HyperEdgeTreeNode *> VertexNodeMap;


class CmpNodesInDim
{
    public:
        CmpNodesInDim(const size_t dim);
        bool operator()(const HyperEdgeTreeNode *lhs,
                const HyperEdgeTreeNode *rhs) const;
    private:
        const size_t m_dimension;
};

}

#endif
