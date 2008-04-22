/*
 * vim: ts=4 sw=4 et tw=0 wm=0
 *
 * libavoid - Fast, Incremental, Object-avoiding Line Router
 * Copyright (C) 2004-2008  Michael Wybrow <mjwybrow@users.sourceforge.net>
 *
 * --------------------------------------------------------------------
 * The Visibility Sweep technique is based upon the method described
 * in Section 5.2 of:
 *     Lee, D.-T. (1978). Proximity and reachability in the plane.,
 *     PhD thesis, Department of Electrical Engineering, 
 *     University of Illinois, Urbana, IL.
 * --------------------------------------------------------------------
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
*/


#include <algorithm>
#include <cfloat>

#include "libavoid/shape.h"
#include "libavoid/debug.h"
#include "libavoid/visibility.h"
#include "libavoid/vertices.h"
#include "libavoid/graph.h"
#include "libavoid/geometry.h"
#include "libavoid/router.h"

#include <math.h>

#ifdef LINEDEBUG
  #include "SDL_gfxPrimitives.h"
#endif

namespace Avoid {


void shapeVis(ShapeRef *shape)
{
    Router *router = shape->router();

    if ( !(router->InvisibilityGrph) )
    {
        // Clear shape from graph.
        shape->removeFromGraph();
    }

    VertInf *shapeBegin = shape->firstVert();
    VertInf *shapeEnd = shape->lastVert()->lstNext;

    VertInf *pointsBegin = NULL;
    if (router->IncludeEndpoints)
    {
        pointsBegin = router->vertices.connsBegin();
    }
    else
    {
        pointsBegin = router->vertices.shapesBegin();
    }

    for (VertInf *curr = shapeBegin; curr != shapeEnd; curr = curr->lstNext)
    {
        bool knownNew = true;

        db_printf("-- CONSIDERING --\n");
        curr->id.db_print();

        db_printf("\tFirst Half:\n");
        for (VertInf *j = pointsBegin ; j != curr; j = j->lstNext)
        {
            EdgeInf::checkEdgeVisibility(curr, j, knownNew);
        }

        db_printf("\tSecond Half:\n");
        VertInf *pointsEnd = router->vertices.end();
        for (VertInf *k = shapeEnd; k != pointsEnd; k = k->lstNext)
        {
            EdgeInf::checkEdgeVisibility(curr, k, knownNew);
        }
    }
}


void shapeVisSweep(ShapeRef *shape)
{
    Router *router = shape->router();

    if ( !(router->InvisibilityGrph) )
    {
        // Clear shape from graph.
        shape->removeFromGraph();
    }

    VertInf *startIter = shape->firstVert();
    VertInf *endIter = shape->lastVert()->lstNext;

    for (VertInf *i = startIter; i != endIter; i = i->lstNext)
    {
        vertexSweep(i);
    }
}


void vertexVisibility(VertInf *point, VertInf *partner, bool knownNew,
        const bool gen_contains)
{
    Router *router = point->_router;
    const VertID& pID = point->id;

    // Make sure we're only doing ptVis for endpoints.
    assert(!(pID.isShape));

    if ( !(router->InvisibilityGrph) )
    {
        point->removeFromGraph();
    }

    if (gen_contains && !(pID.isShape))
    {
        router->generateContains(point);
    }

    if (router->UseLeesAlgorithm)
    {
        vertexSweep(point);
    }
    else
    {
        VertInf *shapesEnd = router->vertices.end();
        for (VertInf *k = router->vertices.shapesBegin(); k != shapesEnd;
                k = k->lstNext)
        {
            EdgeInf::checkEdgeVisibility(point, k, knownNew);
        }
        if (router->IncludeEndpoints && partner)
        {
            EdgeInf::checkEdgeVisibility(point, partner, knownNew);
        }
    }
}


//============================================================================
//  SWEEP CODE
//

static VertInf *centerInf;
static Point centerPoint;
static VertID centerID;
static double centerAngle;


class PointPair
{
    public:
        PointPair(VertInf *inf)
            : vInf(inf)
        {
            double x = vInf->point.x - centerPoint.x;
            double y = vInf->point.y - centerPoint.y;

            angle = pos_to_angle(x, y);
            distance = dist(centerPoint, vInf->point);
        }
        bool operator==(const PointPair& rhs) const
        {
            if (vInf->id == rhs.vInf->id)
            {
                return true;
            }
            return false;
        }
        bool operator<(const PointPair& rhs) const
        {
            if (angle == rhs.angle)
            {
                // If the points are colinear, then order them in increasing
                // distance from the point we are sweeping around.
                return distance < rhs.distance;
            }
            return angle < rhs.angle;
        }
        static double pos_to_angle(double x, double y)
        {
            double ang = atan(y / x);
            ang = (ang * 180) / M_PI;
            if (x < 0)
            {
                ang += 180;
            }
            else if (y < 0)
            {
                ang += 360;
            }
            assert(ang >= 0);
            assert(ang <= 360);
            return ang;
        }

