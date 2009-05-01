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


#include <set>
#include <stdlib.h>
#include <cfloat>

#include "libavoid/router.h"
#include "libavoid/geomtypes.h"
#include "libavoid/shape.h"
#include "libavoid/orthogonal.h"


namespace Avoid {

struct Node;
struct CmpNodePos { bool operator()(const Node* u, const Node* v) const; };

typedef std::set<Node*,CmpNodePos> NodeSet;
struct Node 
{
    ShapeRef *v;
    VertInf *c;
    double pos;
    Node *firstAbove, *firstBelow;
    NodeSet *leftNeighbours, *rightNeighbours;
    Node(ShapeRef *v, double p)
        : v(v),c(NULL),pos(p),
          firstAbove(NULL), firstBelow(NULL),
          leftNeighbours(NULL), rightNeighbours(NULL)
    {   
        //assert(r->width()<1e40);
    }   
    Node(VertInf *c, double p)
        : v(NULL),c(c),pos(p),
          firstAbove(NULL), firstBelow(NULL),
          leftNeighbours(NULL), rightNeighbours(NULL)
    {   
    }   
    ~Node() 
    {
        delete leftNeighbours; 
        delete rightNeighbours;
    }   
    void addLeftNeighbour(Node *u) 
    {
        assert(leftNeighbours!=NULL);
        leftNeighbours->insert(u);
    }   
    void addRightNeighbour(Node *u) 
    {
        assert(rightNeighbours!=NULL);
        rightNeighbours->insert(u);
    }   
    void setNeighbours(NodeSet *left, NodeSet *right) 
    {
        leftNeighbours=left; 
        rightNeighbours=right;
        for (NodeSet::iterator i=left->begin();i!=left->end();++i)
        {
            Node *v=*(i);    
            v->addRightNeighbour(this);
        }
        for (NodeSet::iterator i=right->begin();i!=right->end();++i) 
        {
            Node *v=*(i);    
            v->addLeftNeighbour(this);
        }
    }
};


bool CmpNodePos::operator() (const Node* u, const Node* v) const 
{
    //assert(!isNaN(u->pos));
    //assert(!isNaN(v->pos));
    if (u->pos != v->pos) 
    {
        return u->pos < v->pos;
    }
    return u->v < v->v;
}       

// Note: Open must come first.
typedef enum {Open = 1, Centre = 2, ConnPoint = 3, Close = 4} EventType;

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
#if 0
    else if (isNaN(ea->pos) != isNaN(ea->pos)) 
    {
        /* See comment in CmpNodePos. */
        return ( isNaN(ea->pos) ? -1 : 1 );
    }
#endif
}


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
          shapeSide(false),
          beginVertInf(bvi),
          finishVertInf(fvi)
    {
        //assert( begin <= finish);
        if (begin > finish)
        {
            // XXX: This shouldn't be necessary.  
            //      Is it due to a bug in the sweep code?
            double temp = begin;
            begin = finish;
            finish = temp;

            VertInf *tempVI = beginVertInf;
            beginVertInf = finishVertInf;
            finishVertInf = tempVI;
        }
    }
    
    bool operator==(const LineSegment& rhs) const 
    {
        return ((begin == rhs.begin) && (pos == rhs.pos) &&
                (finish == rhs.finish) && 
                (!beginVertInf || !rhs.beginVertInf || 
                        (beginVertInf == rhs.beginVertInf)) &&
                (!finishVertInf || !rhs.finishVertInf || 
                        (finishVertInf == rhs.finishVertInf)));
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

    double begin;
    double finish;
    double pos;
    bool shapeSide;
    VertInf *beginVertInf;
    VertInf *finishVertInf;
private:
	// MSVC wants to generate the assignment operator and the default 
	// constructor, but fails.  Therefore we declare them private and 
	// don't implement them.
	LineSegment & operator=(LineSegment const &);
	LineSegment();
};
typedef std::list<LineSegment> SegmentList;


struct PosVertInf
{
    PosVertInf(double p, VertInf *vI = NULL)
        : pos(p),
          vert(vI)
    {
    }
    
