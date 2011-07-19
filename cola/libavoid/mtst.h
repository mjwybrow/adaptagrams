/*
 * vim: ts=4 sw=4 et tw=0 wm=0
 *
 * libavoid - Fast, Incremental, Object-avoiding Line Router
 *
 * Copyright (C) 2011  Monash University
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

#ifndef AVOID_MTST_H
#define AVOID_MTST_H

#include <cstdio>
#include <set>
#include <list>

#include "libavoid/hyperedgetree.h"


namespace Avoid {

class VertInf;
class Router;
class ConnRef;
class EdgeInf;

typedef std::list<VertexSet> VertexSetList;

// This class is not intended for public use.
// It is used by the hyperedge routing code to build a minimum terminal
// spanning tree for a set of terminal vertices.
class MinimumTerminalSpanningTree
{
    public:
        MinimumTerminalSpanningTree(Router *router,
                std::set<VertInf *> terminals,
                JunctionHyperEdgeTreeNodeMap *hyperEdgeTreeJunctions = NULL);
        void setDebuggingOutput(FILE *fp, unsigned int counter);
        void execute(void);
        HyperEdgeTreeNode *rootJunction(void) const;

    private:
        void buildHyperEdgeTreeToRoot(VertInf *curr,
                HyperEdgeTreeNode *prevNode);

        void makeSet(VertInf *vertex);
        VertexSetList::iterator findSet(VertInf *vertex);
        void unionSets(VertexSetList::iterator s1, VertexSetList::iterator s2);
        HyperEdgeTreeNode *addNode(VertInf *vertex, HyperEdgeTreeNode *prevNode);

        Router *router;
        std::set<VertInf *> terminals;
        JunctionHyperEdgeTreeNodeMap *hyperEdgeTreeJunctions;

        VertexNodeMap nodes;
        HyperEdgeTreeNode *m_rootJunction;
        double bendCost;
        VertexSetList allsets;
        std::list<VertInf *> extraVertices;
        std::list<EdgeInf *> extraEdges;

        FILE *debug_fp;
        unsigned int debug_count;
};


}

#endif