        VertInf    *vInf;
        double     angle;
        double     distance;
};


typedef std::set<PointPair > VertSet;


class EdgePair
{
    public:
        EdgePair(const PointPair& p1, VertInf *v) : 
                vInf1(p1.vInf), 
                vInf2(v),
                dist1(p1.distance),
                dist2(dist(vInf2->point, centerPoint)),
                angle(p1.angle),
                angleDist(p1.distance)
        {
        }
        bool operator<(const EdgePair& rhs) const
        {
            assert(angle == rhs.angle);
            if (angleDist == rhs.angleDist)
            {
                return (dist2 < rhs.dist2);
            }
            return (angleDist < rhs.angleDist);
        }
        bool operator==(const EdgePair& rhs) const
        {
            if (((vInf1->id == rhs.vInf1->id) &&
                        (vInf2->id == rhs.vInf2->id)) ||
                ((vInf1->id == rhs.vInf2->id) &&
                        (vInf2->id == rhs.vInf1->id)))
            {
                return true;
            }
            return false;
        }
        bool operator!=(const EdgePair& rhs) const
        {
            if (((vInf1->id == rhs.vInf1->id) &&
                        (vInf2->id == rhs.vInf2->id)) ||
                ((vInf1->id == rhs.vInf2->id) &&
                        (vInf2->id == rhs.vInf1->id)))
            {
                return false;
            }
            return true;
        }
        double setCurrAngle(const PointPair& p)
        {
            if (p.vInf->point == vInf2->point)
            {
                angle = p.angle;
                angleDist = p.distance;
            }
            else if (angle != p.angle)
            {
                angle = p.angle;
                Point pp;
                int result = rayIntersectPoint(vInf1->point, vInf2->point,
                        centerPoint, p.vInf->point, &(pp.x), &(pp.y));
                if (result != DO_INTERSECT) 
                {
                    // Written this way to avoid a compiler warning for
                    // not using the result variable if NDEBUG is set.
                    assert(result == DO_INTERSECT);
                }
                angleDist = dist(pp, centerPoint);
            }

            return angleDist;
        }

