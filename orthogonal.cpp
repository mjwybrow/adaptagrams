/*
 * vim: ts=4 sw=4 et tw=0 wm=0
 *
 * libavoid - Fast, Incremental, Object-avoiding Line Router
 *
 * Copyright (C) 2009  Monash University
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
#include "libavoid/connector.h"
#include "libavoid/vpsc.h"

#ifdef LIBAVOID_SDL
  #include <SDL_gfxPrimitives.h>
#endif


namespace Avoid {


static const double CHANNEL_MAX = 10000;

class ShiftSegment 
{
    public:
        ShiftSegment(ConnRef *conn, const size_t low, const size_t high, 
                bool isSBend, const size_t dim)
            : connRef(conn),
              indexLow(low),
              indexHigh(high),
              sBend(isSBend),
              dimension(dim),
              variable(NULL),
              minSpaceLimit(-CHANNEL_MAX),
              maxSpaceLimit(CHANNEL_MAX)
        {
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
        bool overlapsWith(const ShiftSegment& rhs, const size_t dim) const
        {
            size_t altDim = (dim + 1) % 2;
            const Point& lowPt = lowPoint();
            const Point& highPt = highPoint();
            const Point& rhsLowPt = rhs.lowPoint();
            const Point& rhsHighPt = rhs.highPoint();
            if ( (lowPt[altDim] < rhsHighPt[altDim]) &&
                    (rhsLowPt[altDim] < highPt[altDim]))
            {
                if ( (minSpaceLimit < rhs.maxSpaceLimit) &&
                        (rhs.minSpaceLimit < maxSpaceLimit))
                {
                    return true;
                }
            }
            return false;
        }

        ConnRef *connRef;
        const size_t indexLow;
        const size_t indexHigh;
        const bool sBend;
        const size_t dimension;
        Variable *variable;
        double minSpaceLimit;
        double maxSpaceLimit;
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
    ShapeRef *v;
    VertInf *c;
    ShiftSegment *ss;
    double pos;
    double min[2], max[2];
    Node *firstAbove, *firstBelow;
    NodeSet::iterator iter;

    Node(ShapeRef *v, const double p)
        : v(v),
          c(NULL),
          ss(NULL),
          pos(p),
          firstAbove(NULL),
          firstBelow(NULL)
    {   
        //assert(r->width()<1e40);
        v->polygon().getBoundingRect(&min[0], &min[1], &max[0], &max[1]);
    }   
    Node(VertInf *c, const double p)
        : v(NULL),
          c(c),
          ss(NULL),
          pos(p),
          firstAbove(NULL),
          firstBelow(NULL)
    {
        min[0] = max[0] = c->point.x;
        min[1] = max[1] = c->point.y;
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
        min[0] = max[0] = min[1] = max[1] = 0;
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
    double firstPointAbove(size_t dim)
    {
        Node *curr = firstAbove;
        while (curr && (curr->max[dim] >= pos))
        {
            curr = curr->firstAbove;
        }
       
        if (curr)
        {
            return curr->max[dim];
        }
        return -DBL_MAX;
    }
    double firstPointBelow(size_t dim)
    {
        Node *curr = firstBelow;
        while (curr && (curr->min[dim] <= pos))
        {
            curr = curr->firstBelow;
        }
        
        if (curr)
        {
            return curr->min[dim];
        }
        return DBL_MAX;
    }
    // This is a bit inefficient, but we won't need to do it once we have 
    // connection points.
    bool isInsideShapeX(void)
    {
        for (Node *curr = firstBelow; curr; curr = curr->firstBelow)
        {
            if ((curr->min[0] < pos) && (pos < curr->max[0]))
            {
                return true;
            }
        }
        for (Node *curr = firstAbove; curr; curr = curr->firstAbove)
        {
            if ((curr->min[0] < pos) && (pos < curr->max[0]))
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
    assert(ea->v != eb->v);
    return ea->v - eb->v;
}


struct PosVertInf
{
    PosVertInf(double p, VertInf *vI = NULL)
        : pos(p),
          vert(vI)
    {
    }
    
    bool operator<(const PosVertInf& rhs) const 
    {
        if (pos != rhs.pos)
        {
            return pos < rhs.pos;
        }
        return vert < rhs.vert;
    }

    double pos;
    VertInf *vert;
};


struct CmpVertInf { 
        bool operator()(const VertInf* u, const VertInf* v) const
        {
            // Comparator for VertSet, an ordered set of VertInf pointers.
            // It is assumed vertical sets of points will all have the same
            // x position and horizontal sets all share a y position, so this
            // method can be used to sort both these sets.
            assert((u->point.x == v->point.x) || (u->point.y == v->point.y));
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
        //assert( begin <= finish);
        if (begin > finish)
        {
            // XXX: This shouldn't be necessary.  
            //      Is it due to a bug in the sweep code?
            double temp = begin;
            begin = finish;
            finish = temp;

            VertInf *tempVI = bvi;
            bvi = fvi;
            fvi = tempVI;
        }

        if (bvi)
        {
            vertInfs.insert(bvi);
        }
        if (fvi)
        {
            vertInfs.insert(fvi);
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
        assert(shapeSide == rhs.shapeSide);
        return false;
    }

    bool operator==(const LineSegment& rhs) const
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
        return *vertInfs.begin();
    }
    VertInf *finishVertInf(void) const
    {
        if (vertInfs.empty())
        {
            return NULL;
        }
        return *vertInfs.rbegin();
    }

    VertInf * commitPositionX(Router *router, double posX)
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
    void commitBegin(Router *router, VertInf *vert = NULL)
    {
        if (vert)
        {
            vertInfs.insert(vert);
        }

        if (vertInfs.empty() ||
                ((*vertInfs.begin())->point.x != begin))
        {
            vertInfs.insert(new
                    VertInf(router, dummyOrthogID, Point(begin, pos)));
        }
    }

    // Set begin endpoint vertex if none has been assigned.
    void commitFinish(Router *router, VertInf *vert = NULL)
    {
        if (vert)
        {
            vertInfs.insert(vert);
        }

        if (vertInfs.empty() ||
                ((*vertInfs.rbegin())->point.x != finish))
        {
            vertInfs.insert(new
                    VertInf(router, dummyOrthogID, Point(finish, pos)));
        }
    }

    // Converts points list to visibility ends.  Returns the last point 
    // considered.
    VertSet::iterator addSegmentsUpTo(Router *router, double finishPos)
    {
        const bool orthogonal = true;
        VertSet::iterator vert, last;
        for (vert = last = vertInfs.begin(); vert != vertInfs.end();)
        {
            if ((*vert)->point.x > finishPos)
            {
                // We're done.
                break;
            }
            
            VertSet::iterator firstPrev = last;
            while ((*last)->point.x != (*vert)->point.x)
            {
                assert(vert != last);
                // Assert points are not at the same position.
                assert((*vert)->point.x != (*last)->point.x);
               
                if ( !((*vert)->id.isShape || (*last)->id.isShape))
                {
                    // Here we have a pair of two endpoints that are both
                    // connector endpoints and both are inside a shape.
                    
                    // Give vert visibility back to the the first 
                    // non-connector-endpoint vertex (the side of the shape).
                    VertSet::iterator side = last;
                    while (!(*side)->id.isShape)
                    {
                        if (side == vertInfs.begin())
                        {
                            break;
                        }
                        --side;
                    }
                    if ((*side)->id.isShape)
                    {
                        EdgeInf *edge = new EdgeInf(*side, *vert, orthogonal);
                        edge->setDist((*vert)->point.x - (*side)->point.x);
                    }

                    // Give last visibility back to the the first 
                    // non-connector-endpoint vertex (the side of the shape).
                    side = vert;
                    while ((side != vertInfs.end()) && !(*side)->id.isShape)
                    {
                        ++side;
                    }
                    if (side != vertInfs.end())
                    {
                        EdgeInf *edge = new EdgeInf(*last, *side, orthogonal);
                        edge->setDist((*side)->point.x - (*last)->point.x);
                    }

                    EdgeInf *edge = new EdgeInf(*last, *vert, orthogonal);
                    edge->setDist((*vert)->point.x - (*last)->point.x);
                }
                
                // The normal case.
                //
                // Note: It's okay to give two connector endpoints visbility 
                // here since we only consider the partner endpoint as a 
                // candidate while searching.
                EdgeInf *edge = new EdgeInf(*last, *vert, orthogonal);
                edge->setDist((*vert)->point.x - (*last)->point.x);
                
                ++last;
            }

            ++vert;

            if ((vert != vertInfs.end()) &&
                    ((*last)->point.x == (*vert)->point.x))
            {
                // Still looking at same pair, just reset prev number pointer.
                last = firstPrev;
            }
            else
            {
                // vert has moved to the beginning of a number number group.
                // Last is now in the right place, so do nothing.
            }
        }
        // Returns the first of the last second set of vertices.
        return last;
    }

    // Add visibility edge(s) for this segment.  There may be multiple if 
    // one of the endpoints is shared by multiple connector endpoints.
    void addEdgeHorizontal(Router *router)
    {
        commitBegin(router);
        commitFinish(router);
        
        addSegmentsUpTo(router, finish);
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

        commitBegin(router);

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
    void insertBreakpointsBegin(Router *router, LineSegment& vertLine,
            std::set<PosVertInf>& breakPoints)
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
        commitBegin(router, vert);

        for (VertSet::iterator v = vertInfs.begin();
                v != vertInfs.end(); ++v)
        {
            if ((*v)->point.x == begin)
            {
                breakPoints.insert(PosVertInf(pos, *v));
            }
        }
    }

    // Insert vertical breakpoints.
    void insertBreakpointsFinish(Router *router, LineSegment& vertLine,
            std::set<PosVertInf>& breakPoints)
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
        commitFinish(router, vert);

        for (VertSet::iterator v = vertInfs.begin();
                v != vertInfs.end(); ++v)
        {
            if ((*v)->point.x == finish)
            {
                breakPoints.insert(PosVertInf(pos, *v));
            }
        }
    }

    double begin;
    double finish;
    double pos;
    bool shapeSide;
    
    VertSet vertInfs;
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
            for (SegmentList::iterator curr = _list.begin();
                    curr != _list.end(); ++curr)
            {
                if (*curr == segment)
                {
                    // Merge this with existing line.
                    curr->mergeVertInfs(segment);
                    // So don't need to add this line.
                    return &(*curr);
                }
            }
            // Add this line.
            _list.push_back(segment);

            return &(_list.back());
        }
        SegmentList& list(void)
        {
            return _list;
        }
    private:
        SegmentList _list;
};


// Given a router instance and a set of possible horizontal segments, and a
// possible vertical visibility segments, compute and add edges to the
// orthogonal visibility graph for all the visibility edges.
static void intersectSegments(Router *router, SegmentList& segments, 
        LineSegment& vertLine)
{
    std::set<PosVertInf> breakPoints;
    assert(vertLine.beginVertInf() == NULL);
    assert(vertLine.finishVertInf() == NULL);
    for (SegmentList::iterator it = segments.begin(); it != segments.end(); )
    {
        LineSegment& horiLine = *it;

        bool inVertSegRegion = ((vertLine.begin <= horiLine.pos) &&
                                (vertLine.finish >= horiLine.pos));

        if (horiLine.finish < vertLine.pos)
        {
            // Add horizontal visibility segment.
            horiLine.addEdgeHorizontal(router);

            // We've now swept past this horizontal segment, so delete.
            it = segments.erase(it);
            continue;
        }
        else if (horiLine.begin > vertLine.pos)
        {
            // We've yet to reach this segment in the sweep, so ignore.
            ++it;
            continue;
        }
        else if (horiLine.begin == vertLine.pos)
        {
            if (inVertSegRegion)
            {
                horiLine.insertBreakpointsBegin(router, vertLine, breakPoints);
            }
        }
        else if (horiLine.finish == vertLine.pos)
        {
            if (inVertSegRegion)
            {
                // Add horizontal visibility segment.
                horiLine.addEdgeHorizontal(router);
            
                horiLine.insertBreakpointsFinish(router, vertLine, breakPoints);
                
                // And we've now finished with the segment, so delete.
                it = segments.erase(it);
                continue;
            }
        }
        else
        {
            assert(horiLine.begin < vertLine.pos);
            assert(horiLine.finish > vertLine.pos);

            if (inVertSegRegion)
            {
                // Add horizontal visibility segment.
                VertSet intersectionVerts = 
                        horiLine.addEdgeHorizontalTillIntersection(
                            router, vertLine);

                for (VertSet::iterator v = intersectionVerts.begin();
                        v != intersectionVerts.end(); ++v)
                {
                    breakPoints.insert(PosVertInf(horiLine.pos, *v));
                }
            }
        }
        ++it;
    }
    if ((breakPoints.begin())->pos != vertLine.begin)
    {
        if (!vertLine.beginVertInf())
        {
            // Add begin point if it didn't intersect another line.
            VertInf *vert = new VertInf(router, dummyOrthogID, 
                    Point(vertLine.pos, vertLine.begin));
            breakPoints.insert(PosVertInf(vertLine.begin, vert));
        }
    }
    if ((breakPoints.rbegin())->pos != vertLine.finish)
    {
        if (!vertLine.finishVertInf())
        {
            // Add finish point if it didn't intersect another line.
            VertInf *vert = new VertInf(router, dummyOrthogID, 
                    Point(vertLine.pos, vertLine.finish));
            breakPoints.insert(PosVertInf(vertLine.finish, vert));
        }
    }

    // Split breakPoints set into visibility segments.
    // XXX: Perhaps make addSegmentsUpTo generic and use that code.
    const bool orthogonal = true;
    std::set<PosVertInf>::iterator vert, last;
    for (vert = last = breakPoints.begin(); vert != breakPoints.end();)
    {
        std::set<PosVertInf>::iterator firstPrev = last;
        while (last->vert->point.y != vert->vert->point.y)
        {
            assert(vert != last);
            // Assert points are not at the same position.
            assert(vert->vert->point.y != last->vert->point.y);

            if ( !(vert->vert->id.isShape || last->vert->id.isShape))
            {
                // Here we have a pair of two endpoints that are both
                // connector endpoints and both are inside a shape.
                
                // Give vert visibility back to the the first 
                // non-connector-endpoint vertex (the side of the shape).
                std::set<PosVertInf>::iterator side = last;
                while (!side->vert->id.isShape)
                {
                    if (side == breakPoints.begin())
                    {
                        break;
                    }
                    --side;
                }
                if (side->vert->id.isShape)
                {
                    EdgeInf *edge = 
                            new EdgeInf(side->vert, vert->vert, orthogonal);
                    edge->setDist(vert->vert->point.y - side->vert->point.y);
                }

                // Give last visibility back to the the first 
                // non-connector-endpoint vertex (the side of the shape).
                side = vert;
                while ((side != breakPoints.end()) && !side->vert->id.isShape)
                {
                    ++side;
                }
                if (side != breakPoints.end())
                {
                    EdgeInf *edge = 
                            new EdgeInf(last->vert, side->vert, orthogonal);
                    edge->setDist(side->vert->point.y - last->vert->point.y);
                }
            }
            
            // The normal case.
            //
            // Note: It's okay to give two connector endpoints visbility 
            // here since we only consider the partner endpoint as a 
            // candidate while searching.
            EdgeInf *edge = new EdgeInf(last->vert, vert->vert, orthogonal);
            edge->setDist(vert->vert->point.y - last->vert->point.y);

            ++last;
        }

        ++vert;

        if ((vert != breakPoints.end()) &&
                (last->vert->point.y == vert->vert->point.y))
        {
            // Still looking at same pair, just reset prev number pointer.
            last = firstPrev;
        }
        else
        {
            // vert has moved to the beginning of a number number group.
            // Last is now in the right place, so do nothing.
        }
    }
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
        assert(result.second);

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
            // Shape corners.
            double minShape = v->min[0];
            double maxShape = v->max[0];
            // As far as we can see.
            double minLimit = v->firstPointAbove(0);
            double maxLimit = v->firstPointBelow(0);

            // Only difference between Open and Close is whether the line
            // segments are at the top or bottom of the shape.  Decide here.
            double lineY = (e->type == Open) ? v->min[1] : v->max[1];

            // Insert possible visibility segments.
            VertInf *vI1 = new VertInf(router, dummyOrthogID, 
                        Point(minShape, lineY));
            VertInf *vI2 = new VertInf(router, dummyOrthogID, 
                        Point(maxShape, lineY));
            segments.insert(LineSegment(minLimit, minShape, lineY,
                        true, NULL, vI1));
            segments.insert(LineSegment(minShape, maxShape, lineY, 
                        true, vI1, vI2));
            segments.insert(LineSegment(maxShape, maxLimit, lineY,
                        true, vI2, NULL));
        }
        else if (e->type == ConnPoint)
        {
            // Connection point.
            VertInf *centreVert = e->v->c;
            Point& cp = centreVert->point;

            // As far as we can see.
            double minLimit = v->firstPointAbove(0);
            double maxLimit = v->firstPointBelow(0);

            bool inShape = v->isInsideShapeX();

            LineSegment *line1 = NULL, *line2 = NULL;
            if (!inShape || (centreVert->visDirections & ConnDirLeft))
            {
                line1 = segments.insert(LineSegment(minLimit, cp.x, e->pos, 
                        true, NULL, centreVert));
            }
            if (!inShape || (centreVert->visDirections & ConnDirRight))
            {
                line2 = segments.insert(LineSegment(cp.x, maxLimit, e->pos, 
                        true, centreVert, NULL));
            }
            if (!line1 && !line2)
            {
                // Add a point segment for the centre point.
                segments.insert(LineSegment(cp.x, cp.x, e->pos, 
                        true, centreVert, NULL));
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
            assert(result == 1);
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
        assert(result.second);

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
            // As far as we can see.
            double minLimit = v->firstPointAbove(1);
            double maxLimit = v->firstPointBelow(1);

            // Only difference between Open and Close is whether the line
            // segments are at the left or right of the shape.  Decide here.
            double lineX = (e->type == Open) ? v->min[0] : v->max[0];

            LineSegment vertSeg = LineSegment(minLimit, maxLimit, lineX);
            segments.insert(vertSeg);
        }
        else if (e->type == ConnPoint)
        {
            // Connection point.
            VertInf *centreVert = e->v->c;
            Point& cp = centreVert->point;

            // As far as we can see.
            double minLimit = v->firstPointAbove(1);
            double maxLimit = v->firstPointBelow(1);
            
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
            assert(result == 1);
            delete v;
        }
    }
}


extern void generateStaticOrthogonalVisGraph(Router *router)
{
    const size_t n = router->shapeRefs.size();
    const unsigned cpn = router->vertices.connsSize();
    // Set up the events for the vertical sweep.
    size_t totalEvents = (2 * n) + cpn;
    events = new Event*[totalEvents];
    unsigned ctr = 0;
    ShapeRefList::iterator shRefIt = router->shapeRefs.begin();
    for (unsigned i = 0; i < n; i++)
    {
        ShapeRef *shRef = *shRefIt;
        double minX, minY, maxX, maxY;
        shRef->polygon().getBoundingRect(&minX, &minY, &maxX, &maxY);
        double midX = minX + ((maxX - minX) / 2);
        Node *v = new Node(shRef, midX);
        events[ctr++] = new Event(Open, v, minY);
        events[ctr++] = new Event(Close, v, maxY);

        ++shRefIt;
    }
    for (VertInf *curr = router->vertices.connsBegin(); 
            curr && (curr != router->vertices.shapesBegin()); 
            curr = curr->lstNext)
    {
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
    assert(scanline.size() == 0);
    for (unsigned i = 0; i < totalEvents; ++i)
    {
        delete events[i];
    }

    segments.list().sort();

    // Set up the events for the horizontal sweep.
    SegmentListWrapper vertSegments;
    ctr = 0;
    shRefIt = router->shapeRefs.begin();
    for (unsigned i = 0; i < n; i++)
    {
        ShapeRef *shRef = *shRefIt;
        double minX, minY, maxX, maxY;
        shRef->polygon().getBoundingRect(&minX, &minY, &maxX, &maxY);
        double midY = minY + ((maxY - minY) / 2);
        Node *v = new Node(shRef, midY);
        events[ctr++] = new Event(Open, v, minX);
        events[ctr++] = new Event(Close, v, maxX);

        ++shRefIt;
    }
    for (VertInf *curr = router->vertices.connsBegin(); 
            curr && (curr != router->vertices.shapesBegin()); 
            curr = curr->lstNext)
    {
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
    assert(scanline.size() == 0);
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
        assert(result.second);

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
        assert(result == 1);
        delete v;
    }
}


static void buildOrthogonalChannelInfo(Router *router, 
        const size_t dim, ShiftSegmentList& segmentList)
{
    if (router->segmt_penalty == 0)
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
        // Determine all line segments that we are interested in shifting. 
        // We don't consider the first or last segment of a path.
        for (size_t i = 2; (i + 1) < displayRoute.size(); ++i)
        {
            if (displayRoute.ps[i - 1][dim] == displayRoute.ps[i][dim])
            {
                // It's a segment in the dimension we are processing.
                size_t indexLow = i - 1;
                size_t indexHigh = i;
                if (displayRoute.ps[i - 1][altDim] > displayRoute.ps[i][altDim])
                {
                    indexLow = i;
                    indexHigh = i - 1;
                }
                assert(displayRoute.at(indexLow)[altDim] < 
                        displayRoute.at(indexHigh)[altDim]);

                bool isSBend = false;
                if (((displayRoute.ps[i - 2][dim] < displayRoute.ps[i][dim]) &&
                     (displayRoute.ps[i + 1][dim] > displayRoute.ps[i][dim])) 
                     ||
                    ((displayRoute.ps[i - 2][dim] > displayRoute.ps[i][dim]) &&
                     (displayRoute.ps[i + 1][dim] < displayRoute.ps[i][dim])) )
                {
                    isSBend = true;
                }
                segmentList.push_back(ShiftSegment(*curr, indexLow, 
                            indexHigh, isSBend, dim));
            }
        }
    }
    if (segmentList.empty())
    {
        // There are no segments, so we can just return now.
        return;
    }
    
    // Do a sweep and shift these segments.
    const size_t n = router->shapeRefs.size();
    const size_t cpn = segmentList.size();
    // Set up the events for the sweep.
    size_t totalEvents = 2 * (n + cpn);
    events = new Event*[totalEvents];
    unsigned ctr = 0;
    ShapeRefList::iterator shRefIt = router->shapeRefs.begin();
    for (unsigned i = 0; i < n; i++)
    {
        ShapeRef *shRef = *shRefIt;
        Point min, max;
        shRef->polygon().getBoundingRect(&min.x, &min.y, &max.x, &max.y);
        double mid = min[dim] + ((max[dim] - min[dim]) / 2);
        Node *v = new Node(shRef, mid);
        events[ctr++] = new Event(Open, v, min[altDim]);
        events[ctr++] = new Event(Close, v, max[altDim]);

        ++shRefIt;
    }
    for (ShiftSegmentList::iterator curr = segmentList.begin(); 
            curr != segmentList.end(); ++curr)
    {
        const Point& lowPt = curr->lowPoint();
        const Point& highPt = curr->highPoint();

        assert(lowPt[dim] == highPt[dim]);
        assert(lowPt[altDim] < highPt[altDim]);
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
    assert(scanline.size() == 0);
    for (unsigned i = 0; i < totalEvents; ++i)
    {
        delete events[i];
    }
    delete [] events;
}


static void centreOrthogonalChannelRoutes(Router *router, 
        const size_t dim, ShiftSegmentList& segmentList)
{
    for (ShiftSegmentList::iterator curr = segmentList.begin(); 
            curr != segmentList.end(); ++curr)
    {
        ShiftSegment& ss = *curr;
        
        Point& lowPt = ss.lowPoint();
        Point& highPt = ss.highPoint();

#if 0
            // Set reasonable outside channel size, rather than infinity.
            double outside_channel = 30.0;
            if (ss.minSpaceLimit == -DBL_MAX)
            {
                ss.minSpaceLimit = ss.maxSpaceLimit - outside_channel;
            }
            if (ss.maxSpaceLimit == DBL_MAX)
            {
                ss.maxSpaceLimit = ss.minSpaceLimit + outside_channel;
            }
#endif


        if (ss.sBend && (ss.minSpaceLimit > -CHANNEL_MAX) &&
                (ss.maxSpaceLimit < CHANNEL_MAX))
        {
            double halfWay = ss.minSpaceLimit + 
                    ((ss.maxSpaceLimit - ss.minSpaceLimit) / 2);

            lowPt[dim] = halfWay;
            highPt[dim] = halfWay;
        }

#if defined(LIBAVOID_SDL)
            if (router->avoid_screen)
            {
                Point can(151, 55);

                Point min;
                Point max;

                size_t altDim = (dim + 1) % 2;
                min[altDim] = lowPt[altDim] + can[altDim];
                min[dim] = std::max(-5000.0, ss.minSpaceLimit) + can[dim];
                max[altDim] = highPt[altDim] + can[altDim];
                max[dim] = std::min(5000.0, ss.maxSpaceLimit) + can[dim];

                boxRGBA(router->avoid_screen, (int) min.x, (int) min.y, 
                        (int) max.x, (int) max.y, 
                        ((dim) ? 0 : 255), ((dim) ? 255 : 0), 0, 64);
            }
#endif
    }
}


static void buildOrthogonalNudgingOrderInfo(Router *router, 
        PtOrderMap& pointOrders)
{
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
            bool checkForBranchingSegments = false;
            int crossings = 0;
            for (size_t i = 1; i < route.size(); ++i)
            {
                const bool finalSegment = ((i + 1) == route.size());
                crossings += countRealCrossings(route2, true, route, i, 
                        checkForBranchingSegments, finalSegment, NULL, 
                        &pointOrders, NULL, NULL, conn2, conn);
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

        for (size_t dim = 0; dim < 2; ++dim)
        {
            order.sort(dim);
        }
    }
}


class CmpLineOrder 
{
    public:
        CmpLineOrder(PtOrderMap& orders, const size_t dim)
            : orders(orders),
              dimension(dim)
        {
        }
        bool operator()(const ShiftSegment& lhs, const ShiftSegment& rhs)
        {
            Point lhsLow  = lhs.lowPoint(); 
            Point rhsLow  = rhs.lowPoint(); 
#ifndef NDEBUG
            const Point& lhsHigh = lhs.highPoint(); 
            const Point& rhsHigh = rhs.highPoint(); 
#endif
            size_t altDim = (dimension + 1) % 2;

            assert(lhsLow[dimension] == lhsHigh[dimension]);
            assert(rhsLow[dimension] == rhsHigh[dimension]);

            if (lhsLow[dimension] != rhsLow[dimension])
            {
                return lhsLow[dimension] < rhsLow[dimension];
            }
            
            // Need to index using the original point into the map, so find it.
            Point& unchanged = (lhsLow[altDim] > rhsLow[altDim]) ?
                    lhsLow : rhsLow;

            // One might be longer than the other, so use common positon.
            lhsLow[altDim] = std::max(lhsLow[altDim], rhsLow[altDim]);
            rhsLow[altDim] = lhsLow[altDim];

            PtOrder& lowOrder = orders[unchanged];
            int lhsPos = lowOrder.positionFor(lhs.connRef, dimension);
            int rhsPos = lowOrder.positionFor(rhs.connRef, dimension);
            assert(lhsPos != -1);
            assert(rhsPos != -1);

            return lhsPos < rhsPos;
        }

        PtOrderMap& orders;
        const size_t dimension;
};


static void nudgeOrthogonalRoutes(Router *router, size_t dimension, 
        PtOrderMap& pointOrders, ShiftSegmentList& segmentList)
{
    double nudgeDist = 4;

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
                curr = segmentList.erase(curr);
            }
            else
            {
                ++curr;
            }
        }
        CmpLineOrder lineSort(pointOrders, dimension);
        currentRegion.sort(lineSort);
        
        // Process these segments.
        Variables vs;
        Constraints cs;
        Variable *lastVar = NULL;
        size_t lastIndex = 0;
        for (ShiftSegmentList::iterator currRegion = currentRegion.begin();
                currRegion != currentRegion.end(); ++currRegion)
        {
            double halfWay = currRegion->minSpaceLimit + 
                    ((currRegion->maxSpaceLimit - 
                      currRegion->minSpaceLimit) / 2);
            Point& lowPt = currRegion->lowPoint();
            double idealPos = 
                    (currRegion->sBend) ? halfWay : lowPt[dimension];

            currRegion->variable = new Variable(0, idealPos);
            vs.push_back(currRegion->variable);

            size_t index = vs.size() - 1;
            if (lastVar)
            {
                cs.push_back(
                        new Constraint(vs[lastIndex],vs[index],nudgeDist));
                if (vs[lastIndex]->desiredPosition == 
                        vs[index]->desiredPosition)
                {
                    vs[index]->desiredPosition  += 0.1;
                }
            }
            lastIndex = index;
            if (currRegion->minSpaceLimit > -CHANNEL_MAX)
            {
                vs.push_back(new Variable(1, currRegion->minSpaceLimit, 
                            100000));
                cs.push_back(new Constraint(vs[vs.size() - 1], vs[index], 
                            0.0));
            }
            if (currRegion->maxSpaceLimit < CHANNEL_MAX)
            {
                vs.push_back(new Variable(1, currRegion->maxSpaceLimit, 
                            100000));
                cs.push_back(new Constraint(vs[index], vs[vs.size() - 1],
                            0.0));
            }
            lastVar = currRegion->variable;
        }
#if 0
        for(unsigned i=0;i<vs.size();i++) {
            printf("-vs[%d]=%f\n",i,vs[i]->desiredPosition);
        }
#endif
        Solver f(vs,cs);
        f.satisfy();
        bool satisfied = true;
        for (size_t i = 0; i < vs.size(); ++i) 
        {
            if (vs[i]->id == 1)
            {
                if (fabs(vs[i]->finalPosition - vs[i]->desiredPosition) > 0.01)
                {
                    satisfied = false;
                    break;
                }
            }
        }
        if (satisfied)
        {
            for (ShiftSegmentList::iterator currRegion = currentRegion.begin();
                    currRegion != currentRegion.end(); ++currRegion)
            {
                Point& lowPt = currRegion->lowPoint();
                Point& highPt = currRegion->highPoint();
                double newPos = currRegion->variable->finalPosition;
                //db_printf("Pos: %X, %g\n", (int) currRegion->connRef, newPos);
                lowPt[dimension] = newPos;
                highPt[dimension] = newPos;
            }
        }
#if 0
        for(unsigned i=0;i<vs.size();i++) {
            printf("+vs[%d]=%f\n",i,vs[i]->finalPosition);
        }
#endif
        for_each(vs.begin(),vs.end(),delete_object());
        for_each(cs.begin(),cs.end(),delete_object());
    }
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


extern void improveOrthogonalRoutes(Router *router)
{
    ShiftSegmentList segLists[2];

    router->timers.Register(tmOrthogCentre, timerStart);
    // Centre vertical segments in horizontal space.
    buildOrthogonalChannelInfo(router, 0, segLists[0]);
    centreOrthogonalChannelRoutes(router, 0, segLists[0]);
 
    // Centre horizontal segments in vertical space.
    buildOrthogonalChannelInfo(router, 1, segLists[1]);
    centreOrthogonalChannelRoutes(router, 1, segLists[1]);

    simplifyOrthogonalRoutes(router);

    // Centre vertical segments in horizontal space.
    segLists[0].clear();
    buildOrthogonalChannelInfo(router, 0, segLists[0]);
    centreOrthogonalChannelRoutes(router, 0, segLists[0]);
    router->timers.Stop();
   
#if defined(LIBAVOID_SDL)
    if (router->avoid_screen)
    {
        SDL_Flip(router->avoid_screen);
        SDL_Delay(1000);
    }
#endif

    router->timers.Register(tmOrthogNudge, timerStart);
    {
        // Build nudging info.
        PtOrderMap pointOrders;
        buildOrthogonalNudgingOrderInfo(router, pointOrders);

        // Simplify routes.
        simplifyOrthogonalRoutes(router);

        // Rebuild the horizontal channel information:
        segLists[0].clear();
        buildOrthogonalChannelInfo(router, 0, segLists[0]);
        nudgeOrthogonalRoutes(router, 0, pointOrders, segLists[0]);
    }

    // XXX: we need to build the point orders separately in each
    // dimension since things move.  There might be a more efficient
    // way to do this.
    {
        // Build nudging info.
        PtOrderMap pointOrders;
        buildOrthogonalNudgingOrderInfo(router, pointOrders);

        // Simplify routes.
        simplifyOrthogonalRoutes(router);

        segLists[1].clear();
        buildOrthogonalChannelInfo(router, 1, segLists[1]);
        nudgeOrthogonalRoutes(router, 1, pointOrders, segLists[1]);
    }
    router->timers.Stop();
}


}