    bool operator<(const PosVertInf& rhs) const 
    {
        return pos < rhs.pos;
    }

    double pos;
    VertInf *vert;
};


// Given a router instance and a set of possible horizontal segments, and a
// possible vertical visibility segments, compute and add edges to the
// orthogonal visibility graph for all the visibility edges.
static void mergeSegments(Router *router, SegmentList& segments, 
        LineSegment& vertLine)
{
    const bool orthogonal = true;
    // XXX: Are there possibly duplicates in SegmentList?
    //
    // Do some sorting.
    std::set<PosVertInf> breakPoints;
    assert(vertLine.beginVertInf == NULL);
    assert(vertLine.finishVertInf == NULL);
    for (SegmentList::iterator it = segments.begin(); it != segments.end(); )
    {
        LineSegment& horiLine = *it;

        if (horiLine.begin == horiLine.finish)
        {
            // Skip this point segment.
            it = segments.erase(it);
            continue;
        }
        SegmentList::iterator next = it;
        ++next;
        if ((next != segments.end()) && (*next == horiLine))
        {
            //db_printf("SKIP %x %x %x %x\n", horiLine.beginVertInf, 
            //        horiLine.finishVertInf, next->beginVertInf,
            //        next->finishVertInf);
            if (next->finishVertInf == NULL)
            {
                next->finishVertInf = horiLine.finishVertInf;
            }
            if (next->beginVertInf == NULL)
            {
                next->beginVertInf = horiLine.beginVertInf;
            }
            // Skip this point segment.
            it = segments.erase(it);
            continue;
        }

        bool inVertSegRegion = ((vertLine.begin <= horiLine.pos) &&
                                (vertLine.finish >= horiLine.pos));

        if (horiLine.finish < vertLine.pos)
        {
            // XXX Is this even needed?

            if (inVertSegRegion)
            {
                // Add horizontal visibility segment.
                if (!horiLine.beginVertInf)
                {
                    horiLine.beginVertInf = new VertInf(router, 
                            VertID(0, true, 0), 
                            Point(horiLine.begin, horiLine.pos));
                }
                VertInf *vI2 = horiLine.finishVertInf ? horiLine.finishVertInf :
                        new VertInf(router, 
                        VertID(0, true, 0),
                        Point(horiLine.finish, horiLine.pos));
                EdgeInf *edge = new EdgeInf(horiLine.beginVertInf, vI2, 
                        orthogonal);
                edge->setDist(horiLine.finish - horiLine.begin);
            }

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
                if (!horiLine.beginVertInf)
                {
                    if (horiLine.pos == vertLine.begin && vertLine.beginVertInf)
                    {
                        horiLine.beginVertInf = vertLine.beginVertInf;
                    }
                    else if (horiLine.pos == vertLine.finish && 
                            vertLine.finishVertInf)
                    {
                        horiLine.beginVertInf = vertLine.finishVertInf;
                    }
                    else
                    {
                        horiLine.beginVertInf = new VertInf(router, 
                                VertID(0, true, 0), 
                                Point(horiLine.begin, horiLine.pos));
                    }
                }

                breakPoints.insert(
                        PosVertInf(horiLine.pos, horiLine.beginVertInf));
            }
        }
        else if (horiLine.finish == vertLine.pos)
        {
            if (inVertSegRegion)
            {
                // Add horizontal visibility segment.
                if (!horiLine.beginVertInf)
                {
                    horiLine.beginVertInf = new VertInf(router, 
                            VertID(0, true, 0), 
                            Point(horiLine.begin, horiLine.pos));
                }
                VertInf *vI2 = horiLine.finishVertInf ? horiLine.finishVertInf :
                        new VertInf(router, 
                        VertID(0, true, 0),
                        Point(horiLine.finish, horiLine.pos));
                EdgeInf *edge = new EdgeInf(horiLine.beginVertInf, vI2,
                        orthogonal);
                edge->setDist(horiLine.finish - horiLine.begin);
            
                breakPoints.insert(PosVertInf(horiLine.pos, vI2));
                
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
                if (!horiLine.beginVertInf)
                {
                    horiLine.beginVertInf = new VertInf(router, 
                            VertID(0, true, 0), 
                            Point(horiLine.begin, horiLine.pos));
                }
                VertInf *vI2 = new VertInf(router, 
                        VertID(0, true, 0),
                        Point(vertLine.pos, horiLine.pos));
                EdgeInf *edge = 
                        new EdgeInf(horiLine.beginVertInf, vI2, orthogonal);
                edge->setDist(vertLine.pos - horiLine.begin);
            
                horiLine.begin = vertLine.pos;
                horiLine.beginVertInf = vI2;
                
                breakPoints.insert(
                        PosVertInf(horiLine.pos, horiLine.beginVertInf));
            }
        }
        ++it;
    }
    if ((breakPoints.begin())->pos != vertLine.begin)
    {
        // Add begin point if it din't intersect another line.
        breakPoints.insert(PosVertInf(vertLine.begin, vertLine.beginVertInf));
    }
    if ((--breakPoints.end())->pos != vertLine.finish)
    {
        // Add finish point if it din't intersect another line.
        breakPoints.insert(PosVertInf(vertLine.finish, vertLine.finishVertInf));
    }

    std::set<PosVertInf>::iterator prev = breakPoints.begin();
    std::set<PosVertInf>::iterator curr = prev;
    VertInf *prevInf = NULL;
    ++curr;
    for (; curr != breakPoints.end(); ++curr, ++prev)
    {
        // Add vertical visibility segment.
        if (prevInf == NULL)
        {
            prevInf = (prev->vert) ? prev->vert : 
                    new VertInf(router, VertID(0, true, 0), 
                    Point(vertLine.pos, prev->pos));
        }
        assert((prev->vert == NULL) || (prevInf == prev->vert));

        //assert((curr == breakPoints.end()) || (curr->vert));
        VertInf *currInf = curr->vert ? curr->vert : 
                new VertInf(router, VertID(0, true, 0),
                    Point(vertLine.pos, curr->pos));
        EdgeInf *edge = new EdgeInf(prevInf, currInf, orthogonal);
        edge->setDist(curr->pos - prev->pos);
        prevInf = currInf;
    }
}