        VertInf *vInf1;
        VertInf *vInf2;
        double dist1;
        double dist2;
        double angle;
        double angleDist;
};

typedef std::list<EdgePair> SweepEdgeList;


#define AHEAD    1
#define BEHIND  -1

class isBoundingShape
{
    public:
        // constructor remembers the value provided
        isBoundingShape(ShapeSet& set)
        : ss(set)
        { }
        // the following is an overloading of the function call operator
        bool operator () (const PointPair& pp)
        {
            if (pp.vInf->id.isShape &&
                    (ss.find(pp.vInf->id.objID) != ss.end()))
            {
                return true;
            }
            return false;
        }
    private:
        ShapeSet& ss;
};


static bool sweepVisible(SweepEdgeList& T, const PointPair& point, int *blocker)
{
    if (T.empty())
    {
        // No blocking edges.
        return true;
    }

    Router *router = point.vInf->_router;
    bool visible = true;

    SweepEdgeList::const_iterator closestIt = T.begin();
    SweepEdgeList::const_iterator end = T.end();
    if (! point.vInf->id.isShape)
    {
        // It's a connector endpoint, so we have to ignore 
        // edges of containing shapes for determining visibility.
        ShapeSet& rss = router->contains[point.vInf->id];
        while (closestIt != end)
        {
            if (rss.find(closestIt->vInf1->id.objID) == rss.end())
            {
                // This is not a containing edge so do the normal 
                // test and then stop.
                if (point.distance > closestIt->angleDist)
                {
                    visible =  false;
                }
                break;
            }
            // This was a containing edge, so consider the next along.
            ++closestIt;
        }
    }
    else
    {
        // Just test to see if this point is closer than the closest 
        // edge blocking this ray.
        if (point.distance > closestIt->angleDist)
        {
            visible =  false;
        }
    }

    if (!visible)
    {
        *blocker = (*closestIt).vInf1->id.objID;
#ifdef LINEDEBUG
        Point &e1 = (*closestIt).vInf1->point;
        Point &e2 = (*closestIt).vInf2->point;

        if (router->avoid_screen)
        {
            int canx = 151;
            int cany = 55;
            lineRGBA(router->avoid_screen, e1.x + canx, e1.y + cany,
                    e2.x + canx, e2.y + cany, 0, 0, 225, 255);
        }
#endif
    }
    return visible;
}


void vertexSweep(VertInf *vert)
{
    Router *router = vert->_router;
    VertID& pID = vert->id;
    Point& pPoint = vert->point;

    centerInf = vert;
    centerID = pID;
    centerPoint = pPoint;
    Point centerPt = pPoint;
    centerAngle = -1;

    // List of shape (and maybe endpt) vertices, except p
    // Sort list, around
    VertSet v;

    // Initialise the vertex list
    VertInf *beginVert = router->vertices.connsBegin();
    VertInf *endVert = router->vertices.end();
    for (VertInf *inf = beginVert; inf != endVert; inf = inf->lstNext)
    {
        if (inf->id == centerID)
        {
            // Don't include the center point
            continue;
        }

        std::pair<VertSet::iterator, bool> insertedVertex;
        insertedVertex.second = false;

        if (inf->id.isShape)
        {
            // Add shape vertex.
            insertedVertex = v.insert(inf);
        }
        else
        {
            if (router->IncludeEndpoints)
            {
                if (centerID.isShape)
                {
                    // Add endpoint vertex
                    insertedVertex = v.insert(inf);
                }
                else
                {
                    // Center is an endpoint, so only include the other
                    // endpoint from the matching connector.
                    VertID partnerID = VertID(centerID.objID, false,
                            (centerID.vn == 1) ? 2 : 1);
                    if (inf->id == partnerID)
                    {
                        insertedVertex = v.insert(inf);
                    }
                }
            }
        }
    }

    // And edges to T that intersect the initial ray.
    SweepEdgeList e;
    ShapeSet& ss = router->contains[centerID];
    VertSet::const_iterator vbegin = v.begin();
    VertSet::const_iterator vend = v.end();
    for (VertSet::const_iterator t = vbegin; t != vend; ++t)
    {
        VertInf *k = (*t).vInf;
        VertID kID = k->id;
        if (!(centerID.isShape) && (ss.find(kID.objID) != ss.end()))
        {
            unsigned int shapeID = kID.objID;
            db_printf("Center is inside shape %u so ignore shape edges.\n",
                    shapeID);
            continue;
        }

        if (centerInf == k)
        {
            continue;
        }

        Point xaxis(DBL_MAX, centerInf->point.y);

        VertInf *kPrev = k->shPrev;
        VertInf *kNext = k->shNext;
        if (kPrev && (kPrev != centerInf) && 
                (vecDir(centerInf->point, xaxis, kPrev->point) == BEHIND))
        {
            if (segmentIntersect(centerInf->point, xaxis, kPrev->point, 
                        k->point))
            {
                EdgePair intPair = EdgePair(*t, kPrev);
                e.push_back(intPair);
            }
        }
        else if (kNext && (kNext != centerInf) && 
                (vecDir(centerInf->point, xaxis, kNext->point) == BEHIND))
        {
            if (segmentIntersect(centerInf->point, xaxis, kNext->point, 
                        k->point))
            {
                EdgePair intPair = EdgePair(*t, kNext);
                e.push_back(intPair);
            }
        }
    }


    // Start the actual sweep.
    db_printf("SWEEP: "); centerID.db_print(); db_printf("\n");

    isBoundingShape isBounding(ss);
    for (VertSet::const_iterator t = vbegin; t != vend; ++t)
    {
        VertInf *currInf = (*t).vInf;
        VertID& currID = currInf->id;
        Point&  currPt = currInf->point;
        centerAngle = (*t).angle;

#ifdef LINEDEBUG
        Sint16 ppx = (int) centerPt.x;
        Sint16 ppy = (int) centerPt.y;

        Sint16 cx = (int) currPt.x;
        Sint16 cy = (int) currPt.y;

        int canx = 151;
        int cany = 55;
#endif

        const double& currDist = (*t).distance;

        EdgeInf *edge = EdgeInf::existingEdge(centerInf, currInf);
        if (edge == NULL)
        {
            edge = new EdgeInf(centerInf, currInf);
        }

        for (SweepEdgeList::iterator c = e.begin(); c != e.end(); ++c)
        {
            (*c).setCurrAngle(*t);
        }
        e.sort();

        // Check visibility.
        int blocker = 0;
        bool currVisible = sweepVisible(e, *t, &blocker);

        if (!(centerID.isShape) && isBounding(*t))
        {
            // Ignore vertices from bounding shapes, if sweeping 
            // round an endpoint.
            if (router->InvisibilityGrph)
            {
                // if p and t can't see each other, add blank edge
                db_printf("\tSkipping visibility edge... \n\t\t");
                edge->addBlocker(currInf->id.objID);
                edge->db_print();
            }
        }
        else
        {
            bool cone1 = true, cone2 = true;
            if (centerID.isShape)
            {
                cone1 = inValidRegion(router->IgnoreRegions,
                        centerInf->shPrev->point, centerPoint,
                        centerInf->shNext->point, currInf->point);
            }
            if (currInf->id.isShape)
            {
                cone2 = inValidRegion(router->IgnoreRegions,
                        currInf->shPrev->point, currInf->point,
                        currInf->shNext->point, centerPoint);
            }

            if (!cone1 || !cone2)
            {
                if (router->InvisibilityGrph)
                {
                    db_printf("\tSetting invisibility edge... \n\t\t");
                    edge->addBlocker(0);
                    edge->db_print();
                }
            }
            else
            {
                if (currVisible)
                {
#ifdef LINEDEBUG
                    if (router->avoid_screen)
                    {
                        lineRGBA(router->avoid_screen, ppx + canx, ppy + cany,
                                cx + canx, cy + cany, 255, 0, 0, 75);
                    }
#endif
                    db_printf("\tSetting visibility edge... \n\t\t");
                    edge->setDist(currDist);
                    edge->db_print();
                }
                else if (router->InvisibilityGrph)
                {
                    db_printf("\tSetting invisibility edge... \n\t\t");
                    edge->addBlocker(blocker);
                    edge->db_print();
                }
            }

            if (currID.isShape)
            {
                // This is a shape edge

                if (currInf->shPrev != centerInf)
                {
                    Point& prevPt = currInf->shPrev->point;
                    int prevDir = vecDir(centerPt, currPt, prevPt);
                    EdgePair prevPair = EdgePair(*t, currInf->shPrev);

                    if (prevDir == BEHIND)
                    {
                        e.remove(prevPair);
                    }
                    else if (prevDir == AHEAD)
                    {
                        e.push_front(prevPair);
                    }
                }

                if (currInf->shNext != centerInf)
                {
                    Point& nextPt = currInf->shNext->point;
                    int nextDir = vecDir(centerPt, currPt, nextPt);
                    EdgePair nextPair = EdgePair(*t, currInf->shNext);

                    if (nextDir == BEHIND)
                    {
                        e.remove(nextPair);
                    }
                    else if (nextDir == AHEAD)
                    {
                        e.push_front(nextPair);
                    }
                }
            }
        }
#ifdef LINEDEBUG
        if (router->avoid_screen)
        {
            SDL_Flip(router->avoid_screen);
        }
#endif
    }
}


}

