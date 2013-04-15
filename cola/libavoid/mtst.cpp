/*
 * vim: ts=4 sw=4 et tw=0 wm=0
 *
 * libavoid - Fast, Incremental, Object-avoiding Line Router
 *
 * Copyright (C) 2011  Monash University
 *
 * --------------------------------------------------------------------
 * Sequential Construction of the Minimum Terminal Spanning Tree is an
 * extended version of the method described in Section IV.B of:
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
 * Author(s):  Michael Wybrow
*/

#include <cfloat>
#include <vector>
#include <algorithm>
#include <cstring>


#include "libavoid/hyperedgetree.h"
#include "libavoid/router.h"
#include "libavoid/mtst.h"
#include "libavoid/vertices.h"
#include "libavoid/timer.h"
#include "libavoid/junction.h"

namespace Avoid {


// Comparison for the vertex heap in the extended Dijkstra's algorithm.
bool HeapCmpVertInf::operator()(const VertInf *a, const VertInf *b) const
{
    return a->sptfDist > b->sptfDist;
}


// Comparison for the bridging edge heap in the extended Kruskal's algorithm.
bool CmpEdgeInf::operator()(const EdgeInf *a, const EdgeInf *b) const
{
    return a->mtstDist() > b->mtstDist();
}


struct delete_vertex
{
    void operator()(VertInf *ptr)
    { 
        ptr->removeFromGraph(false);
        delete ptr;
    }
};


MinimumTerminalSpanningTree::MinimumTerminalSpanningTree(Router *router,
        std::set<VertInf *> terminals, JunctionHyperEdgeTreeNodeMap *hyperEdgeTreeJunctions)
    : router(router),
      isOrthogonal(true),
      terminals(terminals),
      hyperEdgeTreeJunctions(hyperEdgeTreeJunctions),
      m_rootJunction(NULL),
      bendPenalty(2000),
      debug_fp(NULL),
      debug_count(0),
      dimensionChangeVertexID(0, 42)
{

}

MinimumTerminalSpanningTree::~MinimumTerminalSpanningTree()
{
    // Free the temporary hyperedge tree representation.
    m_rootJunction->deleteEdgesExcept(NULL);
    delete m_rootJunction;
    m_rootJunction = NULL;
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
        HyperEdgeTreeNode *prevNode, VertInf *prevVert, bool markEdges)
{
    COLA_ASSERT(currVert != NULL);

    // This method follows branches in a shortest path tree back to the
    // root, generating hyperedge tree nodes and branches as it goes.
    while (currVert)
    {
        // Add the node, if necessary.
        HyperEdgeTreeNode *addedNode = addNode(currVert, prevNode);

        if (markEdges)
        {
            //COLA_ASSERT( !(currVert->id == dimensionChangeVertexID) );
            //COLA_ASSERT( !(prevVert->id == dimensionChangeVertexID) );
            EdgeInf *edge = prevVert->hasNeighbour(currVert, isOrthogonal);
            if (edge == NULL && (currVert->id == dimensionChangeVertexID))
            {
                VertInf *modCurr = (currVert->id == dimensionChangeVertexID) ? 
                        currVert->m_orthogonalPartner : currVert;
                VertInf *modPrev = (prevVert->id == dimensionChangeVertexID) ? 
                        prevVert->m_orthogonalPartner : prevVert;
                edge = modPrev->hasNeighbour(modCurr, isOrthogonal);
            }
            COLA_ASSERT(edge);
            edge->setHyperedgeSegment(true);
        }

        if (debug_fp)
        {
            fprintf(debug_fp, "<path id=\"%s-%u\" d=\"M %g %g L %g %g\" "
                    "class=\"hyperedge\" style=\"stroke: %s;\" />\n", 
                    debug_label, step_count,
                    currVert->point.x, currVert->point.y,
                    prevNode->point.x, prevNode->point.y, "blue");
            ++step_count;
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
        prevVert = currVert;
        currVert = currVert->pathNext;
    }
}


VertInf **MinimumTerminalSpanningTree::resetDistsForPath(VertInf *currVert, VertInf **newRootVertPtr)
{
    COLA_ASSERT(currVert != NULL);

    // This method follows branches in a shortest path tree back to the
    // root, generating hyperedge tree nodes and branches as it goes.
    while (currVert)
    {
        if (currVert->sptfDist == 0)
        {
            VertInf **oldTreeRootPtr = currVert->treeRootPointer();
            // We've reached a junction, so stop.
            rewriteRestOfHyperedge(currVert, newRootVertPtr);
            return oldTreeRootPtr;
        }

        currVert->sptfDist = 0;
        currVert->setTreeRootPointer(newRootVertPtr);

        terminals.insert(currVert);

        currVert = currVert->pathNext;
    }

    // Shouldn't get here.
    COLA_ASSERT(false);
    return NULL;
}


void MinimumTerminalSpanningTree::constructSequential(void)
{
    // First, perform extended Dijkstra's algorithm
    // ============================================
    //
    router->timers.Register(tmHyperedgeForest, timerStart);

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
        k->sptfDist = DBL_MAX;
        k->pathNext = NULL;
        k->setSPTFRoot(k);
    }
    for (std::set<VertInf *>::iterator ti = terminals.begin();
            ti != terminals.end(); ++ti)
    {
        VertInf *t = *ti;
        // This is a terminal, set a distance of zero.
        t->sptfDist = 0;
        makeSet(t);
        vHeap.push_back(t);

    }
    std::make_heap(vHeap.begin(), vHeap.end(), vHeapCompare);

    step_count = 1;
    // Shortest path terminal forest construction
    //
    if (debug_fp)
    {
        fprintf(debug_fp, "<g inkscape:groupmode=\"layer\" "
                "style=\"display: none;\" "
                "inkscape:label=\"sequentialDijkstra\">\n");
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
            if (u->sptfRoot() == v->sptfRoot())
            {
                continue;
            }

            // This is an extension to the original method that takes a bend
            // cost into account.  When edges from this node, we take into
            // account the direction of the branch in the tree that got us
            // here.  For an edge colinear to this we do the normal thing,
            // and add it to the heap.  For edges at right angle, we don't
            // immediately add these, but instead add a dummy segment and node
            // at the current position and give the edge an distance equal to
            // the bend penalty.  We add equivalent edges for the right-angled
            // original edges, so these may be explored when the algorithm
            // explores the dummy node.  Obviously we also need to clean up
            // these dummy nodes and edges later.
            double newCost = (u->sptfDist + edgeDist);

            double freeConnection = connectsWithoutBend(u, v);
            COLA_ASSERT(!freeConnection == (u->pathNext && ! colinear(u->pathNext->point, 
                    u->point, v->point)));
            if (!freeConnection) 
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
                    extraVertex->sptfDist = bendPenalty + u->sptfDist;
                    extraVertex->pathNext = u;
                    extraVertex->setSPTFRoot(u->sptfRoot());
                    vHeap.push_back(extraVertex);
                    std::push_heap(vHeap.begin(), vHeap.end(), vHeapCompare);
                }
                // Add a copy of the ignored edge to the dummy node, so it
                // may be explored later.
                EdgeInf *extraEdge = new EdgeInf(extraVertex, v, isOrthogonal);
                extraEdge->setDist(edgeDist);                
                continue;
            }
 