void generateStaticOrthogonalVisGraph(Router *router)
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

        // Centre point.
        //double midY = minY + ((maxY - minY) / 2);
        //events[ctr++] = new Event(Centre, v, midY);
        
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

    // Process the vertical sweep
    SegmentList segments;
    NodeSet scanline;
    for (unsigned i = 0; i < totalEvents; ++i)
    {
        Event *e = events[i];
        Node *v = e->v;
        if (e->type == Open) 
        {
            //fprintf(stderr, "1Open %g\n", e->pos); fflush(stderr);
                
            // Work out neighbours
            NodeSet::iterator it = scanline.insert(v).first;
            if (it != scanline.begin()) 
            {
                Node *u = *(--it);
                v->firstAbove = u;
                u->firstBelow = v;
                ++it;
            }
            if (++it != scanline.end()) 
            {
                Node *u = *it;
                v->firstBelow = u;
                u->firstAbove = v;
            }

            // If we are the last event or the next event is not an open,
            // then we are the last in a series of Opens, so we process all
            // these Open events.
            bool lastOpen = ((i == totalEvents) || 
                    (events[i]->pos != events[i + 1]->pos) ||
                    (events[i + 1]->type != Open));
            if (lastOpen)
            {
                unsigned j = i;
                do
                {
                    Node *u = e->v;
                    double minX[3], minY[3], maxX[3];
                    maxX[0] = -DBL_MAX;
                    minX[2] = DBL_MAX;
                    u->v->polygon().getBoundingRect(
                            &minX[1], &minY[1], &maxX[1], NULL);
                    if (u->firstAbove)
                    {
                        u->firstAbove->v->polygon().getBoundingRect(
                                NULL, NULL, &maxX[0], NULL);
                    }
                    if (u->firstBelow)
                    {
                        u->firstBelow->v->polygon().getBoundingRect(
                                &minX[2], NULL, NULL, NULL);
                    }
                    // Insert possible visibility segments.
                    VertInf *vI1 = new VertInf(router, VertID(0, true, 0), 
                                Point(minX[1], minY[1]));
                    VertInf *vI2 = new VertInf(router, VertID(0, true, 0), 
                                Point(maxX[1], minY[1]));
                    segments.push_back(LineSegment(maxX[0], minX[1], minY[1],
                                true, NULL, vI1));
                    segments.push_back(
                            LineSegment(minX[1], maxX[1], minY[1], true,
                                vI1, vI2));
                    segments.push_back(LineSegment(maxX[1], minX[2], minY[1],
                                true, vI2, NULL));

                    if (j == 0) 
                    {
                        break;
                    }
                    e = events[--j];
                }
                while ((e->type == Open) && (e->pos == events[j + 1]->pos));
            }
        }
        else if (e->type == Close) 
        {
            //fprintf(stderr, "1Close %g\n", e->pos); fflush(stderr);
            // If the previous event is not a Close, then we are the first 
            // in a (possible) series of Closes, so we process all these 
            // Close events. (There will always be open events before this.)
            bool firstClose = ((events[i]->pos != events[i - 1]->pos) ||
                    (events[i - 1]->type != Close));
            if (firstClose)
            {
                unsigned j = i;
                do
                {
                    Node *u = e->v;
                    double minX[3], maxY[3], maxX[3];
                    maxX[0] = -DBL_MAX;
                    minX[2] = DBL_MAX;
                    u->v->polygon().getBoundingRect(
                            &minX[1], NULL, &maxX[1], &maxY[1]);
                    if (u->firstAbove)
                    {
                        u->firstAbove->v->polygon().getBoundingRect(
                                NULL, NULL, &maxX[0], NULL);
                    }
                    if (u->firstBelow)
                    {
                        u->firstBelow->v->polygon().getBoundingRect(
                                &minX[2], NULL, NULL, NULL);
                    }
                    // Insert possible visibility segments.
                    VertInf *vI1 = new VertInf(router, VertID(0, true, 0), 
                                Point(minX[1], maxY[1]));
                    VertInf *vI2 = new VertInf(router, VertID(0, true, 0), 
                                Point(maxX[1], maxY[1]));
                    segments.push_back(LineSegment(maxX[0], minX[1], maxY[1],
                                true, NULL, vI1));
                    segments.push_back(
                            LineSegment(minX[1], maxX[1], maxY[1], true,
                                vI1, vI2));
                    segments.push_back(LineSegment(maxX[1], minX[2], maxY[1],
                                true, vI2, NULL));

                    if ((j + 1) == totalEvents) 
                    {
                        break;
                    }
                    e = events[++j];
                }
                while ((e->type == Close) && (e->pos == events[j - 1]->pos));
            } 
            
            // Close event
            size_t result;
            Node *l = v->firstAbove, *r = v->firstBelow;
            if (l != NULL) 
            {
                l->firstBelow=v->firstBelow;
            }
            if (r != NULL)
            {
                r->firstAbove=v->firstAbove;
            }
            result = scanline.erase(v);
            assert(result == 1);
            delete v;
        }
        else if (e->type == ConnPoint) 
        {
            // Centre point:
            VertInf *centreVert = e->v->c;
            Point& cp = centreVert->point;

            // Work out neighbours
            //
            // Note: We can't just use the immediate neghbour, because that
            //       might actually be a shape we are inside, thus keep 
            //       look at consecutive neighbours till we find a boundry 
            //       on the correct side.
            NodeSet::iterator insertIt = scanline.insert(v).first;
            NodeSet::iterator it = insertIt;
            if (it != scanline.begin()) 
            {
                double pos = DBL_MAX;
                do
                {
                    v->firstAbove = (it != scanline.begin()) ? *(--it) : NULL;
                    if (v->firstAbove)
                    {
                        v->firstAbove->v->polygon().getBoundingRect(
                                NULL, NULL, &pos, NULL);
                    }
                }
                while (it != scanline.begin() && pos >= v->c->point.x);
            }
            it = insertIt;
            if (it != scanline.end()) 
            {
                double pos = -DBL_MAX;
                do
                {
                    ++it;
                    v->firstBelow = (it != scanline.end()) ? *(it) : NULL;
                    if (v->firstBelow)
                    {
                        v->firstBelow->v->polygon().getBoundingRect(
                                &pos, NULL, NULL, NULL);
                    }
                }
                while (it != scanline.end() && pos <= v->c->point.x);
            }

            double minX = -DBL_MAX, maxX = DBL_MAX;
            if (v->firstAbove)
            {
                v->firstAbove->v->polygon().getBoundingRect(
                        NULL, NULL, &minX, NULL);
            }
            if (v->firstBelow)
            {
                v->firstBelow->v->polygon().getBoundingRect(
                        &maxX, NULL, NULL, NULL);
            }
            // Insert possible visibility segments.
            segments.push_back(
                    LineSegment(minX, cp.x, e->pos, true, NULL, centreVert));
            segments.push_back(
                    LineSegment(cp.x, maxX, e->pos, true, centreVert, NULL));
            
            scanline.erase(insertIt);
        }
    }
    assert(scanline.size() == 0);
    for (unsigned i = 0; i < totalEvents; ++i)
    {
        delete events[i];
    }

    segments.sort();

    // Set up the events for the horizontal sweep.
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
    for (unsigned i = 0; i < totalEvents; ++i)
    {
        Event *e = events[i];
        Node *v = e->v;
        if (e->type == Open) 
        {
            //fprintf(stderr, "Open  %g %X\n", v->pos, (int) v); fflush(stderr);

            // Work out neighbours
            NodeSet::iterator it = scanline.insert(v).first;
            if (it != scanline.begin()) 
            {
                Node *u = *(--it);
                v->firstAbove = u;
                u->firstBelow = v;
                ++it;
            }
            if (++it != scanline.end()) 
            {
                Node *u = *it;
                v->firstBelow = u;
                u->firstAbove = v;
            }

            // If we are the last event or the next event is not an open,
            // then we are the last in a series of Opens, so we process all
            // these Open events.
            bool lastOpen = ((i == totalEvents) || 
                    (events[i]->pos != events[i + 1]->pos) ||
                    (events[i + 1]->type != Open));
            if (lastOpen)
            {
                unsigned j = i;
                do
                {
                    Node *u = e->v;
                    double minY[3], minX[3], maxY[3];
                    maxY[0] = -DBL_MAX;
                    minY[2] = DBL_MAX;
                    u->v->polygon().getBoundingRect(
                            &minX[1], &minY[1], NULL, &maxY[1]);
                    if (u->firstAbove)
                    {
                        u->firstAbove->v->polygon().getBoundingRect(
                                NULL, NULL, NULL, &maxY[0]);
                    }
                    if (u->firstBelow)
                    {
                        u->firstBelow->v->polygon().getBoundingRect(
                                NULL, &minY[2], NULL, NULL);
                    }
                    LineSegment vertSeg = LineSegment(maxY[0], minY[2], minX[1]);
                    mergeSegments(router, segments, vertSeg);

                    if (j == 0) 
                    {
                        break;
                    }
                    e = events[--j];
                }
                while ((e->type == Open) && (e->pos == events[j + 1]->pos));
            }
        }
        else if (e->type == Close) 
        {
            //fprintf(stderr, "Close %g %X\n", v->pos, (int) v); fflush(stderr);

            // If the previous event is not a Close, then we are the first 
            // in a (possible) series of Closes, so we process all these 
            // Close events. (There will always be open events before this.)
            bool firstClose = ((events[i]->pos != events[i - 1]->pos) ||
                    (events[i - 1]->type != Close));
            if (firstClose)
            {
                unsigned j = i;
                do
                {
                    Node *u = e->v;
                    double minY[3], maxX[3], maxY[3];
                    maxY[0] = -DBL_MAX;
                    minY[2] = DBL_MAX;
                    u->v->polygon().getBoundingRect(
                            NULL, &minY[1], &maxX[1], &maxY[1]);
                    if (u->firstAbove)
                    {
                        u->firstAbove->v->polygon().getBoundingRect(
                                NULL, NULL, NULL, &maxY[0]);
                        }
                    if (u->firstBelow)
                    {
                        u->firstBelow->v->polygon().getBoundingRect(
                                NULL, &minY[2], NULL, NULL);
                    }
                    LineSegment vertSeg = LineSegment(maxY[0], minY[2], maxX[1]);
                    mergeSegments(router, segments, vertSeg);

                    if ((j + 1) == totalEvents) 
                    {
                        break;
                    }
                    e = events[++j];
                }
                while ((e->type == Close) && (e->pos == events[j - 1]->pos));
            } 

            
            // Close event
            size_t result;
            Node *l = v->firstAbove, *r = v->firstBelow;
            if (l != NULL) 
            {
                l->firstBelow=v->firstBelow;
            }
            if (r != NULL)
            {
                r->firstAbove=v->firstAbove;
            }
            result = scanline.erase(v);
            assert(result == 1);
            delete v;
        }
        else if (e->type == ConnPoint)
        {
            // Work out neighbours
            //
            // Note: We can't just use the immediate neghbour, because that
            //       might actually be a shape we are inside, thus keep 
            //       look at consecutive neighbours till we find a boundry 
            //       on the correct side.
            NodeSet::iterator insertIt = scanline.insert(v).first;
            NodeSet::iterator it = insertIt;
            if (it != scanline.begin()) 
            {
                double pos = DBL_MAX;
                do
                {
                    v->firstAbove = (it != scanline.begin()) ? *(--it) : NULL;
                    if (v->firstAbove)
                    {
                        v->firstAbove->v->polygon().getBoundingRect(
                                NULL, NULL, NULL, &pos);
                    }
                }
                while (it != scanline.begin() && pos >= v->c->point.y);
            }
            it = insertIt;
            if (it != scanline.end()) 
            {
                double pos = -DBL_MAX;
                do
                {
                    ++it;
                    v->firstBelow = (it != scanline.end()) ? *(it) : NULL;
                    if (v->firstBelow)
                    {
                        v->firstBelow->v->polygon().getBoundingRect(
                                NULL, &pos, NULL, NULL);
                    }
                }
                while (it != scanline.end() && pos <= v->c->point.y);
            }

            double minY = -DBL_MAX, maxY = DBL_MAX;
            if (v->firstAbove)
            {
                v->firstAbove->v->polygon().getBoundingRect(
                        NULL, NULL, NULL, &minY);
            }
            if (v->firstBelow)
            {
                v->firstBelow->v->polygon().getBoundingRect(
                        NULL, &maxY, NULL, NULL);
            }
            LineSegment vertSeg = LineSegment(minY, maxY, e->pos);
            mergeSegments(router, segments, vertSeg);
            
            scanline.erase(insertIt);
        }
    }
    assert(scanline.size() == 0);
    for (unsigned i = 0; i < totalEvents; ++i)
    {
        delete events[i];
    }
    delete [] events;

    // Add portions of the horizontal line that are after the final vertical
    // position we considered.
    const bool orthogonal = true;
    for (SegmentList::iterator it = segments.begin(); it != segments.end(); )
    {
        LineSegment& horiLine = *it;

        VertInf *vI1 = horiLine.beginVertInf ? horiLine.beginVertInf :
                new VertInf(router, VertID(0, true, 0), 
                Point(horiLine.begin, horiLine.pos));
        VertInf *vI2 = new VertInf(router, VertID(0, true, 0),
                Point(horiLine.finish, horiLine.pos));
        EdgeInf *edge = new EdgeInf(vI1, vI2, orthogonal);
        edge->setDist(horiLine.finish - horiLine.begin);
        
        it = segments.erase(it);
    }
}


}
