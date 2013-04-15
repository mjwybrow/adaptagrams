/*
 * vim: ts=4 sw=4 et tw=0 wm=0
 *
 * libavoid - Fast, Incremental, Object-avoiding Line Router
 *
 * Copyright (C) 2009-2012  Monash University
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

#include "libavoid/scanline.h"
#include "libavoid/obstacle.h"
#include "libavoid/vertices.h"
#include "libavoid/connector.h"
#include "libavoid/router.h"

namespace Avoid {

bool CmpNodePos::operator() (const Node* u, const Node* v) const 
{
    if (u->pos != v->pos) 
    {
        return u->pos < v->pos;
    }
    
    // Use the pointers to the base objects to differentiate them.
    void *up = (u->v) ? (void *) u->v : 
            ((u->c) ? (void *) u->c : (void *) u->ss);
    void *vp = (v->v) ? (void *) v->v : 
            ((v->c) ? (void *) v->c : (void *) v->ss);
    return up < vp;
}


Node::Node(Obstacle *v, const double p)
    : v(v),
      c(NULL),
      ss(NULL),
      pos(p),
      firstAbove(NULL),
      firstBelow(NULL)
{
    Box bBox = v->routingBox();
    min[XDIM] = bBox.min.x;
    min[YDIM] = bBox.min.y;
    max[XDIM] = bBox.max.x;
    max[YDIM] = bBox.max.y;
    //COLA_ASSERT(r->width()<1e40);
}

Node::Node(VertInf *c, const double p)
    : v(NULL),
      c(c),
      ss(NULL),
      pos(p),
      firstAbove(NULL),
      firstBelow(NULL)
{
    min[XDIM] = max[XDIM] = c->point.x;
    min[YDIM] = max[YDIM] = c->point.y;
}

Node::Node(ShiftSegment *ss, const double p)
    : v(NULL),
      c(NULL),
      ss(ss),
      pos(p),
      firstAbove(NULL),
      firstBelow(NULL)
{
    // These values shouldn't ever be used, so they don't matter.
    min[XDIM] = max[XDIM] = min[YDIM] = max[YDIM] = 0;
}

Node::~Node() 
{
}

// Find the first Node above in the scanline that is a shape edge,
// and does not have an open or close event at this position (meaning
// it is just about to be removed).
double Node::firstObstacleAbove(size_t dim)
{
    Node *curr = firstAbove;
    while (curr && (curr->ss || (curr->max[dim] > pos)))
    {
        curr = curr->firstAbove;
    }
   
    if (curr)
    {
        return curr->max[dim];
    }
    return -DBL_MAX;
}

// Find the first Node below in the scanline that is a shape edge,
// and does not have an open or close event at this position (meaning
// it is just about to be removed).
double Node::firstObstacleBelow(size_t dim)
{
    Node *curr = firstBelow;
    while (curr && (curr->ss || (curr->min[dim] < pos)))
    {
        curr = curr->firstBelow;
    }
    
    if (curr)
    {
        return curr->min[dim];
    }
    return DBL_MAX;
}

// Mark all connector segments above in the scanline as being able 
// to see to this shape edge.
void Node::markShiftSegmentsAbove(size_t dim)
{
    Node *curr = firstAbove;
    while (curr && (curr->ss || (curr->pos > min[dim])))
    {
        if (curr->ss && (curr->pos <= min[dim]))
        {
            curr->ss->maxSpaceLimit = 
                    std::min(min[dim], curr->ss->maxSpaceLimit);
        }
        curr = curr->firstAbove;
    }
}

// Mark all connector segments below in the scanline as being able 
// to see to this shape edge.
void Node::markShiftSegmentsBelow(size_t dim)
{
    Node *curr = firstBelow;
    while (curr && (curr->ss || (curr->pos < max[dim])))
    {
        if (curr->ss && (curr->pos >= max[dim]))
        {
            curr->ss->minSpaceLimit = 
                    std::max(max[dim], curr->ss->minSpaceLimit);
        }
        curr = curr->firstBelow;
    }
}

void Node::findFirstPointAboveAndBelow(const size_t dim, const double linePos,
        double& firstAbovePos, double& firstBelowPos, 
        double& lastAbovePos, double& lastBelowPos)
{
    firstAbovePos = -DBL_MAX;
    firstBelowPos = DBL_MAX;
    // We start looking left from the right side of the shape, 
    // and vice versa. 
    lastAbovePos = max[dim];
    lastBelowPos = min[dim];

    Node *curr = NULL;
    bool eventsAtSamePos = false;
    for (int direction = 0; direction < 2; ++direction)
    {
        // Look for obstacles in one direction, then the other.
        curr = (direction == 0) ? firstAbove: firstBelow;

        while (curr)
        {
            // The events are at a shared beginning or end of a shape or 
            // connection point.  Note, connection points have the same 
            // min and max value in the !dim dimension.
            eventsAtSamePos = 
                    (((linePos == max[!dim]) && 
                      (linePos == curr->max[!dim])) || 
                     ((linePos == min[!dim]) && 
                      (linePos == curr->min[!dim])));
            
            if (curr->max[dim] <= min[dim])
            {
                // Curr shape is completely to the left, 
                // so add it's right side as a limit
                firstAbovePos = std::max(curr->max[dim], firstAbovePos);
            }
            else if (curr->min[dim] >= max[dim])
            {
                // Curr shape is completely to the right, 
                // so add it's left side as a limit
                firstBelowPos = std::min(curr->min[dim], firstBelowPos);
            }
            else if (!eventsAtSamePos)
            {
                // Shapes that open or close at the same position do not
                // block visibility, so if they are not at same position
                // determine where they overlap.
                lastAbovePos = std::min(curr->min[dim], lastAbovePos);
                lastBelowPos = std::max(curr->max[dim], lastBelowPos);
            }
            curr = (direction == 0) ? curr->firstAbove : curr->firstBelow;
        }
    }    
}

double Node::firstPointAbove(size_t dim) 
{
    // We are looking for the first obstacle above this position,
    // though we ignore shape edges if this point is inline with
    // the edge of the obstacle.  That is, points have visibility
    // along the edge of shapes.
    size_t altDim = (dim + 1) % 2;
    double result = -DBL_MAX;
    Node *curr = firstAbove; 
    while (curr) 
    {
        bool inLineWithEdge = (min[altDim] == curr->min[altDim]) ||
                (min[altDim] == curr->max[altDim]);
        if ( ! inLineWithEdge && (curr->max[dim] <= pos) )
        {
            result = std::max(curr->max[dim], result);
        }
        curr = curr->firstAbove; 
    } 
    return result; 
}

double Node::firstPointBelow(size_t dim) 
{ 
    // We are looking for the first obstacle below this position,
    // though we ignore shape edges if this point is inline with
    // the edge of the obstacle.  That is, points have visibility
    // along the edge of shapes.
    size_t altDim = (dim + 1) % 2;
    double result = DBL_MAX;
    Node *curr = firstBelow; 
    while (curr) 
    { 
        bool inLineWithEdge = (min[altDim] == curr->min[altDim]) ||
                (min[altDim] == curr->max[altDim]);
        if ( ! inLineWithEdge && (curr->min[dim] >= pos) )
        {
            result = std::min(curr->min[dim], result);
        }
        curr = curr->firstBelow; 
    }
    return result;
}

// This is a bit inefficient, but we won't need to do it once we have 
// connection points.
bool Node::isInsideShape(size_t dimension)
{
    for (Node *curr = firstBelow; curr; curr = curr->firstBelow)
    {
        if ((curr->min[dimension] < pos) && (pos < curr->max[dimension]))
        {
            return true;
        }
    }
    for (Node *curr = firstAbove; curr; curr = curr->firstAbove)
    {
        if ((curr->min[dimension] < pos) && (pos < curr->max[dimension]))
        {
            return true;
        }
    }
    return false;
}


Event::Event(EventType t, Node *v, double p) 
    : type(t),
      v(v),
      pos(p)
{
}


// Used for quicksort.  Must return <0, 0, or >0.
int compare_events(const void *a, const void *b)
{
	Event *ea = *(Event**) a;
	Event *eb = *(Event**) b;
    if (ea->pos != eb->pos)
    {
        return (ea->pos < eb->pos) ? -1 : 1;
    }
    if (ea->type != eb->type)
    {
        return ea->type - eb->type;
    }
    COLA_ASSERT(ea->v != eb->v);
    return ea->v - eb->v;
}


void buildConnectorRouteCheckpointCache(Router *router)
{
    for (ConnRefList::const_iterator curr = router->connRefs.begin(); 
            curr != router->connRefs.end(); ++curr) 
    {
        ConnRef *conn = *curr;
        if (conn->routingType() != ConnType_Orthogonal)
        {
            continue;
        }

        PolyLine& displayRoute = conn->displayRoute();
        std::vector<Checkpoint> checkpoints = conn->routingCheckpoints();
       
        // Initialise checkpoint vector and set to false.  There will be
        // one entry for each *segment* in the path, and the value indicates
        // whether the segment is affected by a checkpoint.
        displayRoute.checkpointsOnRoute = 
                std::vector<std::pair<size_t, Point> >();

        for (size_t ind = 0; ind < displayRoute.size(); ++ind)
        {
            if (ind > 0)
            {
                for (size_t cpi = 0; cpi < checkpoints.size(); ++cpi)
                {
                    if (pointOnLine(displayRoute.ps[ind - 1], 
                             displayRoute.ps[ind], checkpoints[cpi].point) )
                    {
                        // The checkpoint is on a segment.
                        displayRoute.checkpointsOnRoute.push_back(
                                std::make_pair((ind * 2) - 1, 
                                    checkpoints[cpi].point));
                    }
                }
            }

            for (size_t cpi = 0; cpi < checkpoints.size(); ++cpi)
            {
                if (displayRoute.ps[ind].equals(checkpoints[cpi].point))
                {
                    // The checkpoint is at a bendpoint.
                    displayRoute.checkpointsOnRoute.push_back(
                            std::make_pair(ind * 2, checkpoints[cpi].point));
                }
            }
        }
    }
}


void clearConnectorRouteCheckpointCache(Router *router)
{
    for (ConnRefList::const_iterator curr = router->connRefs.begin(); 
            curr != router->connRefs.end(); ++curr) 
    {
        ConnRef *conn = *curr;
        if (conn->routingType() != ConnType_Orthogonal)
        {
            continue;
        }

        // Clear the cache.
        PolyLine& displayRoute = conn->displayRoute();
        displayRoute.checkpointsOnRoute.clear();
    }
}


}