            if (newCost < v->sptfDist && v->sptfRoot() == v)
            {
                // We have got to a node we haven't explored to from any tree.
                // So attach it to the tree and update it with the distance
                // from the root to reach this vertex.  Then add the vertex
                // to the heap of potentials to explore.
                if (debug_fp)
                {
                    fprintf(debug_fp, "<path id=\"for-%u\" class=\"forest\" "
                            "d=\"M %g %g L %g %g\" />\n", step_count,
                            v->point.x, v->point.y, u->point.x,
                            u->point.y);
                    ++step_count;
                }

                v->sptfDist = newCost;
                v->pathNext = u;
                v->setSPTFRoot(u->sptfRoot());
                vHeap.push_back(v);
                std::push_heap(vHeap.begin(), vHeap.end(), vHeapCompare);
            }
            else 
            {
                // We have reached a node that has been reached already through
                // a different tree.  Set the MTST distance for the bridging
                // edge and push it to the priority queue of edges to consider
                // during the extended Kruskal's algorithm.
                double secondJoinCost = connectsWithoutBend(v, u) ? 
                        0.0 : bendPenalty;

                // The default cost is the cost back to the root of each 
                // forest plus the length of this edge.
                double cost = (*edge)->m_vert1->sptfDist + 
                        (*edge)->m_vert2->sptfDist + secondJoinCost + 
                        (*edge)->getDist();
                (*edge)->setMtstDist(cost);
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

    // Next, perform extended Kruskal's algorithm
    // ==========================================
    //
    router->timers.Register(tmHyperedgeMTST, timerStart);
    if (debug_fp)
    {
        step_count = 1;
        fprintf(debug_fp, "<g inkscape:groupmode=\"layer\" "
                "id=\"kru-0\" style=\"display: none;\" "
                "inkscape:label=\"sequentialKuskal\">\n");
        strcpy(debug_label, "kru");
    }
    while ( ! beHeap.empty() )
    {
        // Take the lowest cost edge.
        EdgeInf *e = beHeap.front();

        // Pop the lowest cost edge off of the heap.
        std::pop_heap(beHeap.begin(), beHeap.end(), beHeapCompare);
        beHeap.pop_back();

        // Find the sets of terminals that each of the trees connects.
        VertexSetList::iterator s1 = findSet(e->m_vert1->sptfRoot());
        VertexSetList::iterator s2 = findSet(e->m_vert2->sptfRoot());

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
                fprintf(debug_fp, "<path id=\"kru-%u\" d=\"M %g %g L %g %g\" "
                        "class=\"hyperedge\" style=\"stroke: %s;\" />\n", 
                        step_count, e->m_vert1->point.x, e->m_vert1->point.y, 
                        e->m_vert2->point.x, e->m_vert2->point.y, "red");
                ++step_count;
            }
            buildHyperEdgeTreeToRoot(e->m_vert1->pathNext, node1, e->m_vert1);
            buildHyperEdgeTreeToRoot(e->m_vert2->pathNext, node2, e->m_vert2);
        }
    }
    if (debug_fp)
    {
        fprintf(debug_fp, "</g>\n");
    }

    // Free the dummy nodes and edges created earlier.
    for_each(extraVertices.begin(), extraVertices.end(), delete_vertex());
    extraVertices.clear();
    nodes.clear();
    allsets.clear();

    router->timers.Stop();
}

