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
 * Author(s):  Michael Wybrow
*/

#include <algorithm>

#include "libavoid/hyperedgetree.h"
#include "libavoid/geometry.h"
#include "libavoid/connector.h"
#include "libavoid/router.h"
#include "libavoid/connend.h"
#include "libavoid/assertions.h"

namespace Avoid {


// Constructs a new hyperedge tree node.
//
HyperEdgeTreeNode::HyperEdgeTreeNode()
    : junction(NULL),
      shiftSegmentNodeSet(NULL),
      finalVertex(NULL)
{
}

HyperEdgeTreeNode::~HyperEdgeTreeNode()
{
    if (shiftSegmentNodeSet)
    {
        shiftSegmentNodeSet->erase(this);
        shiftSegmentNodeSet = NULL;
    }
}


// This method traverses the hyperedge tree and outputs each of the edges
// and junction positions as SVG objects to the file fp.
//
void HyperEdgeTreeNode::outputEdgesExcept(FILE *fp, HyperEdgeTreeEdge *ignored)
{
    if (junction)
    {
        fprintf(fp, "<circle cx=\"%g\" cy=\"%g\" r=\"6\" "
            "style=\"fill: green; stroke: none;\" />\n", point.x, point.y);
    }
    for (std::list<HyperEdgeTreeEdge *>::iterator curr = edges.begin();
            curr != edges.end(); ++curr)
    {
        if (*curr != ignored)
        {
            (*curr)->outputNodesExcept(fp, this);
        }
    }
}


// This method traverses the hyperedge tree and removes from treeRoots any
// junction nodes (other than this one).

void HyperEdgeTreeNode::removeOtherJunctionsFrom(HyperEdgeTreeEdge *ignored, 
            JunctionSet& treeRoots)
{
    for (std::list<HyperEdgeTreeEdge *>::iterator curr = edges.begin();
            curr != edges.end(); ++curr)
    {
        if (*curr != ignored)
        {
            (*curr)->removeOtherJunctionsFrom(this, treeRoots);
        }
    }
}


// This method traverses the hyperedge tree and writes each of the paths
// back to the individual connectors as routes.
//
void HyperEdgeTreeNode::writeEdgesToConns(HyperEdgeTreeEdge *ignored,
        size_t pass)
{
    for (std::list<HyperEdgeTreeEdge *>::iterator curr = edges.begin();
            curr != edges.end(); ++curr)
    {
        if (*curr != ignored)
        {
            (*curr)->writeEdgesToConns(this, pass);
        }
    }
}

// This method traverses the hyperedge tree and creates connectors for each
// segment bridging junction and/or terminals.  It also sets the 
// appropriate ConnEnds for each connector.
//
void HyperEdgeTreeNode::addConns(HyperEdgeTreeEdge *ignored, Router *router,
        ConnRefList& oldConns, ConnRef *conn)
{
    // If no connector is set, then we must be starting off at a junction.
    COLA_ASSERT(conn || junction);

    for (std::list<HyperEdgeTreeEdge *>::iterator curr = edges.begin();
            curr != edges.end(); ++curr)
    {
        if (*curr != ignored)
        {
            // If we're not at a junction, then use the connector value being
            // passed in to the method.

            if (junction)
            {
                // If we're at a junction, then we are effectively starting
                // our traversal along a connector, so create this new connector
                // and set it's start ConnEnd to be this junction.
                conn = new ConnRef(router);
                router->removeObjectFromQueuedActions(conn);
                conn->makeActive();
                conn->m_initialised = true;
                ConnEnd connend(junction);
                conn->updateEndPoint(VertID::src, connend);
            }
    
            // Set the connector for this edge.
            (*curr)->conn = conn;
            
            // Continue recursive traversal.
            (*curr)->addConns(this, router, oldConns);
        }
    }
}


// This method traverses the hyperedge tree and returns a list of the junctions
// and connectors that make up the hyperedge.
//
void HyperEdgeTreeNode::listJunctionsAndConnectors(HyperEdgeTreeEdge *ignored,
        JunctionRefList& junctions, ConnRefList& connectors)
{
    if (junction)
    {
        junctions.push_back(junction);
    }

    for (std::list<HyperEdgeTreeEdge *>::iterator curr = edges.begin();
            curr != edges.end(); ++curr)
    {
        if (*curr != ignored)
        {
            (*curr)->listJunctionsAndConnectors(this, junctions, connectors);
        }
    }
}


// This method traverses the hyperedge tree removing zero length edges.
//
void HyperEdgeTreeNode::removeZeroLengthEdges(HyperEdgeTreeEdge *ignored)
{
    for (std::list<HyperEdgeTreeEdge *>::iterator curr = edges.begin();
            curr != edges.end(); ++curr)
    {
        HyperEdgeTreeEdge *edge = *curr;
        if (edge != ignored)
        {
            if (edge->zeroLength())
            {
                HyperEdgeTreeNode *other = edge->followFrom(this);
                HyperEdgeTreeNode *target = NULL;
                HyperEdgeTreeNode *source = NULL;
                if (other->junction && ! junction)
                {
                    target = other;
                    source = this;
                }
                else if ( ! other->junction && junction)
                {
                    target = this;
                    source = other;
                }
                else if ( ! other->junction && ! junction)
                {
                    target = this;
                    source = other;
                }

                if (target)
                {
                    edge->disconnectEdge();
                    delete edge;
                    target->spliceEdgesFrom(source);
                    delete source;
                    target->removeZeroLengthEdges(ignored);
                    return;
                }
                // XXX Deal with merging two junctions?
            }

            // Recursive call.
            edge->removeZeroLengthEdges(this);
        }
    }
}


// This method traverses the hyperedge tree, clearing up the objects and
// memory used to store the tree.
//
void HyperEdgeTreeNode::deleteEdgesExcept(HyperEdgeTreeEdge *ignored)
{
    for (std::list<HyperEdgeTreeEdge *>::iterator curr = edges.begin();
            curr != edges.end(); ++curr)
    {
        if (*curr != ignored)
        {
            (*curr)->deleteNodesExcept(this);
            delete *curr;
        }
    }
    edges.clear();
}


// This method disconnects a specific hyperedge tree edge from the given node.
//
void HyperEdgeTreeNode::disconnectEdge(HyperEdgeTreeEdge *edge)
{
    for (std::list<HyperEdgeTreeEdge *>::iterator curr = edges.begin();
            curr != edges.end(); )
    {
        if (*curr == edge)
        {
            curr = edges.erase(curr);
        }
        else
        {
            ++curr;
        }
    }

}


// This method moves all edges attached to oldNode to instead be attached to
// the given hyperedge tree node.
//
void HyperEdgeTreeNode::spliceEdgesFrom(HyperEdgeTreeNode *oldNode)
{
    for (std::list<HyperEdgeTreeEdge *>::iterator curr = oldNode->edges.begin();
            curr != oldNode->edges.end(); curr = oldNode->edges.begin())
    {
        (*curr)->replaceNode(oldNode, this);
    }
}

// This method moves the junction at the given node along any shared paths
// (so long as this action would not create any additional shared paths),
// while also removing and freeing merged edges and nodes in the process.
// It returns the new node where the junction is now located.
//
HyperEdgeTreeNode *HyperEdgeTreeNode::moveJunctionAlongCommonEdge(
        HyperEdgeTreeNode *self)
{
    COLA_ASSERT(self->junction);

    HyperEdgeTreeNode *newSelf = NULL;
    std::vector<HyperEdgeTreeEdge *> commonEdges;
    std::vector<HyperEdgeTreeEdge *> otherEdges;

    // Consider each edge from this node in turn.
    for (std::list<HyperEdgeTreeEdge *>::iterator curr = self->edges.begin();
            curr != self->edges.end(); ++curr)
    {
        HyperEdgeTreeEdge *currEdge = *curr;
        HyperEdgeTreeNode *currNode = currEdge->followFrom(self);
        commonEdges.clear();
        otherEdges.clear();

        if (currNode->junction)
        {
            // Don't shift junctions onto other junctions.
            continue;
        }

        // The current edge is a common edge we are looking to shift along.
        commonEdges.push_back(currEdge);

        // Consider each of the other edges.
        for (std::list<HyperEdgeTreeEdge *>::iterator curr2 = 
                self->edges.begin(); curr2 != self->edges.end(); ++curr2)
        {
            if (curr == curr2)
            {
                // Except the current (curr) one.
                continue;
            }
            
            HyperEdgeTreeEdge *otherEdge = *curr2;
            HyperEdgeTreeNode *otherNode = otherEdge->followFrom(self);
            if (otherNode->point == currNode->point)
            {
                // A common edge can be at the same point, but can't have
                // a junction at it.
                if (otherNode->junction)
                {
                    otherEdges.push_back(otherEdge);
                }
                else
                {
                    commonEdges.push_back(otherEdge);
                }
            }
            else if (pointOnLine(self->point, otherNode->point, 
                    currNode->point))
            {
                // A common edge can be a (longer) collinear line, but we
                // need to split the longer line at the other end of curr.
                otherEdge->splitFromNodeAtPoint(self, currNode->point);
                commonEdges.push_back(otherEdge);
            }
            else
            {
                // If the edge goes in another direction it is not common.
                otherEdges.push_back(otherEdge);
            }
        }

        if ((commonEdges.size() > 1) && (otherEdges.size() <= 1))
        {
            // One of the common nodes becomes the target node, we move 
            // all connections from the other common nodes to this node.
            // We also move the junction there and remove it from the 
            // current node.
            HyperEdgeTreeNode *targetNode = commonEdges[0]->followFrom(self);
            for (size_t i = 1; i < commonEdges.size(); ++i)
            {
                HyperEdgeTreeNode *thisNode = commonEdges[i]->followFrom(self);
                commonEdges[i]->disconnectEdge();
                targetNode->spliceEdgesFrom(thisNode);
                delete thisNode;
                delete commonEdges[i];
            }
            targetNode->junction = self->junction;
            self->junction = NULL;

            if (otherEdges.empty())
            {
                // Nothing else connected to this node, so remove the node
                // and the edge to the target node.
                commonEdges[0]->disconnectEdge();

                delete commonEdges[0];
                delete self;
            }
            else
            {
                // We need to mark commonEdges[0] as being from the connector
                // of the otherEdges[0].
                commonEdges[0]->conn = otherEdges[0]->conn;
            }
            newSelf = targetNode;

            break;
        }
    }

    return newSelf;
}


// Constructs a new hyperedge tree edge, given two endpoint nodes.
//
HyperEdgeTreeEdge::HyperEdgeTreeEdge(HyperEdgeTreeNode *node1,
        HyperEdgeTreeNode *node2, ConnRef *conn)
    : conn(conn)
{
    ends = std::make_pair(node1, node2);
    node1->edges.push_back(this);
    node2->edges.push_back(this);
}


// Given one endpoint of the hyperedge tree edge, returns the other endpoint.
//
HyperEdgeTreeNode *HyperEdgeTreeEdge::followFrom(HyperEdgeTreeNode *from) const
{
    return (ends.first == from) ? ends.second : ends.first;
}


// Returns true if the length of this edge is zero, i.e., the endpoints are
// located at the same position.
//
bool HyperEdgeTreeEdge::zeroLength(void) const
{
    return (ends.first->point == ends.second->point);
}


// This method traverses the hyperedge tree removing zero length edges.
//
void HyperEdgeTreeEdge::removeZeroLengthEdges(HyperEdgeTreeNode *ignored)
{
    if (ends.first != ignored)
    {
        ends.first->removeZeroLengthEdges(this);
    }

    if (ends.second != ignored)
    {
        ends.second->removeZeroLengthEdges(this);
    }
}


// This method traverses the hyperedge tree and outputs each of the edges
// and junction positions as SVG objects to the file fp.
//
void HyperEdgeTreeEdge::outputNodesExcept(FILE *fp, HyperEdgeTreeNode *ignored)
{
    fprintf(fp, "<path d=\"M %g %g L %g %g\" "
            "style=\"fill: none; stroke: %s; stroke-width: 2px; "
            "stroke-opacity: 0.5;\" />\n",
            ends.first->point.x, ends.first->point.y, 
            ends.second->point.x, ends.second->point.y, "purple");
    if (ends.first != ignored)
    {
        ends.first->outputEdgesExcept(fp, this);
    }

    if (ends.second != ignored)
    {
        ends.second->outputEdgesExcept(fp, this);
    }
}


// This method returns true if the edge is in the dimension given, i.e.,
// either horizontal or vertical.
//
bool HyperEdgeTreeEdge::hasOrientation(const size_t dimension) const
{
    return (ends.first->point[dimension] == ends.second->point[dimension]);
}


// This method updates any of the given hyperedge tree edge's endpoints that
// are attached to oldNode to instead be attached to newNode.
//
void HyperEdgeTreeEdge::replaceNode(HyperEdgeTreeNode *oldNode,
        HyperEdgeTreeNode *newNode)
{
    if (ends.first == oldNode)
    {
        oldNode->disconnectEdge(this);
        newNode->edges.push_back(this);
        ends.first = newNode;
    }
    else if (ends.second == oldNode)
    {
        oldNode->disconnectEdge(this);
        newNode->edges.push_back(this);
        ends.second = newNode;
    }
}


// This method traverses the hyperedge tree and writes each of the paths
// back to the individual connectors as routes.
//
void HyperEdgeTreeEdge::writeEdgesToConns(HyperEdgeTreeNode *ignored,
        size_t pass)
{
    if (pass == 0)
    {
        conn->m_display_route.clear();
    }
    else if (pass == 1)
    {
        if (ignored == ends.first)
        {
            if (conn->m_display_route.empty())
            {
                //printf("[%u] - %g %g\n", conn->id(), ends.first->point.x, ends.first->point.y);
                conn->m_display_route.ps.push_back(ends.first->point);
            }
            //printf("[%u] + %g %g\n", conn->id(), ends.second->point.x, ends.second->point.y);
            conn->m_display_route.ps.push_back(ends.second->point);
        }
        else
        {
            if (conn->m_display_route.empty())
            {
                //printf("[%u] - %g %g\n", conn->id(), ends.second->point.x, ends.second->point.y);
                conn->m_display_route.ps.push_back(ends.second->point);
            }
            //printf("[%u] + %g %g\n", conn->id(), ends.first->point.x, ends.first->point.y);
            conn->m_display_route.ps.push_back(ends.first->point);
        }
    }

    if (ends.first && (ends.first != ignored))
    {
        ends.first->writeEdgesToConns(this, pass);
    }

    if (ends.second && (ends.second != ignored))
    {
        ends.second->writeEdgesToConns(this, pass);
    }
}

// This method traverses the hyperedge tree and creates connectors for each
// segment bridging junction and/or terminals.  It also sets the 
// appropriate ConnEnds for each connector.
//
void HyperEdgeTreeEdge::addConns(HyperEdgeTreeNode *ignored, Router *router,
        ConnRefList& oldConns)
{
    COLA_ASSERT(conn != NULL);
    HyperEdgeTreeNode *endNode = NULL;
    if (ends.first && (ends.first != ignored))
    {
        endNode = ends.first;
        ends.first->addConns(this, router, oldConns, conn);
    }

    if (ends.second && (ends.second != ignored))
    {
        endNode = ends.second;
        ends.second->addConns(this, router, oldConns, conn);
    }

    if (endNode->finalVertex)
    {
        // We have reached a terminal of the hyperedge, so set a ConnEnd for
        // the original connector endpoint
        ConnEnd connend;
        bool result = false;
        // Find the ConnEnd from the list of original connectors.
        for (ConnRefList::iterator curr = oldConns.begin(); 
                curr != oldConns.end(); ++curr)
        {
            result |= (*curr)->getConnEndForEndpointVertex(
                    endNode->finalVertex, connend);
            if (result)
            {
                break;
            }
        }
        if (result)
        {
            // XXX: Create new conn here.
            conn->updateEndPoint(VertID::tar, connend);
        }
    }
    else if (endNode->junction)
    {
        // Or, set a ConnEnd connecting to the junction we have reached.
        ConnEnd connend(endNode->junction);
        conn->updateEndPoint(VertID::tar, connend);
    }
}

// This method traverses the hyperedge tree and returns a list of the junctions
// and connectors that make up the hyperedge.
//
void HyperEdgeTreeEdge::listJunctionsAndConnectors(HyperEdgeTreeNode *ignored,
        JunctionRefList& junctions, ConnRefList& connectors)
{
    ConnRefList::iterator foundPosition =
            std::find(connectors.begin(), connectors.end(), conn);
    if (foundPosition == connectors.end())
    {
        // Add connector if it isn't already in the list.
        connectors.push_back(conn);
    }

    if (ends.first != ignored)
    {
        ends.first->listJunctionsAndConnectors(this, junctions, connectors);
    }
    else if (ends.second != ignored)
    {
        ends.second->listJunctionsAndConnectors(this, junctions, connectors);
    }
}

// This method splits the current edge, adding a node at the given point.
// The current edge will connect the source node and the newly created node.
// A new edge will connect the new node and the node at the other end of the
// original edge.
//
void HyperEdgeTreeEdge::splitFromNodeAtPoint(HyperEdgeTreeNode *source, 
        const Point& point)
{
    // Make the source the first of the two nodes.
    if (ends.second == source)
    {
        std::swap(ends.second, ends.first);
    }
    COLA_ASSERT(ends.first == source);

    // Remember the other end.
    HyperEdgeTreeNode *target = ends.second;

    // Create a new node for the split point at the given position.
    HyperEdgeTreeNode *split = new HyperEdgeTreeNode();
    split->point = point;

    // Create a new edge between the split point and the other end.
    new HyperEdgeTreeEdge(split, target, conn);
    
    // Disconnect the current edge from the other end and connect it to 
    // the new split point node.
    target->disconnectEdge(this);
    ends.second = split;
    split->edges.push_back(this);
}


// This method disconnects the hyperedge tree edge nodes that it's attached to.
//
void HyperEdgeTreeEdge::disconnectEdge(void)
{
    COLA_ASSERT(ends.first != NULL);
    COLA_ASSERT(ends.second != NULL);

    ends.first->disconnectEdge(this);
    ends.second->disconnectEdge(this);
    ends.first = NULL;
    ends.second = NULL;
}


// This method traverses the hyperedge tree and removes from treeRoots any
// junction nodes.
//
void HyperEdgeTreeEdge::removeOtherJunctionsFrom(HyperEdgeTreeNode *ignored,
                    JunctionSet& treeRoots)
{
    if (ends.first && (ends.first != ignored))
    {
        ends.first->removeOtherJunctionsFrom(this, treeRoots);
        if (ends.first->junction)
        {
            treeRoots.erase(ends.first->junction);
        }
    }

    if (ends.second && (ends.second != ignored))
    {
        ends.second->removeOtherJunctionsFrom(this, treeRoots);
        if (ends.second->junction)
        {
            treeRoots.erase(ends.second->junction);
        }
    }
}


// This method traverses the hyperedge tree, clearing up the objects and
// memory used to store the tree.
//
void HyperEdgeTreeEdge::deleteNodesExcept(HyperEdgeTreeNode *ignored)
{
    if (ends.first && (ends.first != ignored))
    {
        ends.first->deleteEdgesExcept(this);
        delete ends.first;
    }
    ends.first = NULL;

    if (ends.second && (ends.second != ignored))
    {
        ends.second->deleteEdgesExcept(this);
        delete ends.second;
    }
    ends.second = NULL;
}


CmpNodesInDim::CmpNodesInDim(const size_t dim)
    : m_dimension(dim)
{
}


// Nodes in set are ordered by position along a line in a certain dimension, 
// and then by Node pointer since multiple may exist at a particular position.
bool CmpNodesInDim::operator()(const HyperEdgeTreeNode *lhs,
        const HyperEdgeTreeNode *rhs) const
{
    if (lhs->point[m_dimension] != rhs->point[m_dimension])
    {
        return lhs->point[m_dimension] < rhs->point[m_dimension];
    }
    return lhs < rhs;
}

}

