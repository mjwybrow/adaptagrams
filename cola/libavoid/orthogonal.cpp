/*
 * vim: ts=4 sw=4 et tw=0 wm=0
 *
 * libavoid - Fast, Incremental, Object-avoiding Line Router
 *
 * Copyright (C) 2009-2010  Monash University
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


#include <cstdlib>
#include <cfloat>
#include <cmath>
#include <set>
#include <list>
#include <algorithm>

#include "libavoid/router.h"
#include "libavoid/geomtypes.h"
#include "libavoid/shape.h"
#include "libavoid/orthogonal.h"
#include "libavoid/connend.h"
#include "libavoid/connector.h"
#include "libavoid/vpsc.h"
#include "libavoid/assertions.h"

#ifdef LIBAVOID_SDL
  #include <SDL_gfxPrimitives.h>
#endif


namespace Avoid {


static const double CHANNEL_MAX = 100000000;


class ShiftSegment 
{
    public:
        // For shiftable segments.
        ShiftSegment(ConnRef *conn, const size_t low, const size_t high, 
                bool isSBend, const size_t dim, double minLim, double maxLim)
            : connRef(conn),
              indexLow(low),
              indexHigh(high),
              sBend(isSBend),
              fixed(false),
              dimension(dim),
              variable(NULL),
              minSpaceLimit(minLim),
              maxSpaceLimit(maxLim)
        {
        }
        // For fixed segments.
        ShiftSegment(ConnRef *conn, const size_t low, const size_t high, 
                const size_t dim)
            : connRef(conn),
              indexLow(low),
              indexHigh(high),
              sBend(false),
              fixed(true),
              dimension(dim),
              variable(NULL)
        {
            // This has no space to shift.
            minSpaceLimit = lowPoint()[dim];
            maxSpaceLimit = lowPoint()[dim];
        }
        Point& lowPoint(void)
        {
            return connRef->displayRoute().ps[indexLow];
        }
        Point& highPoint(void)
        {
            return connRef->displayRoute().ps[indexHigh];
        }
        const Point& lowPoint(void) const
        {
            return connRef->displayRoute().ps[indexLow];
        }
        const Point& highPoint(void) const 
        {
            return connRef->displayRoute().ps[indexHigh];
        }
        double nudgeDistance(void) const
        {
            return connRef->router()->orthogonalNudgeDistance();
        }
        const int fixedOrder(bool& isFixed) const
        {
            double nudgeDist = nudgeDistance();
            double pos = lowPoint()[dimension];
            bool minLimited = ((pos - minSpaceLimit) < nudgeDist);
            bool maxLimited = ((maxSpaceLimit - pos) < nudgeDist);
            
            if (fixed || (minLimited && maxLimited))
            {
                isFixed = true;
                return 0;
            }
            else if (minLimited)
            {
                return 1;
            }
            else if (maxLimited)
            {
                return -1;
            }
            return 0;
        }
        const int order(void) const
        {
            if (lowC())
            {
                return -1;
            }
            else if (highC())
            {
                return 1;
            }
            return 0;
        }
        bool operator<(const ShiftSegment& rhs) const
        {
            const Point& lowPt = lowPoint();
            const Point& rhsLowPt = rhs.lowPoint();
            
            if (lowPt[dimension] != rhsLowPt[dimension])
            {
                return lowPt[dimension] < rhsLowPt[dimension];
            }
            return this < &rhs;
        }
        // This counts segments that are collinear and share an endpoint as
        // overlapping.  This allows them to be nudged apart where possible.
        bool overlapsWith(const ShiftSegment& rhs, const size_t dim) const
        {
            size_t altDim = (dim + 1) % 2;
            const Point& lowPt = lowPoint();
            const Point& highPt = highPoint();
            const Point& rhsLowPt = rhs.lowPoint();
            const Point& rhsHighPt = rhs.highPoint();
            if ( (lowPt[altDim] <= rhsHighPt[altDim]) &&
                    (rhsLowPt[altDim] <= highPt[altDim]))
            {
                if ( (minSpaceLimit <= rhs.maxSpaceLimit) &&
                        (rhs.minSpaceLimit <= maxSpaceLimit))
                {
                    return true;
                }
            }
            return false;
        }

        ConnRef *connRef;
        size_t indexLow;
        size_t indexHigh;
        bool sBend;
        bool fixed;
        size_t dimension;
        Variable *variable;
        double minSpaceLimit;
        double maxSpaceLimit;
    private:
        const bool lowC(void) const
        {
            // This is true if this is a cBend and its adjoining points
            // are at lower positions.
            if (!sBend && !fixed && (minSpaceLimit == lowPoint()[dimension]))
            {
                return true;
            }
            return false;
        }
        const bool highC(void) const
        {
            // This is true if this is a cBend and its adjoining points
            // are at higher positions.
            if (!sBend && !fixed && (maxSpaceLimit == lowPoint()[dimension]))
            {
                return true;
            }
            return false;
        }
};
typedef std::list<ShiftSegment> ShiftSegmentList;

bool cmpShiftSegment(const ShiftSegment& u, const ShiftSegment& v)
{
    return u < v;
}


struct Node;
struct CmpNodePos { bool operator()(const Node* u, const Node* v) const; };

typedef std::set<Node*,CmpNodePos> NodeSet;
struct Node 
{
    Obstacle *v;
    VertInf *c;
    ShiftSegment *ss;
    double pos;
    double min[2], max[2];
    Node *firstAbove, *firstBelow;
    NodeSet::iterator iter;

    Node(Obstacle *v, const double p)
        : v(v),
          c(NULL),
          ss(NULL),
          pos(p),
          firstAbove(NULL),
          firstBelow(NULL)
    {   
        //COLA_ASSERT(r->width()<1e40);
        v->polygon().getBoundingRect(
                &min[XDIM], &min[YDIM], &max[XDIM], &max[YDIM]);
    }   
    Node(VertInf *c, const double p)
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
    Node(ShiftSegment *ss, const double p)
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
    ~Node() 
    {
    }
    // Find the first Node above in the scanline that is a shape edge,
    // and does not have an open or close event at this position (meaning
    // it is just about to be removed).
    double firstObstacleAbove(size_t dim)
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
    double firstObstacleBelow(size_t dim)
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
    void markShiftSegmentsAbove(size_t dim)
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
    void markShiftSegmentsBelow(size_t dim)
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
    void findFirstPointAboveAndBelow(const size_t dim, const double linePos,
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
                // min and max value in the the !dim dimension.
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
    double firstPointAbove(size_t dim) 
    {
        double result = -DBL_MAX;
        Node *curr = firstAbove; 
        while (curr) 
        {
            if (curr->max[dim] < pos)
            {
                result = std::max(curr->max[dim], result);
            }
            curr = curr->firstAbove; 
        } 
        return result; 
    } 
    double firstPointBelow(size_t dim) 
    { 
        double result = DBL_MAX;
        Node *curr = firstBelow; 
        while (curr) 
        { 
            if (curr->min[dim] > pos)
            {
                result = std::min(curr->min[dim], result);
            }
            curr = curr->firstBelow; 
        }
        return result;
    } 
    // This is a bit inefficient, but we won't need to do it once we have 
    // connection points.
    bool isInsideShape(size_t dimension)
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
};


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


// Note: Open must come first.
typedef enum {
    Open = 1,
    SegOpen = 2,
    ConnPoint = 3, 
    SegClose = 4,
    Close = 5
} EventType;


struct Event
{
    Event(EventType t, Node *v, double p) 
        : type(t),
          v(v),
          pos(p)
    {};
    EventType type;
    Node *v;
    double pos;
};

Event **events;


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


enum ScanVisDirFlag {
    VisDirNone  = 0,
    VisDirUp    = 1,
    VisDirDown  = 2
};
typedef unsigned int ScanVisDirFlags;


// Returns a bitfield of the directions of visibility in terms of the scanline
// in a particular dimension dimension.  It will return either ConnDirDown 
// (meaning visibility to lower position values) or ConnDirUp (for visibility 
// towards higher position values).
//
static ScanVisDirFlags getPosVertInfDirections(VertInf *v, size_t dim)
{
    if (dim == XDIM) // X-dimension
    {
        unsigned int dirs = v->visDirections & (ConnDirLeft | ConnDirRight);
        if (dirs == (ConnDirLeft | ConnDirRight))
        {
            return (VisDirDown | VisDirUp);
        }
        else if (dirs == ConnDirLeft)
        {
            return VisDirDown;
        }
        else if (dirs == ConnDirRight)
        {
            return VisDirUp;
        }
    }
    else if (dim == YDIM) // Y-dimension
    {
        unsigned int dirs = v->visDirections & (ConnDirDown | ConnDirUp);
        if (dirs == (ConnDirDown | ConnDirUp))
        {
            return (VisDirDown | VisDirUp);
        }
        else if (dirs == ConnDirDown)
        {
            // libavoid's Y-axis points downwards, so where the user has 
            // specified visibility downwards, this results in visibility to
            // higher scanline positition values. 
            return VisDirUp;
        }
        else if (dirs == ConnDirUp)
        {
            // libavoid's Y-axis points downwards, so where the user has 
            // specified visibility upwards, this results in visibility to
            // lower scanline positition values. 
            return VisDirDown;
        }
    }

    // Can occur for ConnDirNone visibility.
    return VisDirNone;
}


struct PosVertInf
{
    PosVertInf(double p, VertInf *vI, ScanVisDirFlags d = VisDirNone)
        : pos(p),
          vert(vI),
          dirs(d)
    {
    }
    
    bool operator<(const PosVertInf& rhs) const 
    {
        if (pos != rhs.pos)
        {
            return pos < rhs.pos;
        }
        if ((vert->id == rhs.vert->id) && (vert->id == dummyOrthogID))
        {
            // Multiple dummy nodes can get placed at the same point for 
            // multiple ShapeConnectionPins on junctions (outside of shapes).
            // We only need one at each position, so multiples can be seen
            // as equal here.
            return false;
        }
        return vert < rhs.vert;
    }

    double pos;
    VertInf *vert;

    // A bitfield marking the direction of visibility (in this dimension)
    // made up of VisDirDown (for visibility towards lower position values) 
    // and VisDirUp (for visibility towards higher position values).
    //
    ScanVisDirFlags dirs;
};


struct CmpVertInf
{ 
    bool operator()(const VertInf* u, const VertInf* v) const
    {
        // Comparator for VertSet, an ordered set of VertInf pointers.
        // It is assumed vertical sets of points will all have the same
        // x position and horizontal sets all share a y position, so this
        // method can be used to sort both these sets.
        COLA_ASSERT((u->point.x == v->point.x) || (u->point.y == v->point.y));
        if (u->point.x != v->point.x)
        {
            return u->point.x < v->point.x;
        }
        else if (u->point.y != v->point.y)
        {
            return u->point.y < v->point.y;
        }
        return u < v;
    }
};


typedef std::set<VertInf *, CmpVertInf> VertSet;

// A set of points to break the line segment, 
// along with vertices for these points.
typedef std::set<PosVertInf> BreakpointSet;

// Temporary structure used to store the possible horizontal visibility 
// lines arising from the vertical sweep.
class LineSegment 
{
public:
    LineSegment(const double& b, const double& f, const double& p, 
            bool ss = false, VertInf *bvi = NULL, VertInf *fvi = NULL)
        : begin(b),
          finish(f),
          pos(p),
          shapeSide(false)
    {
        COLA_ASSERT(begin < finish);

        if (bvi)
        {
            vertInfs.insert(bvi);
        }
        if (fvi)
        {
            vertInfs.insert(fvi);
        }
    }
    LineSegment(const double& bf, const double& p, VertInf *bfvi = NULL)
        : begin(bf),
          finish(bf),
          pos(p),
          shapeSide(false)
    {
        if (bfvi)
        {
            vertInfs.insert(bfvi);
        }
    }
 
    // Order by begin, pos, finish.
    bool operator<(const LineSegment& rhs) const 
    {
        if (begin != rhs.begin)
        {
            return begin < rhs.begin;
        }
        if (pos != rhs.pos)
        {
            return pos < rhs.pos;
        }
        if (finish != rhs.finish)
        {
            return finish < rhs.finish;
        }
        COLA_ASSERT(shapeSide == rhs.shapeSide);
        return false;
    }

    bool overlaps(const LineSegment& rhs) const
    {
        if ((begin == rhs.begin) && (pos == rhs.pos) &&
                (finish == rhs.finish))
        {
            // Lines are exactly equal.
            return true;
        }
        
        if (pos == rhs.pos)
        {
            if (((begin >= rhs.begin) && (begin <= rhs.finish)) ||
                ((rhs.begin >= begin) && (rhs.begin <= finish)) )
            {
                // They are colinear and overlap by some amount.
                return true;
            }
        }
        return false;
    }

    void mergeVertInfs(const LineSegment& segment)
    {
        begin = std::min(begin, segment.begin);
        finish = std::max(finish, segment.finish);
        vertInfs.insert(segment.vertInfs.begin(), segment.vertInfs.end());
    }
    
    VertInf *beginVertInf(void) const
    {
        if (vertInfs.empty())
        {
            return NULL;
        }
        VertInf *inf = *vertInfs.begin();
        if ( ((inf->point.y == begin) && (inf->point.x == pos)) ||
             ((inf->point.x == begin) && (inf->point.y == pos)) )
        {
            // Only return the point if it is actually at the begin pos.
            return inf;
        }
        return NULL;
    }
    VertInf *finishVertInf(void) const
    {
        if (vertInfs.empty())
        {
            return NULL;
        }
        VertInf *inf = *vertInfs.rbegin();
        if ( ((inf->point.y == finish) && (inf->point.x == pos)) ||
             ((inf->point.x == finish) && (inf->point.y == pos)) )
        {
            // Only return the point if it is actually at the finish pos.
            return inf;
        }
        return NULL;
    }

    VertInf *commitPositionX(Router *router, double posX)
    {
        VertInf *found = NULL;
        for (VertSet::iterator v = vertInfs.begin();
                v != vertInfs.end(); ++v)
        {
            if ((*v)->point.x == posX)
            {
                found = *v;
                break;
            }
        }
        if (!found)
        {
            found = new VertInf(router, dummyOrthogID, Point(posX, pos));
            vertInfs.insert(found);
        }
        return found;
    }
    // Set begin endpoint vertex if none has been assigned.
    void horiCommitBegin(Router *router, VertInf *vert = NULL)
    {
        if (vert)
        {
            vertInfs.insert(vert);
        }

        if (vertInfs.empty() ||
                ((*vertInfs.begin())->point.x != begin))
        {
            if (begin != -DBL_MAX)
            {
                vertInfs.insert(new
                        VertInf(router, dummyOrthogID, Point(begin, pos)));
            }
        }
    }

    // Set begin endpoint vertex if none has been assigned.
    void horiCommitFinish(Router *router, VertInf *vert = NULL)
    {
        if (vert)
        {
            vertInfs.insert(vert);
        }

        if (vertInfs.empty() ||
                ((*vertInfs.rbegin())->point.x != finish))
        {
            if (finish != DBL_MAX)
            {
                vertInfs.insert(new
                        VertInf(router, dummyOrthogID, Point(finish, pos)));
            }
        }
    }

    // Converts a section of the points list to a set of breakPoints.  
    // Returns the first of the intersection points occuring at finishPos.
    VertSet::iterator addSegmentsUpTo(Router *router, double finishPos)
    {
        VertSet::iterator firstIntersectionPt = vertInfs.end();
        for (VertSet::iterator vert = vertInfs.begin(); 
                vert != vertInfs.end(); ++vert)
        {
            if ((*vert)->point.x > finishPos)
            {
                // We're done.
                break;
            }
            
            breakPoints.insert(PosVertInf((*vert)->point.x, (*vert),
                        getPosVertInfDirections(*vert, XDIM)));

            if ((firstIntersectionPt == vertInfs.end()) && 
                    ((*vert)->point.x == finishPos))
            {
                firstIntersectionPt = vert;
            }
        }
        // Returns the first of the intersection points at finishPos.
        return firstIntersectionPt;
    }

    // Add visibility edge(s) for this segment.  There may be multiple if 
    // one of the endpoints is shared by multiple connector endpoints.
    void addEdgeHorizontal(Router *router)
    {
        horiCommitBegin(router);
        horiCommitFinish(router);
        
        addSegmentsUpTo(router, finish);
    }

    // Set flags to show what can be passed on this visibility line.
    // This can be used later to disregard some edges in the visibility
    // graph when routing particular connectors.
    void setLongRangeVisibilityFlags(size_t dim)
    {
        // First, travel in one direction
        bool seenConnPt = false;
        bool seenShapeEdge = false;
        for (BreakpointSet::iterator nvert = breakPoints.begin(); 
                nvert != breakPoints.end(); ++nvert)
        {
            VertIDProps mask = 0;
            if (dim == XDIM)
            {
                if (seenConnPt)
                {
                    mask |= XL_CONN;
                }
                if (seenShapeEdge)
                {
                    mask |= XL_EDGE;
                }
            }
            else // if (dim == YDIM)
            {
                if (seenConnPt)
                {
                    mask |= YL_CONN;
                }
                if (seenShapeEdge)
                {
                    mask |= YL_EDGE;
                }
            }
            nvert->vert->orthogVisPropFlags |= mask;

            if (nvert->vert->id.isConnPt())
            {
                seenConnPt = true;
            }
            if (nvert->vert->id.isOrthShapeEdge())
            {
                seenShapeEdge = true;
            }
        }
        // Then in the other direction
        seenConnPt = false;
        seenShapeEdge = false;
        for (BreakpointSet::reverse_iterator rvert = breakPoints.rbegin(); 
                rvert != breakPoints.rend(); ++rvert)
        {
            VertIDProps mask = 0;
            if (dim == XDIM)
            {
                if (seenConnPt)
                {
                    mask |= XH_CONN;
                }
                if (seenShapeEdge)
                {
                    mask |= XH_EDGE;
                }
            }
            else // if (dim == YDIM)
            {
                if (seenConnPt)
                {
                    mask |= YH_CONN;
                }
                if (seenShapeEdge)
                {
                    mask |= YH_EDGE;
                }
            }
            rvert->vert->orthogVisPropFlags |= mask;

            if (rvert->vert->id.isConnPt())
            {
                seenConnPt = true;
            }
            if (rvert->vert->id.isOrthShapeEdge())
            {
                seenShapeEdge = true;
            }
        }
    }

    // Add visibility edge(s) for this segment up until an intersection.
    // Then, move the segment beginning to the intersection point, so we
    // later only consider the remainder of the segment.
    // There may be multiple segments added to the graph if the beginning 
    // endpoint of the segment is shared by multiple connector endpoints.
    VertSet addEdgeHorizontalTillIntersection(Router *router, 
            LineSegment& vertLine)
    {
        VertSet intersectionSet;

        horiCommitBegin(router);

        // Does a vertex already exist for this point.
        commitPositionX(router, vertLine.pos);
   
        // Generate segments and set end iterator to the first point 
        // at the intersection position.
        VertSet::iterator restBegin = addSegmentsUpTo(router, vertLine.pos);

        // Add the intersections points to intersectionSet.
        VertSet::iterator restEnd = restBegin;
        while ((restEnd != vertInfs.end()) && 
                (*restEnd)->point.x == vertLine.pos)
        {
            ++restEnd;
        }
        intersectionSet.insert(restBegin, restEnd);

        // Adjust segment to remove processed portion.
        begin = vertLine.pos;
        vertInfs.erase(vertInfs.begin(), restBegin);

        return intersectionSet;
    }
                
    // Insert vertical breakpoints.
    void insertBreakpointsBegin(Router *router, LineSegment& vertLine)
    {
        VertInf *vert = NULL;
        if (pos == vertLine.begin && vertLine.beginVertInf())
        {
            vert = vertLine.beginVertInf();
        }
        else if (pos == vertLine.finish && vertLine.finishVertInf())
        {
            vert = vertLine.finishVertInf();
        }
        horiCommitBegin(router, vert);

        for (VertSet::iterator v = vertInfs.begin();
                v != vertInfs.end(); ++v)
        {
            if ((*v)->point.x == begin)
            {
                vertLine.breakPoints.insert(PosVertInf(pos, *v, 
                        getPosVertInfDirections(*v, YDIM)));
            }
        }
    }

    // Insert vertical breakpoints.
    void insertBreakpointsFinish(Router *router, LineSegment& vertLine)
    {
        VertInf *vert = NULL;
        if (pos == vertLine.begin && vertLine.beginVertInf())
        {
            vert = vertLine.beginVertInf();
        }
        else if (pos == vertLine.finish && vertLine.finishVertInf())
        {
            vert = vertLine.finishVertInf();
        }
        horiCommitFinish(router, vert);

        for (VertSet::iterator v = vertInfs.begin();
                v != vertInfs.end(); ++v)
        {
            if ((*v)->point.x == finish)
            {
                vertLine.breakPoints.insert(PosVertInf(pos, *v,
                        getPosVertInfDirections(*v, YDIM)));
            }
        }
    }
    void generateVisibilityEdgesFromBreakpointSet(Router *router, size_t dim)
    {
        if (breakPoints.empty() || ((breakPoints.begin())->pos > begin))
        {
            // Add a begin point if there was not already an intersection
            // found at that point. Though, don't do this if the line
            // segment goes off to infinity -- we can't reach anything
            // by going in that direction!
            if (begin == -DBL_MAX)
            {
                // Shorten line to first intersection point.
                COLA_ASSERT(!breakPoints.empty());
                begin = breakPoints.begin()->pos;
            }
            else
            {
                // Add begin point.
                Point point(pos, pos);
                point[dim] = begin;
                VertInf *vert = new VertInf(router, dummyOrthogID, point);
                breakPoints.insert(PosVertInf(begin, vert));
            }
        }
        if (breakPoints.empty() || ((breakPoints.rbegin())->pos < finish))
        {
            // Add a finish point if there was not already an intersection
            // found at that point. Though, don't do this if the line
            // segment goes off to infinity -- we can't reach anything
            // by going in that direction!
            if (finish == DBL_MAX)
            {
                // Shorten line to first intersection point.
                finish = breakPoints.rbegin()->pos;
            }
            else
            {
                // Add begin point.
                Point point(pos, pos);
                point[dim] = finish;
                VertInf *vert = new VertInf(router, dummyOrthogID, point);
                breakPoints.insert(PosVertInf(finish, vert));
            }
        }

        // Set flags for orthogonal routing optimisation.
        setLongRangeVisibilityFlags(dim);

        const bool orthogonal = true;
        BreakpointSet::iterator vert, last;
#if 0
        last = breakPoints.end();
        for (vert = breakPoints.begin(); vert != breakPoints.end();)
        {
            if (vert->vert->id == dummyOrthogID)
            {
                if (last == breakPoints.end() || 
                        (last->vert->point != vert->vert->point))
                {
                    last = vert;
                }
                else
                {
                    // Already seen a dummy orthogonal point at this 
                    // position, delete it.

            }
            else
            {
                ++vert;
            }
        }
#endif
        for (vert = last = breakPoints.begin(); vert != breakPoints.end();)
        {
            BreakpointSet::iterator firstPrev = last;
            while (last->vert->point[dim] != vert->vert->point[dim])
            {
                COLA_ASSERT(vert != last);
                // Assert points are not at the same position.
                COLA_ASSERT(vert->vert->point != last->vert->point);

                if (vert->vert->id.isConnPt() && last->vert->id.isConnPt())
                {
                    // Here we have a pair of two endpoints that are both
                    // connector endpoints and both are inside a shape.
                    
                    // Give vert visibility back to the first non-connector
                    // endpoint vertex (i.e., the side of the shape).
                    BreakpointSet::iterator side = last;
                    while (side->vert->id.isConnPt())
                    {
                        if (side == breakPoints.begin())
                        {
                            break;
                        }
                        --side;
                    }
                    bool canSeeDown = (vert->dirs & VisDirDown);
                    if (canSeeDown && !(side->vert->id.isConnPt()))
                    {
                        EdgeInf *edge = new 
                                EdgeInf(side->vert, vert->vert, orthogonal);
                        edge->setDist(vert->vert->point[dim] - 
                                side->vert->point[dim]);
                    }

                    // Give last visibility back to the first non-connector
                    // endpoint vertex (i.e., the side of the shape).
                    side = vert;
                    while ((side != breakPoints.end()) && 
                            side->vert->id.isConnPt())
                    {
                        ++side;
                    }
                    bool canSeeUp = (last->dirs & VisDirUp);
                    if (canSeeUp && (side != breakPoints.end()))
                    {
                        EdgeInf *edge = new 
                                EdgeInf(last->vert, side->vert, orthogonal);
                        edge->setDist(side->vert->point[dim] - 
                                last->vert->point[dim]);
                    }
                }
                
                // The normal case.
                //
                // Note: It's okay to give two connector endpoints visbility 
                // here since we only consider the partner endpoint as a 
                // candidate while searching if it is the other endpoint of
                // the connector in question.
                //
                bool generateEdge = true;
                if (last->vert->id.isConnPt() && !(last->dirs & VisDirUp))
                {
                    // Don't generate the visibility segment if the ConnEnd
                    // doesn't have visibility in that direction.
                    generateEdge = false;
                }
                else if (vert->vert->id.isConnPt() && !(vert->dirs & VisDirDown))
                {
                    // Don't generate the visibility segment if the ConnEnd
                    // doesn't have visibility in that direction.
                    generateEdge = false;
                }
                if (generateEdge)
                {
                    EdgeInf *edge = 
                            new EdgeInf(last->vert, vert->vert, orthogonal);
                    edge->setDist(vert->vert->point[dim] - 
                            last->vert->point[dim]);
                }

                ++last;
            }

            ++vert;

            if ((vert != breakPoints.end()) &&
                    (last->vert->point[dim] == vert->vert->point[dim]))
            {
                // Still looking at same pair, just reset prev number pointer.
                last = firstPrev;
            }
            else
            {
                // vert has moved to the beginning of a group at a new 
                // position.  Last is now in the right place, so do nothing.
            }
        }
    }

    double begin;
    double finish;
    double pos;
    bool shapeSide;
    
    VertSet vertInfs;
    BreakpointSet breakPoints;
private:
	// MSVC wants to generate the assignment operator and the default 
	// constructor, but fails.  Therefore we declare them private and 
	// don't implement them.
    LineSegment & operator=(LineSegment const &);
    LineSegment();
};

typedef std::list<LineSegment> SegmentList;

class SegmentListWrapper
{
    public:
        LineSegment *insert(LineSegment segment)
        {
            SegmentList::iterator found = _list.end();
            for (SegmentList::iterator curr = _list.begin();
                    curr != _list.end(); ++curr)
            {
                if (curr->overlaps(segment))
                {
                    if (found != _list.end())
                    {
                        // This is not the first segment that overlaps,
                        // so we need to merge and then delete an existing
                        // segment.
                        curr->mergeVertInfs(*found);
                        _list.erase(found);
                        found = curr;
                    }
                    else
                    {
                        // This is the first overlapping segment, so just 
                        // merge the new segment with this one.
                        curr->mergeVertInfs(segment);
                        found = curr;
                    }
                }
            }

            if (found == _list.end())
            {
                // Add this line.
                _list.push_back(segment);
                return &(_list.back());
            }

            return &(*found);
        }
        SegmentList& list(void)
        {
            return _list;
        }
    private:
        SegmentList _list;
};


// Given a router instance and a set of possible horizontal segments, and a
// possible vertical visibility segment, compute and add edges to the
// orthogonal visibility graph for all the visibility edges.
static void intersectSegments(Router *router, SegmentList& segments, 
        LineSegment& vertLine)
{
    // XXX: It seems that this case can sometimes occur... maybe when
    // there are many overlapping rectangles.
    //COLA_ASSERT(vertLine.beginVertInf() == NULL);
    //COLA_ASSERT(vertLine.finishVertInf() == NULL);

    COLA_ASSERT(!segments.empty());
    for (SegmentList::iterator it = segments.begin(); it != segments.end(); )
    {
        LineSegment& horiLine = *it;

        bool inVertSegRegion = ((vertLine.begin <= horiLine.pos) &&
                                (vertLine.finish >= horiLine.pos));

        if (vertLine.pos < horiLine.begin)
        {
            // We've yet to reach this segment in the sweep, so ignore.
            ++it;
            continue;
        }
        else if (vertLine.pos == horiLine.begin)
        {
            if (inVertSegRegion)
            {
                horiLine.insertBreakpointsBegin(router, vertLine);
            }
        }
        else if (vertLine.pos == horiLine.finish)
        {
            if (inVertSegRegion)
            {
                // Add horizontal visibility segment.
                horiLine.addEdgeHorizontal(router);
            
                horiLine.insertBreakpointsFinish(router, vertLine);
                
                size_t dim = XDIM; // x-dimension
                horiLine.generateVisibilityEdgesFromBreakpointSet(router, dim);

                // And we've now finished with the segment, so delete.
                it = segments.erase(it);
                continue;
            }
        }
        else if (vertLine.pos > horiLine.finish)
        {
            // Add horizontal visibility segment.
            horiLine.addEdgeHorizontal(router);

            size_t dim = XDIM; // x-dimension
            horiLine.generateVisibilityEdgesFromBreakpointSet(router, dim);

            // We've now swept past this horizontal segment, so delete.
            it = segments.erase(it);
            continue;
        }
        else
        {
            COLA_ASSERT(vertLine.pos > horiLine.begin);
            COLA_ASSERT(vertLine.pos < horiLine.finish);

            if (inVertSegRegion)
            {
                // Add horizontal visibility segment.
                VertSet intersectionVerts = 
                        horiLine.addEdgeHorizontalTillIntersection(
                            router, vertLine);

                for (VertSet::iterator v = intersectionVerts.begin();
                        v != intersectionVerts.end(); ++v)
                {
                    vertLine.breakPoints.insert(PosVertInf(horiLine.pos, *v,
                            getPosVertInfDirections(*v, YDIM)));
                }
            }
        }
        ++it;
    }

    // Split breakPoints set into visibility segments.
    size_t dimension = YDIM; // y-dimension
    vertLine.generateVisibilityEdgesFromBreakpointSet(router, dimension);
}


// Processes an event for the vertical sweep used for computing the static 
// orthogonal visibility graph.  This adds possible visibility sgments to 
// the segments list.
// The first pass is adding the event to the scanline, the second is for
// processing the event and the third for removing it from the scanline.
static void processEventVert(Router *router, NodeSet& scanline, 
        SegmentListWrapper& segments, Event *e, unsigned int pass)
{
    Node *v = e->v;
    
    if ( ((pass == 1) && (e->type == Open)) ||
         ((pass == 2) && (e->type == ConnPoint)) )
    {
        std::pair<NodeSet::iterator, bool> result = scanline.insert(v);
        v->iter = result.first;
        COLA_ASSERT(result.second);

        NodeSet::iterator it = v->iter;
        // Work out neighbours
        if (it != scanline.begin()) 
        {
            Node *u = *(--it);
            v->firstAbove = u;
            u->firstBelow = v;
        }
        it = v->iter;
        if (++it != scanline.end()) 
        {
            Node *u = *it;
            v->firstBelow = u;
            u->firstAbove = v;
        }
    }
    
    if (pass == 2)
    {
        if ((e->type == Open) || (e->type == Close))
        {
            // Only difference between Open and Close is whether the line
            // segments are at the top or bottom of the shape.  Decide here.
            double lineY = (e->type == Open) ? v->min[YDIM] : v->max[YDIM];

            // Shape edge positions.
            double minShape = v->min[XDIM];
            double maxShape = v->max[XDIM];
            // As far as we can see.
            double minLimit, maxLimit;
            double minLimitMax, maxLimitMin;
            v->findFirstPointAboveAndBelow(XDIM, lineY, minLimit, maxLimit,
                    minLimitMax, maxLimitMin);

            // Insert possible visibility segments.
            if (minLimitMax >= maxLimitMin)
            {
                // These vertices represent the shape corners.
                VertInf *vI1 = new VertInf(router, dummyOrthogShapeID, 
                            Point(minShape, lineY));
                VertInf *vI2 = new VertInf(router, dummyOrthogShapeID, 
                            Point(maxShape, lineY));
                
                // There are no overlapping shapes, so give full visibility.
                if (minLimit < minShape)
                {
                    segments.insert(LineSegment(minLimit, minShape, lineY,
                                true, NULL, vI1));
                }
                segments.insert(LineSegment(minShape, maxShape, lineY, 
                            true, vI1, vI2));
                if (maxShape < maxLimit)
                {
                    segments.insert(LineSegment(maxShape, maxLimit, lineY,
                                true, vI2, NULL));
                }
            }
            else
            {
                // There are overlapping shapes along this shape edge.

                if ((minLimitMax > minLimit) && (minLimitMax >= minShape))
                {
                    LineSegment *line = segments.insert(
                            LineSegment(minLimit, minLimitMax, lineY, true));
                    // Shape corner:
                    VertInf *vI1 = new VertInf(router, dummyOrthogShapeID, 
                                Point(minShape, lineY));
                    line->vertInfs.insert(vI1);
                }
                if ((maxLimitMin < maxLimit) && (maxLimitMin <= maxShape))
                {
                    LineSegment *line = segments.insert(
                            LineSegment(maxLimitMin, maxLimit, lineY, true));
                    // Shape corner:
                    VertInf *vI2 = new VertInf(router, dummyOrthogShapeID, 
                                Point(maxShape, lineY));
                    line->vertInfs.insert(vI2);
                }
            }
        }
        else if (e->type == ConnPoint)
        {
            // Connection point.
            VertInf *centreVert = e->v->c;
            Point& cp = centreVert->point;

            // As far as we can see.
            double minLimit = v->firstPointAbove(XDIM);
            double maxLimit = v->firstPointBelow(XDIM);
            bool inShape = v->isInsideShape(XDIM);

            LineSegment *line1 = NULL, *line2 = NULL;
            if (centreVert->visDirections & ConnDirLeft)
            {
                line1 = segments.insert(LineSegment(minLimit, cp.x, e->pos, 
                        true, NULL, centreVert));
            }
            if (centreVert->visDirections & ConnDirRight)
            {
                line2 = segments.insert(LineSegment(cp.x, maxLimit, e->pos, 
                        true, centreVert, NULL));
                // If there was a line1, then we just merged with it, so
                // that pointer will be invalid (and now unnecessary).
                line1 = NULL;
            }
            if (!line1 && !line2)
            {
                // Add a point segment for the centre point.
                segments.insert(LineSegment(cp.x, e->pos, centreVert));
            }
            
            if (!inShape)
            {
                // This is not contained within a shape so add a normal
                // visibility graph point here too (since paths won't route
                // *through* connector endpoint vertices).
                if (line1 || line2)
                {
                    VertInf *cent = new VertInf(router, dummyOrthogID, cp);
                    if (line1)
                    {
                        line1->vertInfs.insert(cent);
                    }
                    if (line2)
                    {
                        line2->vertInfs.insert(cent);
                    }
                }
            }
        }
    }
    
    if ( ((pass == 3) && (e->type == Close)) ||
         ((pass == 2) && (e->type == ConnPoint)) )
    {
        // Clean up neighbour pointers.
        Node *l = v->firstAbove, *r = v->firstBelow;
        if (l != NULL) 
        {
            l->firstBelow = v->firstBelow;
        }
        if (r != NULL)
        {
            r->firstAbove = v->firstAbove;
        }

        if (e->type == ConnPoint)
        {
            scanline.erase(v->iter);
            delete v;
        }
        else  // if (e->type == Close)
        {
            size_t result;
            result = scanline.erase(v);
            COLA_ASSERT(result == 1);
            delete v;
        }
    }
}


// Processes an event for the vertical sweep used for computing the static 
// orthogonal visibility graph.  This adds possible visibility sgments to 
// the segments list.
// The first pass is adding the event to the scanline, the second is for
// processing the event and the third for removing it from the scanline.
static void processEventHori(Router *router, NodeSet& scanline, 
        SegmentListWrapper& segments, Event *e, unsigned int pass)
{
    Node *v = e->v;
    
    if ( ((pass == 1) && (e->type == Open)) ||
         ((pass == 2) && (e->type == ConnPoint)) )
    {
        std::pair<NodeSet::iterator, bool> result = scanline.insert(v);
        v->iter = result.first;
        COLA_ASSERT(result.second);

        NodeSet::iterator it = v->iter;
        // Work out neighbours
        if (it != scanline.begin()) 
        {
            Node *u = *(--it);
            v->firstAbove = u;
            u->firstBelow = v;
        }
        it = v->iter;
        if (++it != scanline.end()) 
        {
            Node *u = *it;
            v->firstBelow = u;
            u->firstAbove = v;
        }
    }
    
    if (pass == 2)
    {
        if ((e->type == Open) || (e->type == Close))
        {
            // Only difference between Open and Close is whether the line
            // segments are at the left or right of the shape.  Decide here.
            double lineX = (e->type == Open) ? v->min[XDIM] : v->max[XDIM];

            // Shape edge positions.
            double minShape = v->min[YDIM];
            double maxShape = v->max[YDIM];
            // As far as we can see.
            double minLimit, maxLimit;
            double minLimitMax, maxLimitMin;
            v->findFirstPointAboveAndBelow(YDIM, lineX, minLimit, maxLimit,
                    minLimitMax, maxLimitMin);

            if (minLimitMax >= maxLimitMin)
            {
                LineSegment *line = segments.insert(
                        LineSegment(minLimit, maxLimit, lineX));

                // Shape corners:
                VertInf *vI1 = new VertInf(router, dummyOrthogShapeID, 
                        Point(lineX, minShape));
                VertInf *vI2 = new VertInf(router, dummyOrthogShapeID, 
                        Point(lineX, maxShape));
                line->vertInfs.insert(vI1);
                line->vertInfs.insert(vI2);
            }
            else
            {
                if ((minLimitMax > minLimit) && (minLimitMax >= minShape))
                {
                    LineSegment *line = segments.insert(
                            LineSegment(minLimit, minLimitMax, lineX));

                    // Shape corner:
                    VertInf *vI1 = new VertInf(router, dummyOrthogShapeID, 
                                Point(lineX, minShape));
                    line->vertInfs.insert(vI1);
                }
                if ((maxLimitMin < maxLimit) && (maxLimitMin <= maxShape))
                {
                    LineSegment *line = segments.insert(
                            LineSegment(maxLimitMin, maxLimit, lineX));

                    // Shape corner:
                    VertInf *vI2 = new VertInf(router, dummyOrthogShapeID, 
                                Point(lineX, maxShape));
                    line->vertInfs.insert(vI2);
                }
            }
        }
        else if (e->type == ConnPoint)
        {
            // Connection point.
            VertInf *centreVert = e->v->c;
            Point& cp = centreVert->point;

            // As far as we can see.
            double minLimit = v->firstPointAbove(YDIM);
            double maxLimit = v->firstPointBelow(YDIM);
            //bool inShape = v->isInsideShape(YDIM);
            
            if (centreVert->visDirections & ConnDirUp)
            {
                segments.insert(LineSegment(minLimit, cp.y, e->pos));
            }
            if (centreVert->visDirections & ConnDirDown)
            {
                segments.insert(LineSegment(cp.y, maxLimit, e->pos));
            }
        }
    }
    
    if ( ((pass == 3) && (e->type == Close)) ||
         ((pass == 2) && (e->type == ConnPoint)) )
    {
        // Clean up neighbour pointers.
        Node *l = v->firstAbove, *r = v->firstBelow;
        if (l != NULL) 
        {
            l->firstBelow = v->firstBelow;
        }
        if (r != NULL)
        {
            r->firstAbove = v->firstAbove;
        }

        if (e->type == ConnPoint)
        {
            scanline.erase(v->iter);
            delete v;
        }
        else  // if (e->type == Close)
        {
            size_t result;
            result = scanline.erase(v);
            COLA_ASSERT(result == 1);
            delete v;
        }
    }
}


extern void generateStaticOrthogonalVisGraph(Router *router)
{
    const size_t n = router->m_obstacles.size();
    const unsigned cpn = router->vertices.connsSize();
    // Set up the events for the vertical sweep.
    size_t totalEvents = (2 * n) + cpn;
    events = new Event*[totalEvents];
    unsigned ctr = 0;
    ObstacleList::iterator obstacleIt = router->m_obstacles.begin();
    for (unsigned i = 0; i < n; i++)
    {
        Obstacle *obstacle = *obstacleIt;
        double minX, minY, maxX, maxY;
        obstacle->polygon().getBoundingRect(&minX, &minY, &maxX, &maxY);
        double midX = minX + ((maxX - minX) / 2);
        Node *v = new Node(obstacle, midX);
        events[ctr++] = new Event(Open, v, minY);
        events[ctr++] = new Event(Close, v, maxY);

        ++obstacleIt;
    }
    for (VertInf *curr = router->vertices.connsBegin(); 
            curr && (curr != router->vertices.shapesBegin()); 
            curr = curr->lstNext)
    {
        if (curr->visDirections == ConnDirNone)
        {
            // This is a connector endpoint that is attached to a connection
            // pin on a shape, so it doesn't need to be given visibility.
            // Thus, skip it and record that there is one less total event.
            --totalEvents;
            continue;
        }
        Point& point = curr->point;

        Node *v = new Node(curr, point.x);
        events[ctr++] = new Event(ConnPoint, v, point.y);
    }
    qsort((Event*)events, (size_t) totalEvents, sizeof(Event*), compare_events);

    // Process the vertical sweep.
    // We do multiple passes over sections of the list so we can add relevant
    // entries to the scanline that might follow, before process them.
    SegmentListWrapper segments;
    NodeSet scanline;
    double thisPos = (totalEvents > 0) ? events[0]->pos : 0;
    unsigned int posStartIndex = 0;
    unsigned int posFinishIndex = 0;
    for (unsigned i = 0; i <= totalEvents; ++i)
    {
        // If we have finished the current scanline or all events, then we
        // process the events on the current scanline in a couple of passes.
        if ((i == totalEvents) || (events[i]->pos != thisPos))
        {
            posFinishIndex = i;
            for (int pass = 2; pass <= 3; ++pass)
            {
                for (unsigned j = posStartIndex; j < posFinishIndex; ++j)
                {
                    processEventVert(router, scanline, segments, 
                            events[j], pass);
                }
            }

            if (i == totalEvents)
            {
                // We have cleaned up, so we can now break out of loop.
                break;
            }

            thisPos = events[i]->pos;
            posStartIndex = i;
        }

        // Do the first sweep event handling -- building the correct 
        // structure of the scanline.
        const int pass = 1;
        processEventVert(router, scanline, segments, events[i], pass);
    }
    COLA_ASSERT(scanline.size() == 0);
    for (unsigned i = 0; i < totalEvents; ++i)
    {
        delete events[i];
    }

    segments.list().sort();

    // Set up the events for the horizontal sweep.
    SegmentListWrapper vertSegments;
    ctr = 0;
    obstacleIt = router->m_obstacles.begin();
    for (unsigned i = 0; i < n; i++)
    {
        Obstacle *obstacle = *obstacleIt;
        double minX, minY, maxX, maxY;
        obstacle->polygon().getBoundingRect(&minX, &minY, &maxX, &maxY);
        double midY = minY + ((maxY - minY) / 2);
        Node *v = new Node(obstacle, midY);
        events[ctr++] = new Event(Open, v, minX);
        events[ctr++] = new Event(Close, v, maxX);

        ++obstacleIt;
    }
    for (VertInf *curr = router->vertices.connsBegin(); 
            curr && (curr != router->vertices.shapesBegin()); 
            curr = curr->lstNext)
    {
        if (curr->visDirections == ConnDirNone)
        {
            // This is a connector endpoint that is attached to a connection
            // pin on a shape, so it doesn't need to be given visibility.
            // Thus, skip it.
            continue;
        }
        Point& point = curr->point;

        Node *v = new Node(curr, point.y);
        events[ctr++] = new Event(ConnPoint, v, point.x);
    }
    qsort((Event*)events, (size_t) totalEvents, sizeof(Event*), compare_events);

    // Process the horizontal sweep
    thisPos = (totalEvents > 0) ? events[0]->pos : 0;
    posStartIndex = 0;
    posFinishIndex = 0;
    for (unsigned i = 0; i <= totalEvents; ++i)
    {
        // If we have finished the current scanline or all events, then we
        // process the events on the current scanline in a couple of passes.
        if ((i == totalEvents) || (events[i]->pos != thisPos))
        {
            posFinishIndex = i;
            for (int pass = 2; pass <= 3; ++pass)
            {
                for (unsigned j = posStartIndex; j < posFinishIndex; ++j)
                {
                    processEventHori(router, scanline, vertSegments, 
                            events[j], pass);
                }
            }
            
            // Process the merged line segments.
            vertSegments.list().sort();
            for (SegmentList::iterator curr = vertSegments.list().begin();
                    curr != vertSegments.list().end(); ++curr)
            {
                intersectSegments(router, segments.list(), *curr);
            }
            vertSegments.list().clear();

            if (i == totalEvents)
            {
                // We have cleaned up, so we can now break out of loop.
                break;
            }

            thisPos = events[i]->pos;
            posStartIndex = i;
        }

        // Do the first sweep event handling -- building the correct 
        // structure of the scanline.
        const int pass = 1;
        processEventHori(router, scanline, vertSegments, events[i], pass);
    }
    COLA_ASSERT(scanline.size() == 0);
    for (unsigned i = 0; i < totalEvents; ++i)
    {
        delete events[i];
    }
    delete [] events;

    // Add portions of the horizontal line that are after the final vertical
    // position we considered.
    for (SegmentList::iterator it = segments.list().begin(); 
            it != segments.list().end(); )
    {
        LineSegment& horiLine = *it;

        horiLine.addEdgeHorizontal(router);
        
        size_t dim = XDIM; // x-dimension
        horiLine.generateVisibilityEdgesFromBreakpointSet(router, dim);

        it = segments.list().erase(it);
    }
}


//============================================================================
//                           Path Adjustment code
//============================================================================




// Processes sweep events used to determine each horizontal and vertical 
// line segment in a connector's channel of visibility.  
// Four calls to this function are made at each position by the scanline:
//   1) Handle all Close event processing.
//   2) Remove Close event objects from the scanline.
//   3) Add Open event objects to the scanline.
//   4) Handle all Open event processing.
//
static void processShiftEvent(Router *router, NodeSet& scanline, 
        ShiftSegmentList& segments, Event *e, size_t dim,
        unsigned int pass)
{
    Node *v = e->v;
    
    if ( ((pass == 3) && (e->type == Open)) ||
         ((pass == 3) && (e->type == SegOpen)) )
    {
        std::pair<NodeSet::iterator, bool> result = scanline.insert(v);
        v->iter = result.first;
        COLA_ASSERT(result.second);

        NodeSet::iterator it = v->iter;
        // Work out neighbours
        if (it != scanline.begin()) 
        {
            Node *u = *(--it);
            v->firstAbove = u;
            u->firstBelow = v;
        }
        it = v->iter;
        if (++it != scanline.end()) 
        {
            Node *u = *it;
            v->firstBelow = u;
            u->firstAbove = v;
        }
    }
    
    if ( ((pass == 4) && (e->type == Open)) ||
         ((pass == 4) && (e->type == SegOpen)) ||
         ((pass == 1) && (e->type == SegClose)) ||
         ((pass == 1) && (e->type == Close)) )
    {
        if (v->ss)
        {
            // As far as we can see.
            double minLimit = v->firstObstacleAbove(dim);
            double maxLimit = v->firstObstacleBelow(dim);

            v->ss->minSpaceLimit = 
                    std::max(minLimit, v->ss->minSpaceLimit);
            v->ss->maxSpaceLimit = 
                    std::min(maxLimit, v->ss->maxSpaceLimit);
        }
        else
        {
            v->markShiftSegmentsAbove(dim);
            v->markShiftSegmentsBelow(dim);
        }
    }
    
    if ( ((pass == 2) && (e->type == SegClose)) ||
         ((pass == 2) && (e->type == Close)) )
    {
        // Clean up neighbour pointers.
        Node *l = v->firstAbove, *r = v->firstBelow;
        if (l != NULL) 
        {
            l->firstBelow = v->firstBelow;
        }
        if (r != NULL)
        {
            r->firstAbove = v->firstAbove;
        }

        size_t result;
        result = scanline.erase(v);
        COLA_ASSERT(result == 1);
        delete v;
    }
}


static void buildOrthogonalChannelInfo(Router *router, 
        const size_t dim, ShiftSegmentList& segmentList)
{
    if (router->routingPenalty(segmentPenalty) == 0)
    {
        // This code assumes the routes are pretty optimal, so we don't
        // do this adjustment if the routes have no segment penalty.
        return;
    }

    size_t altDim = (dim + 1) % 2;
    // For each connector.
    for (ConnRefList::const_iterator curr = router->connRefs.begin(); 
            curr != router->connRefs.end(); ++curr) 
    {
        if ((*curr)->routingType() != ConnType_Orthogonal)
        {
            continue;
        }
        Polygon& displayRoute = (*curr)->displayRoute();
        std::vector<Point> checkpoints = (*curr)->routingCheckpoints();
        // Determine all line segments that we are interested in shifting. 
        // We don't consider the first or last segment of a path.
        for (size_t i = 1; i < displayRoute.size(); ++i)
        {
            if (displayRoute.ps[i - 1][dim] == displayRoute.ps[i][dim])
            {
                // It's a segment in the dimension we are processing,
                size_t indexLow = i - 1;
                size_t indexHigh = i;
                if (displayRoute.ps[i - 1][altDim] == 
                        displayRoute.ps[i][altDim])
                {
                    // This is a zero length segment, so ignore it.
                    continue;
                }
                else if (displayRoute.ps[i - 1][altDim] > 
                        displayRoute.ps[i][altDim])
                {
                    indexLow = i;
                    indexHigh = i - 1;
                }

                bool containsCheckpoint = false;
                for (size_t cpi = 0; cpi < checkpoints.size(); ++cpi)
                {
                    if ( (displayRoute.ps[i] == checkpoints[cpi]) ||
                         (displayRoute.ps[i - 1] == checkpoints[cpi]) ||
                         pointOnLine(displayRoute.ps[i - 1], \
                             displayRoute.ps[i], checkpoints[cpi]) )
                    {
                        containsCheckpoint = true;
                        break;
                    }
                }
                if (containsCheckpoint)
                {
                    // This segment includes one of the routing
                    // checkpoints so we shouldn's shift it.
                    segmentList.push_back(
                            ShiftSegment(*curr, indexLow, indexHigh, dim));
                    continue;
                }

                if ((i == 1) || ((i + 1) == displayRoute.size()))
                {
                    // The first and last segment of a connector can't be 
                    // shifted.  We call them fixed segments.  Note: this
                    // will change if we later allow connection channels.
                    segmentList.push_back(
                            ShiftSegment(*curr, indexLow, indexHigh, dim));
                    continue;
                }

                // The segment probably has space to be shifted.
                double thisPos = displayRoute.ps[i][dim];
                double minLim = -CHANNEL_MAX;
                double maxLim = CHANNEL_MAX;

                bool isSBend = false;

                double prevPos = displayRoute.ps[i - 2][dim];
                double nextPos = displayRoute.ps[i + 1][dim];
                if ( ((prevPos < thisPos) && (nextPos > thisPos)) ||
                     ((prevPos > thisPos) && (nextPos < thisPos)) )
                {
                    isSBend = true;

                    // Determine limits if the s-bend is not due to an 
                    // obstacle.  In this case we need to limit the channel 
                    // to the span of the adjoining segments to this one.
                    if ((prevPos < thisPos) && (nextPos > thisPos))
                    {
                        minLim = std::max(minLim, prevPos);
                        maxLim = std::min(maxLim, nextPos);
                    }
                    else
                    {
                        minLim = std::max(minLim, nextPos);
                        maxLim = std::min(maxLim, prevPos);
                    }
                }
                else
                {
                    // isCBend: Both adjoining segments are in the same
                    // direction.  We indicate this for later by setting 
                    // the maxLim or minLim to the segment position.
                    if (prevPos < thisPos)
                    {
                        minLim = thisPos;
                    }
                    else
                    {
                        maxLim = thisPos;
                    }
                }

                segmentList.push_back(ShiftSegment(*curr, indexLow, 
                            indexHigh, isSBend, dim, minLim, maxLim));
            }
        }
    }
    if (segmentList.empty())
    {
        // There are no segments, so we can just return now.
        return;
    }
    
    // Do a sweep and shift these segments.
    const size_t n = router->m_obstacles.size();
    const size_t cpn = segmentList.size();
    // Set up the events for the sweep.
    size_t totalEvents = 2 * (n + cpn);
    events = new Event*[totalEvents];
    unsigned ctr = 0;
    ObstacleList::iterator obstacleIt = router->m_obstacles.begin();
    for (unsigned i = 0; i < n; i++)
    {
        Obstacle *obstacle = *obstacleIt;
        Point min, max;
        obstacle->polygon().getBoundingRect(&min.x, &min.y, &max.x, &max.y);
        double mid = min[dim] + ((max[dim] - min[dim]) / 2);
        Node *v = new Node(obstacle, mid);
        events[ctr++] = new Event(Open, v, min[altDim]);
        events[ctr++] = new Event(Close, v, max[altDim]);

        ++obstacleIt;
    }
    for (ShiftSegmentList::iterator curr = segmentList.begin(); 
            curr != segmentList.end(); ++curr)
    {
        const Point& lowPt = curr->lowPoint();
        const Point& highPt = curr->highPoint();

        COLA_ASSERT(lowPt[dim] == highPt[dim]);
        COLA_ASSERT(lowPt[altDim] < highPt[altDim]);
        Node *v = new Node(&(*curr), lowPt[dim]);
        events[ctr++] = new Event(SegOpen, v, lowPt[altDim]);
        events[ctr++] = new Event(SegClose, v, highPt[altDim]);
    }
    qsort((Event*)events, (size_t) totalEvents, sizeof(Event*), compare_events);

    // Process the sweep.
    // We do multiple passes over sections of the list so we can add relevant
    // entries to the scanline that might follow, before process them.
    NodeSet scanline;
    double thisPos = (totalEvents > 0) ? events[0]->pos : 0;
    unsigned int posStartIndex = 0;
    unsigned int posFinishIndex = 0;
    for (unsigned i = 0; i <= totalEvents; ++i)
    {
        // If we have finished the current scanline or all events, then we
        // process the events on the current scanline in a couple of passes.
        if ((i == totalEvents) || (events[i]->pos != thisPos))
        {
            posFinishIndex = i;
            for (int pass = 2; pass <= 4; ++pass)
            {
                for (unsigned j = posStartIndex; j < posFinishIndex; ++j)
                {
                    processShiftEvent(router, scanline, segmentList, events[j], 
                            dim, pass);
                }
            }

            if (i == totalEvents)
            {
                // We have cleaned up, so we can now break out of loop.
                break;
            }

            thisPos = events[i]->pos;
            posStartIndex = i;
        }

        // Do the first sweep event handling -- building the correct 
        // structure of the scanline.
        const int pass = 1;
        processShiftEvent(router, scanline, segmentList, events[i],
                dim, pass);
    }
    COLA_ASSERT(scanline.size() == 0);
    for (unsigned i = 0; i < totalEvents; ++i)
    {
        delete events[i];
    }
    delete [] events;
}


static void simplifyOrthogonalRoutes(Router *router)
{
    // Simplify routes.
    for (ConnRefList::const_iterator curr = router->connRefs.begin(); 
            curr != router->connRefs.end(); ++curr) 
    {
        if ((*curr)->routingType() != ConnType_Orthogonal)
        {
            continue;
        }
        (*curr)->set_route((*curr)->displayRoute().simplify());
    }
}


static void buildOrthogonalNudgingOrderInfo(Router *router, 
        PtOrderMap& pointOrders)
{
    // Simplify routes.
    simplifyOrthogonalRoutes(router);

    int crossingsN = 0;

    // Do segment splitting.
    for (ConnRefList::const_iterator curr = router->connRefs.begin(); 
            curr != router->connRefs.end(); ++curr) 
    {
        if ((*curr)->routingType() != ConnType_Orthogonal)
        {
            continue;
        }
        ConnRef *conn = *curr;
        
        for (ConnRefList::const_iterator curr2 = router->connRefs.begin(); 
                curr2 != router->connRefs.end(); ++curr2) 
        {
            if ((*curr2)->routingType() != ConnType_Orthogonal)
            {
                continue;
            }
            ConnRef *conn2 = *curr2;
            
            if (conn == conn2)
            {
                continue;
            }
            
            Avoid::Polygon& route = conn->displayRoute();
            Avoid::Polygon& route2 = conn2->displayRoute();
            splitBranchingSegments(route2, true, route);
        }
    }

    for (ConnRefList::const_iterator curr = router->connRefs.begin(); 
            curr != router->connRefs.end(); ++curr) 
    {
        if ((*curr)->routingType() != ConnType_Orthogonal)
        {
            continue;
        }
        ConnRef *conn = *curr;
        
        for (ConnRefList::const_iterator curr2 = curr; 
                curr2 != router->connRefs.end(); ++curr2) 
        {
            if ((*curr2)->routingType() != ConnType_Orthogonal)
            {
                continue;
            }
            ConnRef *conn2 = *curr2;

            if (conn == conn2)
            {
                continue;
            }
            
            Avoid::Polygon& route = conn->displayRoute();
            Avoid::Polygon& route2 = conn2->displayRoute();
            int crossings = 0;
            ConnectorCrossings cross(route2, true, route, conn2, conn);
            cross.pointOrders = &pointOrders;
            for (size_t i = 1; i < route.size(); ++i)
            {
                const bool finalSegment = ((i + 1) == route.size());
                cross.countForSegment(i, finalSegment);

                crossings += cross.crossingCount;
            }
            if (crossings > 0)
            {
                crossingsN += crossings;
            }
        }
    }
    
    // Sort the point orders.
    PtOrderMap::iterator finish = pointOrders.end();
    for (PtOrderMap::iterator it = pointOrders.begin(); it != finish; ++it)
    {
        //const VertID& ptID = it->first;
        PtOrder& order = it->second;

        for (size_t dim = XDIM; dim <= YDIM; ++dim)
        {
            order.sort(dim);
        }
    }
}


class CmpLineOrder 
{
    public:
        CmpLineOrder(PtOrderMap& ord, const size_t dim)
            : orders(ord),
              dimension(dim)
        {
        }
        bool operator()(const ShiftSegment& lhs, const ShiftSegment& rhs,
                bool *comparable = NULL) const
        {
            if (comparable)
            {
                *comparable = true;
            }
            Point lhsLow  = lhs.lowPoint(); 
            Point rhsLow  = rhs.lowPoint(); 
#ifndef NDEBUG
            const Point& lhsHigh = lhs.highPoint(); 
            const Point& rhsHigh = rhs.highPoint(); 
#endif
            size_t altDim = (dimension + 1) % 2;

            COLA_ASSERT(lhsLow[dimension] == lhsHigh[dimension]);
            COLA_ASSERT(rhsLow[dimension] == rhsHigh[dimension]);

            // We consider things at effectively the same position to 
            // be ordered based on their order and fixedOrder, so only 
            // compare segments further apart than the nudgeDistance.
            if (fabs(lhsLow[dimension] - rhsLow[dimension]) > 
                    lhs.nudgeDistance())
            {
                return lhsLow[dimension] < rhsLow[dimension];
            }
            
            // If one of these is fixed, then determine order based on 
            // fixed segment, that is, order so the fixed segment doesn't 
            // block movement.
            bool oneIsFixed = false;
            const int lhsFixedOrder = lhs.fixedOrder(oneIsFixed);
            const int rhsFixedOrder = rhs.fixedOrder(oneIsFixed);
            if (oneIsFixed && (lhsFixedOrder != rhsFixedOrder))
            {
                return lhsFixedOrder < rhsFixedOrder;
            }

            // C-bends that did not have a clear order with s-bends might 
            // not have a good ordering here, so compare their order in 
            // terms of C-bend direction and S-bends and use that if it
            // differs for the two segments.
            const int lhsOrder = lhs.order();
            const int rhsOrder = rhs.order();
            if (lhsOrder != rhsOrder)
            {
                return lhsOrder < rhsOrder;
            }

            // Need to index using the original point into the map, so find it.
            Point& unchanged = (lhsLow[altDim] > rhsLow[altDim]) ?
                    lhsLow : rhsLow;

            PtOrder& lowOrder = orders[unchanged];
            int lhsPos = lowOrder.positionFor(lhs.connRef, dimension);
            int rhsPos = lowOrder.positionFor(rhs.connRef, dimension);
            if ((lhsPos == -1) || (rhsPos == -1))
            {
                // A value for rhsPos or lhsPos mean the points are not directly
                // comparable, meaning they are at the same position but cannot
                // overlap (they are just collinear.  The relative order for 
                // these segments is not important since we do not constrain
                // them against each other.
                //COLA_ASSERT(lhs.overlapsWith(rhs, dimension) == false);
                // We do need to be consistent though.
                if (comparable)
                {
                    *comparable = false;
                }
                return lhsLow[altDim] < rhsLow[altDim];
            }
            return lhsPos < rhsPos;
        }

        PtOrderMap& orders;
        const size_t dimension;
};


// We can't use the normal sort algorithm for lists since it is not possible 
// to compare all elements, but there will be an ordering defined between 
// most of the elements.  Hence we order these, using insertion sort, and 
// the case of them not being able to be compared is handled by not setting 
// up any constraints between such segments when doing the nudging.
//
static ShiftSegmentList linesort(ShiftSegmentList origList, 
        CmpLineOrder& comparison)
{
    ShiftSegmentList resultList;

    size_t origListSize = origList.size();
    size_t deferredN = 0; 
    while (!origList.empty())
    {
        // Get and remove the first element from the origList.
        ShiftSegment segment = origList.front();
        origList.pop_front();

        // Find the insertion point in the resultList.
        bool allComparable = true;
        ShiftSegmentList::iterator curr;
        for (curr = resultList.begin(); curr != resultList.end(); ++curr)
        {
            bool comparable = false;
            bool lessThan = comparison(segment, *curr, &comparable);

            allComparable &= comparable;

            if (comparable && lessThan)
            {
                // If it is comparable and lessThan, then we have found the
                // insertion point.
                break;
            }
        }
    
        if (resultList.empty() || allComparable || (deferredN >= origListSize))
        {
            // Insert the element into the resultList at the required point.
            resultList.insert(curr, segment);
            // Reset the origListSize and deferred counter.
            deferredN = 0;
            origListSize = origList.size();
        }
        else
        {
            // This wasn't comparable to anything in the sorted list, 
            // so defer addition of the segment till later.
            origList.push_back(segment);
            deferredN++;
        }
    }

    return resultList;
}


typedef std::list<ShiftSegment *> ShiftSegmentPtrList;


static void nudgeOrthogonalRoutes(Router *router, size_t dimension, 
        PtOrderMap& pointOrders, ShiftSegmentList& segmentList)
{
    double baseSepDist = router->orthogonalNudgeDistance();
    COLA_ASSERT(baseSepDist >= 0);
    // If we can fit things with the desired separtation distance, then
    // we try 10 times, reducing eac time by a 10th of the original amount.
    double reductionSteps = 10.0;

    // Do the actual nudging.
    ShiftSegmentList currentRegion;
    while (!segmentList.empty())
    {
        // Take a reference segment
        ShiftSegment& currentSegment = segmentList.front();
        // Then, find the segments that overlap this one.
        currentRegion.clear();
        currentRegion.push_back(currentSegment);
        segmentList.erase(segmentList.begin());
        for (ShiftSegmentList::iterator curr = segmentList.begin();
                curr != segmentList.end(); )
        {
            bool overlaps = false;
            for (ShiftSegmentList::iterator curr2 = currentRegion.begin();
                    curr2 != currentRegion.end(); ++curr2)
            {
                if (curr->overlapsWith(*curr2, dimension))
                {
                    overlaps = true;
                    break;
                }
            }
            if (overlaps)
            {
                currentRegion.push_back(*curr);
                segmentList.erase(curr);
                // Consider segments from the beginning, since we mave have
                // since passed segments that overlap with the new set.
                curr = segmentList.begin();
            }
            else
            {
                ++curr;
            }
        }
        CmpLineOrder lineSortComp(pointOrders, dimension);
        currentRegion = linesort(currentRegion, lineSortComp);
        
        if (currentRegion.size() == 1)
        {
            // Save creating the solver instance if there is just one
            // immovable segment.
            if (!currentRegion.front().sBend)
            {
                continue;
            }
        }

        // Process these segments.
        Variables vs;
        Constraints cs;
        Constraints gapcs;
        ShiftSegmentPtrList prevVars;
        // IDs:
        const int freeID    = 0;
        const int fixedID   = 1;
        // Weights:
        double freeWeight   = 0.00001;
        double strongWeight = 0.001;
        double fixedWeight  = 100000;
        double sepDist = baseSepDist;
#ifdef NUDGE_DEBUG 
        printf("-------------------------------------------------------\n");
        printf("Nudge -- size: %d\n", (int) currentRegion.size());
#endif
        for (ShiftSegmentList::iterator currSegment = currentRegion.begin();
                currSegment != currentRegion.end(); ++currSegment)
        {
            Point& lowPt = currSegment->lowPoint();
            
            // Create a solver variable for the position of this segment.
            int varID = freeID;
            double idealPos = lowPt[dimension];
            double weight = freeWeight;
            if (currSegment->sBend)
            {
                COLA_ASSERT(currSegment->minSpaceLimit > -CHANNEL_MAX);
                COLA_ASSERT(currSegment->maxSpaceLimit < CHANNEL_MAX);
                
                // For s-bends, take the middle as ideal.
                idealPos = currSegment->minSpaceLimit +
                        ((currSegment->maxSpaceLimit -
                          currSegment->minSpaceLimit) / 2);
            }
            else if (currSegment->fixed)
            {
                // Fixed segments shouldn't get moved.
                weight = fixedWeight;
                varID = fixedID;
            }
            else
            {
                // Set a higher weight for c-bends to stop them sometimes 
                // getting pushed out into channels by more-free connectors
                // to the "inner" side of them.
                weight = strongWeight;
            }
            currSegment->variable = new Variable(varID, idealPos, weight);
            vs.push_back(currSegment->variable);
            size_t index = vs.size() - 1;
#ifdef NUDGE_DEBUG 
            printf("line  %.15f  dim: %d pos: %g   min: %g  max: %g\n"
                   "minEndPt: %g  maxEndPt: %g\n",
                    lowPt[dimension], (int) dimension, idealPos, 
                    currSegment->minSpaceLimit, currSegment->maxSpaceLimit,
                    lowPt[!dimension], currSegment->highPoint()[!dimension]);
#endif
#if 0
            // Debugging info:
            double minP = std::max(currSegment->minSpaceLimit, -5000.0);
            double maxP = std::min(currSegment->maxSpaceLimit, 5000.0);
            fprintf(stdout, "<rect style=\"fill: #f00; opacity: 0.2;\" "
                    "x=\"%g\" y=\"%g\" width=\"%g\" height=\"%g\" />\n",
                    lowPt[XDIM], minP, 
                    currSegment->highPoint()[XDIM] - lowPt[XDIM], 
                    maxP - minP);
            fprintf(stdout, "<line style=\"stroke: #000;\" x1=\"%g\" "
                    "y1=\"%g\" x2=\"%g\" y2=\"%g\" />\n",
                    lowPt[XDIM], lowPt[YDIM], currSegment->highPoint()[XDIM], 
                    currSegment->highPoint()[YDIM]);
#endif

            // Constrain position in relation to previously seen segments,
            // if necessary (i.e. when they could overlap).
            for (ShiftSegmentPtrList::iterator prevVarIt = prevVars.begin();
                    prevVarIt != prevVars.end(); )
            {
                ShiftSegment *prevSeg = *prevVarIt;
                Variable *prevVar = prevSeg->variable;

                if (currSegment->overlapsWith(*prevSeg, dimension) &&
                        (!(currSegment->fixed) || !(prevSeg->fixed)))
                {
                    // If there is a previous segment to the left that 
                    // could overlap this in the shift direction, then 
                    // constrain the two segments to be separated.
                    // Though don't add the constraint if both the 
                    // segments are fixed in place.
                    if (currSegment->connRef == prevSeg->connRef)
                    {
                        // We need to address the problem of two neighbouring
                        // segments of the same connector being kept separated
                        // due only to a kink created in the other dimension.
                        // Here, we let such segments drift back together.
                        sepDist = 0;
                    }
                    Constraint *constraint = 
                            new Constraint(prevVar, vs[index], sepDist);
                    cs.push_back(constraint);
                    // Add to the list of gap constraints so we can rewrite
                    // the separation distance later.
                    gapcs.push_back(constraint);
                    prevVarIt = prevVars.erase(prevVarIt);
                }
                else
                {
                    ++prevVarIt;
                }
            }

            if (!currSegment->fixed)
            {
                // If this segment sees a channel boundary to its left, 
                // then constrain its placement as such.
                if (currSegment->minSpaceLimit > -CHANNEL_MAX)
                {
                    vs.push_back(new Variable(fixedID, 
                                currSegment->minSpaceLimit, fixedWeight));
                    cs.push_back(new Constraint(vs[vs.size() - 1], vs[index], 
                                0.0));
                }
                
                // If this segment sees a channel boundary to its right, 
                // then constrain its placement as such.
                if (currSegment->maxSpaceLimit < CHANNEL_MAX)
                {
                    vs.push_back(new Variable(fixedID, 
                                currSegment->maxSpaceLimit, fixedWeight));
                    cs.push_back(new Constraint(vs[index], vs[vs.size() - 1],
                                0.0));
                }
            }
            prevVars.push_back(&(*currSegment));
        }
#ifdef NUDGE_DEBUG
        for (unsigned i = 0;i < vs.size(); ++i)
        {
            printf("-vs[%d]=%f\n", i, vs[i]->desiredPosition);
        }
#endif
        // Repeatedly try solving this with smaller separation distances till
        // we find a solution that is satisfied.
        bool satisfied;
        do 
        {
            IncSolver f(vs,cs);
            f.solve();
            satisfied = true;
            for (size_t i = 0; i < vs.size(); ++i) 
            {
                if (vs[i]->id == fixedID)
                {
                    if (fabs(vs[i]->finalPosition - 
                            vs[i]->desiredPosition) > 0.01)
                    {
                        satisfied = false;
                        break;
                    }
                }
            }

            if (!satisfied)
            {
                // Reduce the separation distance.
                sepDist -= (baseSepDist / reductionSteps);
                // And rewrite all the gap constraints to have the new reduced
                // separation distance.
                for (Constraints::iterator cIt = gapcs.begin(); 
                        cIt != gapcs.end(); ++cIt)
                {
                    Constraint *constraint = *cIt;
                    constraint->gap = sepDist;
                }
            }
        }
        while (!satisfied && (sepDist > 0.0001));

        if (satisfied)
        {
            for (ShiftSegmentList::iterator currSegment = currentRegion.begin();
                    currSegment != currentRegion.end(); ++currSegment)
            {
                if (currSegment->fixed)
                {
                    continue;
                }
                Point& lowPt = currSegment->lowPoint();
                Point& highPt = currSegment->highPoint();
                double newPos = currSegment->variable->finalPosition;
#ifdef NUDGE_DEBUG
                printf("Pos: %lX, %g\n", (long) currSegment->connRef, newPos);
#endif
                lowPt[dimension] = newPos;
                highPt[dimension] = newPos;
            }
        }
#ifdef NUDGE_DEBUG
        for(unsigned i=0;i<vs.size();i++) {
            printf("+vs[%d]=%f\n",i,vs[i]->finalPosition);
        }
#endif
        for_each(vs.begin(), vs.end(), delete_object());
        for_each(cs.begin(), cs.end(), delete_object());
    }
}


extern void improveOrthogonalRoutes(Router *router)
{
    router->timers.Register(tmOrthogNudge, timerStart);
    for (size_t dimension = 0; dimension < 2; ++dimension)
    {
        // Build nudging info.
        // XXX: We need to build the point orders separately in each
        //      dimension since things move.  There is probably a more 
        //      efficient way to do this.
        PtOrderMap pointOrders;
        buildOrthogonalNudgingOrderInfo(router, pointOrders);

        // Simplify routes.
        simplifyOrthogonalRoutes(router);

        // Do the centring and nudging.
        ShiftSegmentList segLists;
        buildOrthogonalChannelInfo(router, dimension, segLists);
        nudgeOrthogonalRoutes(router, dimension, pointOrders, segLists);
    }
    router->timers.Stop();
}


}