VertInf *MinimumTerminalSpanningTree::orthogonalPartner(VertInf *vert, 
        double penalty)
{
    if (penalty == 0)
    {
        penalty = bendPenalty;
    }
    if (vert->m_orthogonalPartner == NULL)
    {
        vert->m_orthogonalPartner = new VertInf(router, 
                dimensionChangeVertexID, vert->point, false);
        vert->m_orthogonalPartner->m_orthogonalPartner = vert;
        extraVertices.push_back(vert->m_orthogonalPartner);
        EdgeInf *extraEdge = new EdgeInf(vert->m_orthogonalPartner, vert,
                isOrthogonal);
        extraEdge->setDist(penalty);
    }
    return vert->m_orthogonalPartner;
}

void MinimumTerminalSpanningTree::popInvalidBridgingEdges()
{
    while (!beHeap.empty())
    {
        // Take the lowest cost bridging edge.
        EdgeInf *e = beHeap.front();

        VertexPair ends = realVerticesCountingPartners(e);
        if ((ends.first->treeRoot() != ends.second->treeRoot()) &&
                ends.first->treeRoot() && ends.second->treeRoot())
        {
            // We have a real bridging edge on top, so carry on with the
            // search.
            break;
        }

        // The top edge doesn't bridge separate groups, so pop it and look at
        // the next one on the heap.
        std::pop_heap(beHeap.begin(), beHeap.end(), beHeapCompare);
        beHeap.pop_back();
    }
}

