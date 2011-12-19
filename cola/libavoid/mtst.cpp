/*
 * vim: ts=4 sw=4 et tw=0 wm=0
 *
 * libavoid - Fast, Incremental, Object-avoiding Line Router
 *
 * Copyright (C) 2011  Monash University
 *
 * --------------------------------------------------------------------
 * Computation of the Minimum Terminal Spanning Tree is based upon the 
 * method described in Section IV.B of:
 *     Long, J., Zhou, H., Memik, S.O. (2008). EBOARST: An efficient 
 *     edge-based obstacle-avoiding rectilinear Steiner tree construction 
 *     algorithm. IEEE Trans. on Computer-Aided Design of Integrated 
 *     Circuits and Systems 27(12), pages 2169–2182.
 * --------------------------------------------------------------------
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

#include <cfloat>
#include <vector>
#include <algorithm>


#include "libavoid/hyperedgetree.h"
#include "libavoid/router.h"
#include "libavoid/mtst.h"
#include "libavoid/vertices.h"
#include "libavoid/timer.h"
#include "libavoid/junction.h"

namespace Avoid {

// Comparison for the vertex heap in the extended Dijkstra's algorithm.
struct HeapCmpVertInf
{
    bool operator()(const VertInf *a, const VertInf *b) const
    {
        return a->sptfDist > b->sptfDist;
    }
};


// Comparison for the bridging edge heap in the extended Kruskal's algorithm.
struct CmpEdgeInf
{
    bool operator()(const EdgeInf *a, const EdgeInf *b) const
    {
        return a->mtstDist() > b->mtstDist();
    }
};


struct delete_object
{
    template <typename T>
    void operator()(T *ptr){ delete ptr;}
};


MinimumTerminalSpanningTree::MinimumTerminalSpanningTree(Router *router,
        std::set<VertInf *> terminals,
        JunctionHyperEdgeTreeNodeMap *hyperEdgeTreeJunctions)
    : router(router),
      terminals(terminals),
      hyperEdgeTreeJunctions(hyperEdgeTreeJunctions),
      m_rootJunction(NULL),
      bendCost(2000),
      debug_fp(NULL),
      debug_count(0)
{

}

void MinimumTerminalSpanningTree::setDebuggingOutput(FILE *fp,
        unsigned int counter)
{
    debug_fp = fp;
    debug_count = counter;
}


HyperEdgeTreeNode *MinimumTerminalSpanningTree::rootJunction(void) const
{
    return m_rootJunction;
}


void MinimumTerminalSpanningTree::makeSet(VertInf *vertex)
{
    VertexSet newSet;
    newSet.insert(vertex);
    allsets.push_back(newSet);
}

VertexSetList::iterator MinimumTerminalSpanningTree::findSet(VertInf *vertex)
{
    for (VertexSetList::iterator it = allsets.begin();
            it != allsets.end(); ++it)
    {
        if (it->find(vertex) != it->end())
        {
            return it;
        }
    }
    return allsets.end();
}

void MinimumTerminalSpanningTree::unionSets(VertexSetList::iterator s1,
        VertexSetList::iterator s2)
{
    std::set<VertInf *> s = *s1;
    s.insert(s2->begin(), s2->end());

    allsets.erase(s1);
    allsets.erase(s2);
    allsets.push_back(s);
}

HyperEdgeTreeNode *MinimumTerminalSpanningTree::addNode(VertInf *vertex, 
        HyperEdgeTreeNode *prevNode)
{
    // Do we already have a node for this vertex?
    VertexNodeMap::iterator match = nodes.find(vertex);
    if (match == nodes.end())
    {
        // Not found.  Create new node.
        HyperEdgeTreeNode *newNode = new HyperEdgeTreeNode();
        newNode->point = vertex->point;
        // Remember it.
        nodes[vertex] = newNode;
        // Join it to the previous node.
        new HyperEdgeTreeEdge(prevNode, newNode, NULL);

        return newNode;
    }
    else
    {
        // Found.
        HyperEdgeTreeNode *junctionNode = match->second;
        if (junctionNode->junction == NULL)
        {
            // Create a junction, if one has not already been created.
            junctionNode->junction = new JunctionRef(router, vertex->point);
            if (m_rootJunction == NULL)
            {
                // Remember the first junction node, so we can use it to 
                // traverse the tree, added and connecting connectors to 
                // junctions and endpoints.
                m_rootJunction = junctionNode;
            }
            router->removeObjectFromQueuedActions(junctionNode->junction);
            junctionNode->junction->makeActive();
        }
        // Joint to junction
        new HyperEdgeTreeEdge(prevNode, junctionNode, NULL);

        return NULL;
    }
}

void MinimumTerminalSpanningTree::buildHyperEdgeTreeToRoot(VertInf *currVert,
        HyperEdgeTreeNode *prevNode)
{
    COLA_ASSERT(currVert != NULL);

    // This method follows branches in a shortest path tree back to the
    // root, generating hyperedge tree nodes and branches as it goes.
    while (currVert)
    {
        // Add the node, if necessary.
        HyperEdgeTreeNode *addedNode = addNode(currVert, prevNode);
        
        if (debug_fp)
        {
            fprintf(debug_fp, "<path d=\"M %g %g L %g %g\" "
                    "style=\"fill: none; stroke: %s; "
                    "stroke-width: 1px;\" />\n",
                    currVert->point.x, currVert->point.y,
                    prevNode->point.x, prevNode->point.y, "blue");
        }
        
        if (addedNode == NULL)
        {
            // We've reached a junction, so stop.
            break;
        }

        if (currVert->pathNext == NULL)
        {
            // This is a terminal of the hyperedge, mark the node with the 
            // vertex representing the endpoint of the connector so we can
            // later use this to set the correct ConnEnd for the connector.
            addedNode->finalVertex = currVert;
        }
        prevNode = addedNode;
        currVert = currVert->pathNext;
    }
}


void MinimumTerminalSpanningTree::execute(void)
{
    // Perform extended Dijkstra's algorithm
    // =====================================
    //
    router->timers.Register(tmHyperedgeForest, timerStart);
    bool isOrthogonal = true;

    // Vertex heap for extended Dijkstra's algorithm.
    std::vector<VertInf *> vHeap;
    HeapCmpVertInf vHeapCompare;

    // Bridging edge heap for the extended Kruskal's algorithm.
    std::vector<EdgeInf *> beHeap;
    CmpEdgeInf beHeapCompare;

    // Initialisation
    //
    VertInf *endVert = router->vertices.end();
    for (VertInf *k = router->vertices.connsBegin(); k != endVert;
            k = k->lstNext)
    {
        if (terminals.find(k) != terminals.end())
        {
            // This is a terminal, set a distance of zero.
            k->sptfDist = 0;
            makeSet(k);
            vHeap.push_back(k);
        }
        else
        {
            // This is a non-terminal vertex.  Assign it the maximum distance.
            k->sptfDist = DBL_MAX;
        }
        k->pathNext = NULL;
        k->sptfRoot = k;
    }
    std::make_heap(vHeap.begin(), vHeap.end(), vHeapCompare);
    
    // Shortest path terminal forest construction
    //
    const VertID dimensionChangeVertexID(0, 42);
    if (debug_fp)
    {
        fprintf(debug_fp, "<g inkscape:groupmode=\"layer\" "
                "style=\"display: none;\" "
                "inkscape:label=\"MTST-%o2u-F\""
                ">\n", debug_count);
    }
    while ( ! vHeap.empty() )
    {
        // Take the lowest vertex from heap.
        VertInf *u = vHeap.front();

        // Pop the lowest vertex off the heap.
        std::pop_heap(vHeap.begin(), vHeap.end(), vHeapCompare);
        vHeap.pop_back();

        // For each edge from this vertex...
        EdgeInfList& visList = (!isOrthogonal) ? u->visList : u->orthogVisList;
        EdgeInfList::const_iterator finish = visList.end();
        VertInf *extraVertex = NULL;
        for (EdgeInfList::const_iterator edge = visList.begin(); 
                edge != finish; ++edge)
        {
            VertInf *v = (*edge)->otherVert(u);
            double edgeDist = (*edge)->getDist();

            // Assign a distance (length) of 1 for dummy visibility edges
            // which may not accurately reflect the real distance of the edge.
            if (v->id.isDummyPinHelper() || u->id.isDummyPinHelper())
            {
                edgeDist = 1;
            }

            // Ignore an edge we have already explored.
            if (u->pathNext == v || 
                    (u->pathNext && u->pathNext->pathNext == v))
            {
                continue;
            }

            // Don't do anything more here if this is an intra-tree edge that
            // would just bridge branches of the same tree.
            if (u->sptfRoot == v->sptfRoot)
            {
                continue;
            }

            // This is an extension to the original method that takes a bend
            // cost into account.  When edges from this node, we take into
            // account the direction of the branch in the tree that got us
            // here.  For an edge colinear to this we do the normal thing,
            // and add it to the heap.  For edges at right angle, we don't
            // immediately add these, but instead add a dummy segment and node
            // at the current position and give the edge a distance equal to
            // the bend penalty.  We add equivalent edges for the right-angled
            // original edges, so these may be explored when the algorithm
            // explores the dummy node.  Obviously we also need to clean up
            // these dummy nodes and edges later.
            double newCost = (u->sptfDist + edgeDist);
            if (u->pathNext && ! colinear(u->pathNext->point, 
                    u->point, v->point))
            {
                // This edge is not colinear, so add it to the dummy node and
                // ignore it.
                COLA_ASSERT(u->id != dimensionChangeVertexID);
                if ( ! extraVertex )
                {
                    // Create the dummy node if necessary.
                    extraVertex = new VertInf(router, dimensionChangeVertexID,
                           u->point, false);
                    extraVertices.push_back(extraVertex);
                    extraVertex->sptfDist = bendCost + u->sptfDist;
                    extraVertex->pathNext = u;
                    extraVertex->sptfRoot = u->sptfRoot;
                    vHeap.push_back(extraVertex);
                    std::push_heap(vHeap.begin(), vHeap.end(), vHeapCompare);
                }
                // Add a copy of the ignored edge to the dummy node, so it
                // may be explored later.
                EdgeInf *extraEdge = new EdgeInf(extraVertex, v, true);
                extraEdge->setDist(edgeDist);
                extraEdges.push_back(extraEdge);
                continue;
            }
 
            if (newCost < v->sptfDist && v->sptfRoot == v)
            {
                // We have got to a node we haven't explored to from any tree.
                // So attach it to the tree and update it with the distance
                // from the root to reach this vertex.  Then add the vertex
                // to the heap of potentials to explore.
                if (debug_fp)
                {
                    fprintf(debug_fp, "<path d=\"M %g %g L %g %g\" "
                            "style=\"fill: none; stroke: %s; "
                            "stroke-width: 1px;\" />\n",
                            v->point.x, v->point.y, u->point.x,
                            u->point.y, "purple");
                }

                v->sptfDist = newCost;
                v->pathNext = u;
                v->sptfRoot = u->sptfRoot;
                vHeap.push_back(v);
                std::push_heap(vHeap.begin(), vHeap.end(), vHeapCompare);
            }
            else 
            {
                // We have reached a node that has been reached already through
                // a different tree.  Set the MTST distance for the bridging
                // edge and push it to the priority queue of edges to consider
                // during the extended Kruskal's algorithm.
                (*edge)->setMtstDist(u, bendCost);
                beHeap.push_back(*edge);
            }
        }
    }
    // Make the bridging edge heap.
    std::make_heap(beHeap.begin(), beHeap.end(), beHeapCompare);
    if (debug_fp)
    {
        fprintf(debug_fp, "</g>\n");
    }
    router->timers.Stop();

    // Perform extended Kruskal's algorithm
    // ====================================
    //
    router->timers.Register(tmHyperedgeMTST, timerStart);
    if (debug_fp)
    {
        fprintf(debug_fp, "<g inkscape:groupmode=\"layer\" "
                "style=\"display: none;\" "
                "inkscape:label=\"MTST-%02u-T\">\n", debug_count);
    }
    while ( ! beHeap.empty() )
    {
        // Take the lowest cost edge.
        EdgeInf *e = beHeap.front();

        // Pop the lowest cost edge off of the heap.
        std::pop_heap(beHeap.begin(), beHeap.end(), beHeapCompare);
        beHeap.pop_back();

        // Find the sets of terminals that each of the trees connects.
        VertexSetList::iterator s1 = findSet(e->m_vert1->sptfRoot);
        VertexSetList::iterator s2 = findSet(e->m_vert2->sptfRoot);

        if ((s1 == allsets.end()) || (s2 == allsets.end()))
        {
            // This is a special case if we would be connecting to something
            // that isn't a standard terminal shortest path tree, and thus
            // doesn't have a root.
            continue;
        }

        // We ignore edges that don't connect us to anything new, i.e., when
        // the terminal sets are the same.
        if (s1 != s2)
        {
            // This is bridging us to one or more new terminals.

            // Union the terminal sets.
            unionSets(s1, s2);

            // Connect this edge into the MTST by building HyperEdgeTree nodes
            // and edges for this edge and the path back to the tree root.
            HyperEdgeTreeNode *node1 = NULL;
            HyperEdgeTreeNode *node2 = NULL;
            if (hyperEdgeTreeJunctions)
            {
                node1 = new HyperEdgeTreeNode();
                node1->point = e->m_vert1->point;
                nodes[e->m_vert1] = node1;

                node2 = new HyperEdgeTreeNode();
                node2->point = e->m_vert2->point;
                nodes[e->m_vert2] = node2;

                new HyperEdgeTreeEdge(node1, node2, NULL);
            }
            if (debug_fp)
            {
                fprintf(debug_fp, "<path d=\"M %g %g L %g %g\" "
                        "style=\"fill: none; stroke: %s; "
                        "stroke-width: 1px;\" />\n", e->m_vert1->point.x,
                        e->m_vert1->point.y, e->m_vert2->point.x,
                        e->m_vert2->point.y, "red");
            }
            buildHyperEdgeTreeToRoot(e->m_vert1->pathNext, node1);
            buildHyperEdgeTreeToRoot(e->m_vert2->pathNext, node2);
        }
    }
    if (debug_fp)
    {
        fprintf(debug_fp, "</g>\n");
    }

    // Free the dummy nodes and edges created earlier.
    for_each(extraEdges.begin(), extraEdges.end(), delete_object());
    for_each(extraVertices.begin(), extraVertices.end(), delete_object());

    router->timers.Stop();
}

}