LayeredOrthogonalEdgeList MinimumTerminalSpanningTree::
        getOrthogonalEdgesFromVertex(VertInf *vert, VertInf *prev)
{
    LayeredOrthogonalEdgeList edgeList;

    COLA_ASSERT(vert);
    
    double penalty = (prev == NULL) ? 0.1 : 0;
    orthogonalPartner(vert, penalty);

    bool isRealVert = (vert->id != dimensionChangeVertexID);
    VertInf *realVert = (isRealVert) ? vert : orthogonalPartner(vert);
    COLA_ASSERT(realVert->id != dimensionChangeVertexID);
    EdgeInfList& visList = (!isOrthogonal) ? realVert->visList : realVert->orthogVisList;
    EdgeInfList::const_iterator finish = visList.end();
    for (EdgeInfList::const_iterator edge = visList.begin(); edge != finish; ++edge)
    {
        VertInf *other = (*edge)->otherVert(realVert);
        
        if (other == orthogonalPartner(realVert))
        {
            VertInf *partner = (isRealVert) ? other : orthogonalPartner(other);
            if (partner != prev)
            {
                edgeList.push_back(std::make_pair(*edge, partner));
            }
            continue;
        }
        
        VertInf *partner = (isRealVert) ? other : orthogonalPartner(other);
        COLA_ASSERT(partner);
        
        if (other->point.y == realVert->point.y)
        {
            if (isRealVert && (prev != partner))
            {
                edgeList.push_back(std::make_pair(*edge, partner));
            }
        }
        else if (other->point.x == realVert->point.x)
        {
            if (!isRealVert && (prev != partner))
            {
                edgeList.push_back(std::make_pair(*edge, partner));
            }
        }
        else
        {
            printf("Warning, nonorthogonal edge.\n");
            edgeList.push_back(std::make_pair(*edge, other));
        }
    }
        
    return edgeList;
}

void MinimumTerminalSpanningTree::constructInterleaved(void)
{
    step_count = 1;
    // Perform an interleaved construction of the MTST and SPTF
    // ========================================================
    //
    router->timers.Register(tmHyperedgeAlt, timerStart);
    strcpy(debug_label, "int");

    origTerminals = terminals;

    // Initialisation
    //
    VertInf *endVert = router->vertices.end();
    for (VertInf *k = router->vertices.connsBegin(); k != endVert;
            k = k->lstNext)
    {
        k->sptfDist = DBL_MAX;
        k->pathNext = NULL;
        k->setTreeRootPointer(NULL);
        k->m_orthogonalPartner = NULL;
    }

    COLA_ASSERT(rootVertexPointers.empty());
    for (std::set<VertInf *>::iterator ti = terminals.begin();
            ti != terminals.end(); ++ti)
    {
        VertInf *t = *ti;
        // This is a terminal, set a distance of zero.
        t->sptfDist = 0;
        rootVertexPointers.push_back(t->makeTreeRootPointer(t));
        vHeap.push_back(t);
    }
    for (EdgeInf *t = router->visOrthogGraph.begin();
            t != router->visOrthogGraph.end(); t = t->lstNext)
    {
        t->setHyperedgeSegment(false);
    }

    std::make_heap(vHeap.begin(), vHeap.end(), vHeapCompare);
    
    // Shortest Path Terminal Forest construction
    //
    unsigned step = 1;
    if (debug_fp)
    {
        fprintf(debug_fp, "<g inkscape:groupmode=\"layer\" "
                "inkscape:label=\"interleaved\">\n");
    }
    if (debug_fp)
    {
        fprintf(debug_fp, "<g inkscape:groupmode=\"layer\" "
                "style=\"display: none;\" "
                "inkscape:label=\"grow-%02u\">\n", step++);
    }
    while ( ! vHeap.empty() )
    {
        // Take the lowest vertex from heap.
        VertInf *u = vHeap.front();

        // Pop the lowest vertex off the heap.
        std::pop_heap(vHeap.begin(), vHeap.end(), vHeapCompare);
        vHeap.pop_back();
     
        if ((u->treeRoot() == NULL))
        {
            // This is an orphaned vertex.
            continue;
        }
        COLA_ASSERT(u->pathNext || (u->sptfDist == 0));

        popInvalidBridgingEdges();

        if (!beHeap.empty() && u->sptfDist >= (0.5 * beHeap.front()->mtstDist()))
        {
            // Take the lowest cost edge.
            EdgeInf *e = beHeap.front();

            // Pop the lowest cost edge off of the heap.
            std::pop_heap(beHeap.begin(), beHeap.end(), beHeapCompare);
            beHeap.pop_back();

            VertexPair ends = realVerticesCountingPartners(e);
            if (origTerminals.find(ends.first->treeRoot()) == origTerminals.end() ||
                origTerminals.find(ends.second->treeRoot()) == origTerminals.end() )
            {
                continue;
            }

            unsigned int saved_step_count = step_count;
            commitToBridgingEdge(e, step);
            step_count = saved_step_count + 1;

            if (origTerminals.size() == 1)
            {
                break;
            }
            // Don't pop this vertex, but continue.
            continue;
        }

        // For each edge from this vertex...
        LayeredOrthogonalEdgeList edgeList = getOrthogonalEdgesFromVertex(u,
                u->pathNext);
        for (LayeredOrthogonalEdgeList::const_iterator edge = edgeList.begin(); 
                edge != edgeList.end(); ++edge)
        {
            VertInf *v = edge->second;
            EdgeInf *e = edge->first;
            double edgeDist = e->getDist();

            // Assign a distance (length) of 1 for dummy visibility edges
            // which may not accurately reflect the real distance of the edge.
            if (v->id.isDummyPinHelper() || u->id.isDummyPinHelper())
            {
                edgeDist = 1;
            }

            // Don't do anything more here if this is an intra-tree edge that
            // would just bridge branches of the same tree.
            if (u->treeRoot() == v->treeRoot())
            {
                continue;
            }

            // This is an extension to the original method that takes a bend
            // cost into account.  When edges from this node, we take into
            // account the direction of the branch in the tree that got us
            // here.  For an edge colinear to this we do the normal thing,
            // and add it to the heap.  For edges at right angle, we don't
            // immediately add these, but instead add a dummy segment and node
            // at the current position and give the edge an distance equal to
            // the bend penalty.  We add equivalent edges for the right-angled
            // original edges, so these may be explored when the algorithm
            // explores the dummy node.  Obviously we also need to clean up
            // these dummy nodes and edges later.
            if (v->treeRoot() == NULL)
            {
                double newCost = (u->sptfDist + edgeDist);
            
                // We have got to a node we haven't explored to from any tree.
                // So attach it to the tree and update it with the distance
                // from the root to reach this vertex.  Then add the vertex
                // to the heap of potentials to explore.
                v->sptfDist = newCost;
                v->pathNext = u;
                v->setTreeRootPointer(u->treeRootPointer());
                vHeap.push_back(v);
                std::push_heap(vHeap.begin(), vHeap.end(), vHeapCompare);

                if (debug_fp)
                {
                    fprintf(debug_fp, "<path id=\"frst-%u\" class=\"forest\" "
                            "d=\"M %g %g L %g %g\" />\n", step_count,
                            v->point.x, v->point.y, u->point.x,
                            u->point.y);
                    ++step_count;
                }
            }
            else 
            {
                // We have reached a node that has been reached already through
                // a different tree.  Set the MTST distance for the bridging
                // edge and push it to the priority queue of edges to consider
                // during the extended Kruskal's algorithm.
                double cost = v->sptfDist + u->sptfDist + e->getDist();
                e->setMtstDist(cost);
                beHeap.push_back(e);
                std::push_heap(beHeap.begin(), beHeap.end(), beHeapCompare);
            }
        }
    }
    COLA_ASSERT(origTerminals.size() == 1);
    if (debug_fp)
    {
        fprintf(debug_fp, "</g>\n</g>\n");
        //printf("-- %d %d %d\n", (int) origTerminals.size(), 
        //        (int) vHeap.size(), (int) beHeap.size());
    }
    router->timers.Stop();

    // Free Root Vertex Points from all vertices.
    for (std::list<VertInf **>::iterator curr = rootVertexPointers.begin();
            curr != rootVertexPointers.end(); ++curr)
    {
        delete *curr;
    }
    rootVertexPointers.clear();

    // Free the dummy nodes and edges created earlier.
    for_each(extraVertices.begin(), extraVertices.end(), delete_vertex());
    extraVertices.clear();
}

bool MinimumTerminalSpanningTree::connectsWithoutBend(VertInf *oldLeaf,
        VertInf *newLeaf)
{
    COLA_ASSERT(isOrthogonal);

    if (oldLeaf->sptfDist == 0)
    {
        bool hyperedgeConnection = false;
        EdgeInfList& visList = (!isOrthogonal) ? 
                oldLeaf->visList : oldLeaf->orthogVisList;
        EdgeInfList::const_iterator finish = visList.end();
        for (EdgeInfList::const_iterator edge = visList.begin();
                edge != finish; ++edge)
        {
            VertInf *other = (*edge)->otherVert(oldLeaf);

            if (other == newLeaf)
            {
                continue;
            }
         
            if (other->point == oldLeaf->point)
            {
                continue;
            }

            if ((*edge)->isHyperedgeSegment())
            {
                hyperedgeConnection = true;
                if (colinear(other->point, oldLeaf->point, newLeaf->point))
                {
                    return true;
                }
            }
        }
        // If there was no hyperedge to connect to, then its a tree source
        // and we can connect without a bend.
        return (!hyperedgeConnection) ? true : false;
    }
    else
    {
        if (oldLeaf->pathNext)
        {
            return colinear(oldLeaf->pathNext->point, oldLeaf->point,
                    newLeaf->point);
        }
        else
        {
            // No penalty.
            return true;
        }
    }
}

void MinimumTerminalSpanningTree::rewriteRestOfHyperedge(VertInf *vert,
        VertInf **newTreeRootPtr)
{
    vert->setTreeRootPointer(newTreeRootPtr);

    LayeredOrthogonalEdgeList edgeList = getOrthogonalEdgesFromVertex(vert,
                NULL);
    for (LayeredOrthogonalEdgeList::const_iterator edge = edgeList.begin(); 
            edge != edgeList.end(); ++edge)
    {
        VertInf *v = edge->second;

        if (v->treeRootPointer() == newTreeRootPtr)
        {
            // Already marked.
            continue;
        }

        if (v->sptfDist == 0)
        {
            // This is part of the rest of an existing hyperedge,
            // so mark it and continue.
            rewriteRestOfHyperedge(v, newTreeRootPtr);
        }
    }
}

void MinimumTerminalSpanningTree::drawForest(VertInf *vert, VertInf *prev)
{
    if (prev == NULL)
    {
        char colour[40];
        strcpy(colour, "green");
        /*
        if (vert->id == dimensionChangeVertexID)
        {
            strcpy(colour, "blue");
        }
        */

        if (vert->treeRoot() == NULL)
        {
            strcpy(colour, "red");
        }

        COLA_ASSERT(vert->treeRootPointer() != NULL);
        COLA_ASSERT(vert->treeRoot() != NULL);
        fprintf(debug_fp, "<circle cx=\"%g\" cy=\"%g\" r=\"3\" db:sptfDist=\"%g\" "
                "style=\"fill: %s; stroke: %s; fill-opacity: 0.5; "
                "stroke-width: 1px; stroke-opacity:0.5\" />\n",
                vert->point.x, vert->point.y, vert->sptfDist, colour, "black");
    }
    
    LayeredOrthogonalEdgeList edgeList = getOrthogonalEdgesFromVertex(vert,
            prev);
    for (LayeredOrthogonalEdgeList::const_iterator edge = edgeList.begin(); 
            edge != edgeList.end(); ++edge)
    {
        VertInf *v = edge->second;

        if (v->sptfDist == 0)
        {
            continue;
        }

        if (v->treeRoot() == vert->treeRoot())
        {
            if (v->pathNext == vert)
            {
                if (debug_fp && (vert->point != v->point))
                {
                    fprintf(debug_fp, "<path d=\"M %g %g L %g %g\" db:sptfDist=\"%g\" "
                            "style=\"fill: none; stroke: %s; "
                            "stroke-width: 1px;\" />\n",
                            vert->point.x, vert->point.y, v->point.x,
                            v->point.y, v->sptfDist, "purple");
                }
                drawForest(v, vert);
            }
        }
    }
}

VertexPair MinimumTerminalSpanningTree::
        realVerticesCountingPartners(EdgeInf *edge)
{
    VertInf *v1 = edge->m_vert1;
    VertInf *v2 = edge->m_vert2;

    VertexPair realVertices = std::make_pair(v1, v2);

    if ((v1->id != dimensionChangeVertexID) && 
            (v2->id != dimensionChangeVertexID) && 
            (v1->point != v2->point) && 
            (v1->point.x == v2->point.x))
    {
        if (v1->m_orthogonalPartner)
        {
            realVertices.first = v1->m_orthogonalPartner;
        }
        if (v2->m_orthogonalPartner)
        {
            realVertices.second = v2->m_orthogonalPartner;
        }
    }

    return realVertices;
}


void MinimumTerminalSpanningTree::commitToBridgingEdge(EdgeInf *e, unsigned& step)
{
    VertexPair ends = realVerticesCountingPartners(e);
    VertInf *newRoot = std::min(ends.first->treeRoot(), ends.second->treeRoot());
    VertInf *oldRoot = std::max(ends.first->treeRoot(), ends.second->treeRoot());

    // Connect this edge into the MTST by building HyperEdgeTree nodes
    // and edges for this edge and the path back to the tree root.
    HyperEdgeTreeNode *node1 = NULL;
    HyperEdgeTreeNode *node2 = NULL;

    VertInf *vert1 = ends.first;
    VertInf *vert2 = ends.second;
    if (hyperEdgeTreeJunctions)
    {
        node1 = new HyperEdgeTreeNode();
        node1->point = ends.first->point;
        nodes[vert1] = node1;

        node2 = new HyperEdgeTreeNode();
        node2->point = ends.second->point;
        nodes[vert2] = node2;

        new HyperEdgeTreeEdge(node1, node2, NULL);
        e->setHyperedgeSegment(true);
    }
    if (debug_fp)
    {
        fprintf(debug_fp, "</g>\n<g id=\"join-%u\" "
                "inkscape:groupmode=\"layer\" "
                "inkscape:label=\"join-%02u\">\n", step - 1, step - 1);
        fprintf(debug_fp, "<path d=\"M %g %g L %g %g\" "
                "style=\"fill: none; stroke: %s; "
                "stroke-width: 1px;\" />\n", vert1->point.x,
                vert1->point.y, vert2->point.x,
                vert2->point.y, "red");
    }
    buildHyperEdgeTreeToRoot(vert1->pathNext, node1, vert1, true);
    buildHyperEdgeTreeToRoot(vert2->pathNext, node2, vert2, true);
    
    if (debug_fp)
    {
        fprintf(debug_fp, "</g>\n<g inkscape:groupmode=\"layer\" "
                "style=\"display: none;\" "
                "inkscape:label=\"forest-%02u\">\n", step -1);
        for (std::set<VertInf *>::iterator ti = terminals.begin();
                ti != terminals.end(); ++ti)
        {
            drawForest(*ti, NULL);
        }
    }
    
    VertInf **oldTreeRootPtr1 = vert1->treeRootPointer();
    VertInf **oldTreeRootPtr2 = vert2->treeRootPointer();
    origTerminals.erase(oldRoot);
    VertInf **newTreeRootPtr = vert1->makeTreeRootPointer(newRoot);
    rootVertexPointers.push_back(newTreeRootPtr);
    vert2->setTreeRootPointer(newTreeRootPtr);

    // Zero paths and add vertices on path to the terminal set.
    COLA_ASSERT(newRoot);
    resetDistsForPath(vert1, newTreeRootPtr);
    resetDistsForPath(vert2, newTreeRootPtr);
   
    COLA_ASSERT(oldTreeRootPtr1);
    COLA_ASSERT(oldTreeRootPtr2);
    *oldTreeRootPtr1 = NULL;
    *oldTreeRootPtr2 = NULL;

    if (debug_fp)
    {
        fprintf(debug_fp, "</g>\n<g id=\"pruned-%u\" "
                "inkscape:groupmode=\"layer\" style=\"display: none;\" "
                "inkscape:label=\"pruned-%02u\">\n", step - 1, step - 1);
        for (std::set<VertInf *>::iterator ti = terminals.begin();
                ti != terminals.end(); ++ti)
        {
            drawForest(*ti, NULL);
        }
    }

    // Finish when we have joined everything.
    if (origTerminals.size() == 1)
    {
        return;
    }

    if (debug_fp)
    {
        fprintf(debug_fp, "</g>\n");
        fprintf(debug_fp, "<g inkscape:groupmode=\"layer\" "
                "style=\"display: none;\" "
                "inkscape:label=\"grow-%02u\">\n", step++);
    }

    // Reset all terminals to zero.
    for (std::set<VertInf *>::iterator v2 = terminals.begin(); 
            v2 != terminals.end(); ++v2)
    {
        COLA_ASSERT((*v2)->sptfDist == 0);
        vHeap.push_back(*v2);
        std::push_heap(vHeap.begin(), vHeap.end(), vHeapCompare);
    }
}

}

