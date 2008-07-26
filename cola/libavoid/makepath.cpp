/*
 * vim: ts=4 sw=4 et tw=0 wm=0
 *
 * libavoid - Fast, Incremental, Object-avoiding Line Router
 *
 * Copyright (C) 2004-2007  Michael Wybrow <mjwybrow@users.sourceforge.net>
 * Copyright (C) 2008  Monash University
 *
 * --------------------------------------------------------------------
 * The dijkstraPath function is based on code published and described
 * in "Algorithms in C" (Second Edition), 1990, by Robert Sedgewick.
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
 * License along with this library in the file LICENSE; if not, 
 * write to the Free Software Foundation, Inc., 59 Temple Place, 
 * Suite 330, Boston, MA  02111-1307  USA
 * 
*/

#include "libavoid/vertices.h"
#include "libavoid/makepath.h"
#include "libavoid/geometry.h"
#include "libavoid/connector.h"
#include "libavoid/graph.h"
#include "libavoid/router.h"
#include "libavoid/polyutil.h"
#include <algorithm>
#include <vector>
#include <limits.h>
#define _USE_MATH_DEFINES
#include <cmath>


namespace Avoid {


static double Dot(const Point& l, const Point& r)
{
    return (l.x * r.x) + (l.y * r.y);
}

static double CrossLength(const Point& l, const Point& r)
{
    return (l.x * r.y) - (l.y * r.x);
}


// Return the angle between the two line segments made by the
// points p1--p2 and p2--p3.  Return value is in radians.
//
static double angleBetween(const Point& p1, const Point& p2, const Point& p3)
{
    Point v1(p1.x - p2.x, p1.y - p2.y);
    Point v2(p3.x - p2.x, p3.y - p2.y);

    return fabs(atan2(CrossLength(v1, v2), Dot(v1, v2)));
}


// Construct a temporary Polygn path given several VertInf's for a connector.
//
static Polygn constructPolygnPath(VertInf *inf1, VertInf *inf2, VertInf *inf3)
{
    Polygn connRoute;
    int routeSize = 2;
    for (VertInf *curr = inf1; curr != NULL; curr = curr->pathNext)
    {
        routeSize += 1;
    }
    connRoute = newPoly(routeSize);
    connRoute.ps[routeSize - 1] = inf3->point;
    connRoute.ps[routeSize - 2] = inf2->point;
    routeSize -= 3;
    for (VertInf *curr = inf1; curr != NULL; curr = curr->pathNext)
    {
        connRoute.ps[routeSize] = curr->point;
        routeSize -= 1;
    }
    return connRoute;
}

// Given the two points for a new segment of a path (inf2 & inf3)
// as well as the distance between these points (dist), as well as
// possibly the previous point (inf1) [from inf1--inf2], return a
// cost associated with this route.
//
double cost(ConnRef *lineRef, const double dist, VertInf *inf1,
        VertInf *inf2, VertInf *inf3)
{
    double result = dist;
    Polygn connRoute;
    connRoute.ps = NULL;

    Router *router = inf2->_router;
    if (inf2->pathNext != NULL)
    {
        double& angle_penalty = router->angle_penalty;
        double& segmt_penalty = router->segmt_penalty;

        // This is not the first segment, so there is a bend
        // between it and the last one in the existing path.
        if ((angle_penalty > 0) || (segmt_penalty > 0))
        {
            Point p1 = inf1->point;
            Point p2 = inf2->point;
            Point p3 = inf3->point;

            double rad = M_PI - angleBetween(p1, p2, p3);

            // Make `xval' between 0--10 then take its log so small
            // angles are not penalised as much as large ones.
            //
            double xval = rad * 10 / M_PI;
            double yval = xval * log10(xval + 1) / 10.5;
            result += (angle_penalty * yval);
            //printf("deg from straight: %g\tpenalty: %g\n",
            //        rad * 180 / M_PI, (angle_penalty * yval));

            // Don't penalise as an extra segment if there is no turn.
            if (rad > 0.0005)
            {
                result += segmt_penalty;
            }
        }
    }

    if (! router->clusterRefs.empty() )
    {
        if (connRoute.ps == NULL)
        {
            connRoute = constructPolygnPath(inf1, inf2, inf3);
        }
        // There are clusters so do cluster routing.
        for (ClusterRefList::const_iterator cl = router->clusterRefs.begin(); 
                cl != router->clusterRefs.end(); ++cl)
        {
            ReferencingPolygn& cBoundary = (*cl)->poly();
            assert(cBoundary.ps[0] != cBoundary.ps[cBoundary.size() - 1]);
            for (int j = 0; j < cBoundary.size(); ++j)
            {
                // Cluster boundary points should correspond to shape 
                // vertices and hence already be in the list of vertices.
                assert(router->vertices.getVertexByPos(cBoundary.at(j))!=NULL);
            }
            
            bool isConn = false;
            DynamicPolygn dynamic_c_boundary(cBoundary);
            DynamicPolygn dynamic_conn_route(connRoute);
            int crossings = countRealCrossings(dynamic_c_boundary, isConn, 
                    dynamic_conn_route, connRoute.pn - 1, true);
            result += (crossings * router->cluster_crossing_penalty);
        }
    }

    if (lineRef->doesHateCrossings() && (router->crossing_penalty > 0))
    {
        if (connRoute.ps == NULL)
        {
            connRoute = constructPolygnPath(inf1, inf2, inf3);
        }
        ConnRefList::const_iterator curr, finish = router->connRefs.end();
        for (curr = router->connRefs.begin(); curr != finish; ++curr)
        {
            ConnRef *connRef = *curr;

            if (connRef->id() == lineRef->id())
            {
                continue;
            }
            const Avoid::PolyLine& route2 = connRef->route();
            
            bool isConn = true;
            DynamicPolygn dynamic_route2(route2);
            DynamicPolygn dynamic_conn_route(connRoute);
            int crossings = countRealCrossings(dynamic_route2, isConn, 
                    dynamic_conn_route, connRoute.pn - 1, true);
            result += (crossings * router->crossing_penalty);
        }
    }
    if (connRoute.ps != NULL)
    {
        freePoly(connRoute);
    }

    return result;
}


// Returns the best path from src to tar using the cost function.
//
// The path is worked out via Dijkstra's algorithm, and is encoded via
// pathNext links in each of the VerInfs along the path.
//
// Based on the code of 'matrixpfs'.
//
static void dijkstraPath(ConnRef *lineRef, VertInf *src, VertInf *tar)
{
    Router *router = src->_router;

    double unseen = (double) INT_MAX;

    // initialize arrays
    VertInf *finish = router->vertices.end();
    for (VertInf *t = router->vertices.connsBegin(); t != finish; t = t->lstNext)
    {
        t->pathNext = NULL;
        t->pathDist = -unseen;
    }

    VertInf *min = src;
    while (min != tar)
    {
        VertInf *k = min;
        min = NULL;

        k->pathDist *= -1;
        if (k->pathDist == unseen)
        {
            k->pathDist = 0;
        }

        EdgeInfList& visList = k->visList;
        EdgeInfList::const_iterator finish = visList.end();
        for (EdgeInfList::const_iterator edge = visList.begin(); edge != finish;
                ++edge)
        {
            VertInf *t = (*edge)->otherVert(k);
            VertID tID = t->id;

            // Only check shape verticies, or endpoints.
            if ((t->pathDist < 0) &&
                    ((tID.objID == src->id.objID) || tID.isShape))
            {
                double kt_dist = (*edge)->getDist();
                double priority = k->pathDist +
                        cost(lineRef, kt_dist, k->pathNext, k, t);

                if ((kt_dist != 0) && (t->pathDist < -priority))
                {
                    t->pathDist = -priority;
                    t->pathNext = k;
                }
                if ((min == NULL) || (t->pathDist > min->pathDist))
                {
                    min = t;
                }
            }
        }
        EdgeInfList& invisList = k->invisList;
        finish = invisList.end();
        for (EdgeInfList::const_iterator edge = invisList.begin(); 
                edge != finish; ++edge)
        {
            VertInf *t = (*edge)->otherVert(k);
            VertID tID = t->id;

            // Only check shape verticies, or endpoints.
            if ((t->pathDist < 0) &&
                    ((tID.objID == src->id.objID) || tID.isShape ))
            {
                if ((min == NULL) || (t->pathDist > min->pathDist))
                {
                    min = t;
                }
            }
        }
    }
}


class ANode
{
    public:
        VertInf* inf;
        double g;        // Gone
        double h;        // Heuristic
        double f;        // Formula f = g + h
        VertInf *pp;

        ANode(VertInf *vinf)
            : inf(vinf)
            , g(0)
            , h(0)
            , f(0)
            , pp(NULL)
        {
        }
        ANode()
            : inf(NULL)
            , g(0)
            , h(0)
            , f(0)
            , pp(NULL)
        {
        }
};


// This returns the opposite result so that when used with stl::make_heap, 
// the head node of the heap will be the smallest value, rather than the 
// largest.  This saves us from having to sort the heap (and then reorder
// it back into a heap) when getting the next node to examine.  This way we
// get better complexity -- logarithmic pushs and pops to the heap.
bool operator<(const ANode &a, const ANode &b)
{
    return a.f > b.f;
}


// Returns the best path from src to tar using the cost function.
//
// The path is worked out using the aStar algorithm, and is encoded via
// pathNext links in each of the VerInfs along the path.
//
// The aStar STL code is based on public domain code available on the
// internet.
//
static void aStarPath(ConnRef *lineRef, VertInf *src, VertInf *tar, 
        VertInf *start)
{
    std::vector<ANode> PENDING;     // STL Vectors chosen because of rapid
    std::vector<ANode> DONE;        // insertions/deletions at back,
    ANode Node, BestNode;           // Temporary Node and BestNode
    bool bNodeFound = false;        // Flag if node is found in container

    if (start == NULL)
    {
        start = src;
    }

    Router *router = lineRef->router();
    if (router->RubberBandRouting && (start != src))
    {
        assert(router->IgnoreRegions == true);
        
        const PolyLine& currRoute = lineRef->route();
        VertInf *last = NULL;
        int rIndx = 0;
        while (last != start)
        {
            Point& pnt = currRoute.ps[rIndx];
            bool isShape = (rIndx > 0);
            VertID vID(pnt.id, isShape, pnt.vn);

#ifdef PATHDEBUG
            printf("/// %d %d %d\n", pnt.id, (int) isShape, pnt.vn);
#endif
            VertInf *curr = router->vertices.getVertexByID(vID);
            assert(curr != NULL);

            Node = ANode(curr);
            if (!last)
            {
                Node.g = 0;
                Node.h = dist(Node.inf->point, tar->point);
                Node.f = Node.g + Node.h;
                Node.pp = NULL;
            }
            else
            {
                VertInf *prevInf = BestNode.inf;

                double edgeDist  = dist(BestNode.inf->point, curr->point);

                Node.g = BestNode.g + cost(lineRef, edgeDist, prevInf,
                        BestNode.inf, Node.inf);

                // Calculate the Heuristic.
                Node.h = dist(Node.inf->point, tar->point);

                // The A* formula
                Node.f = Node.g + Node.h;
                // Point parent to last BestNode (pushed onto DONE)
                Node.pp = BestNode.inf;
            }

            if (curr != start)
            {
                BestNode = Node;

                BestNode.inf->pathNext = BestNode.pp;
                DONE.push_back(BestNode);
            }
            else
            {
                PENDING.push_back(Node);
            }

            rIndx++;
            last = curr;
        }
    }
    else
    {
        // Create the start node
        Node = ANode(src);
        Node.g = 0;
        Node.h = dist(Node.inf->point, tar->point);
        Node.f = Node.g + Node.h;
        // Set a null parent, so cost function knows this is the first segment.
        Node.pp = NULL;

        // Populate the PENDING container with the first location
        PENDING.push_back(Node);
    }

    tar->pathNext = NULL;

    // Create a heap from PENDING for sorting
    using std::make_heap; using std::push_heap; using std::pop_heap;
    make_heap( PENDING.begin(), PENDING.end() );

    while (!PENDING.empty())
    {
        // Set the Node with lowest f value to BESTNODE.
        // Since the ANode operator< is reversed, the head of the
        // heap is the node with the lowest f value.
        BestNode = PENDING.front();

        // Pop off the heap.  Actually this moves the
        // far left value to the far right.  The node
        // is not actually removed since the pop is to
        // the heap and not the container.
        pop_heap(PENDING.begin(), PENDING.end());
        // Remove node from right (the value we pop_heap'd)
        PENDING.pop_back();

        // Push the BestNode onto DONE
        BestNode.inf->pathNext = BestNode.pp;
        DONE.push_back(BestNode);

#if 0
        printf("Considering... ");
        BestNode.ID->print(stdout);
        printf(" - g: %3.1f h: %3.1f f: %3.1f back: ", BestNode.g, BestNode.h,
                BestNode.f);
        BestNode.pp.print(stdout);
        printf("\n");
#endif

        // If at destination, break and create path below
        if (BestNode.inf == tar)
        {
#ifdef PATHDEBUG
            printf("Cost: %g\n", BestNode.f);
#endif
            //bPathFound = true; // arrived at destination...
            break;
        }

        // Check adjacent points in graph
        EdgeInfList& visList = BestNode.inf->visList;
        EdgeInfList::const_iterator finish = visList.end();
        for (EdgeInfList::const_iterator edge = visList.begin(); edge != finish;
                ++edge)
        {
            Node.inf = (*edge)->otherVert(BestNode.inf);

            // Only check shape verticies, or the tar endpoint.
            if (!(Node.inf->id.isShape) && (Node.inf != tar))
            {
                continue;
            }

            double edgeDist = (*edge)->getDist();

            if (edgeDist == 0)
            {
                continue;
            }

            VertInf *prevInf = BestNode.inf->pathNext;

            if ((!router->RubberBandRouting || (start == src)) && 
                  (validateBendPoint(prevInf, BestNode.inf, Node.inf) == false))
            {
                // The bendpoint is not valid, i.e., is a zigzag corner, so...
                continue;
                // For RubberBand routing we want to allow these routes and
                // unwind them later, otherwise instead or unwinding, paths
                // can go the *really* long way round.
            }

            Node.g = BestNode.g + cost(lineRef, edgeDist, prevInf,
                    BestNode.inf, Node.inf);

            // Calculate the Heuristic.
            Node.h = dist(Node.inf->point, tar->point);

            // The A* formula
            Node.f = Node.g + Node.h;
            // Point parent to last BestNode (pushed onto DONE)
            Node.pp = BestNode.inf;

            bNodeFound = false;

            // Check to see if already on PENDING
            for (unsigned int i = 0; i < PENDING.size(); i++)
            {
                ANode& ati = PENDING.at(i);
                if (Node.inf == ati.inf)
                {   // If already on PENDING
                    if (Node.g < ati.g)
                    {
                        ati.g = Node.g;
                        ati.f = Node.g + ati.h;
                        ati.pp = Node.pp;
                        
                        make_heap( PENDING.begin(), PENDING.end() );
                    }
                    bNodeFound = true;
                    break;
                }
            }
            if (!bNodeFound ) // If Node NOT found on PENDING
            {
                // Check to see if already on DONE
                for (unsigned int i = 0; i < DONE.size(); i++)
                {
                    ANode& ati = DONE.at(i);
                    if (Node.inf == ati.inf)
                    {
                        // If on DONE, Which has lower gone?
                        if (Node.g < ati.g)
                        {
                            ati.g = Node.g;
                            ati.f = Node.g + ati.h;
                            ati.pp = Node.pp;
                            ati.inf->pathNext = Node.pp;
                        }
                        bNodeFound = true;
                        break;
                    }
                }
            }

            if (!bNodeFound ) // If Node NOT found on PENDING or DONE
            {
                // Push NewNode onto PENDING
                PENDING.push_back(Node);
                // Push NewNode onto heap
                push_heap( PENDING.begin(), PENDING.end() );

#if 0
                using std::cout; using std::endl;
                // Display PENDING and DONE containers (For Debugging)
                cout << "PENDING:   ";
                for (unsigned int i = 0; i < PENDING.size(); i++)
                {
                    cout << PENDING.at(i).g << "," << PENDING.at(i).h << ",";
                    cout << PENDING.at(i).inf << "," << PENDING.at(i).pp << "  ";
                }
                cout << endl;
                cout << "DONE:   ";
                for (unsigned int i = 0; i < DONE.size(); i++)
                {
                    cout << DONE.at(i).g << "," << DONE.at(i).h << ",";
                    cout << DONE.at(i).inf << "," << DONE.at(i).pp << "  ";
                }
                cout << endl << endl;
#endif
            }
        }
    }
}


// Returns the best path for the connector referred to by lineRef.
//
// The path encoded in the pathNext links in each of the VerInfs
// backwards along the path, from the tar back to the source.
//
void makePath(ConnRef *lineRef, bool *flag)
{
    Router *router = lineRef->router();
    VertInf *src = lineRef->src();
    VertInf *tar = lineRef->dst();
    VertInf *start = lineRef->start();

    // If the connector hates crossings or there are cluster presnet,
    // then we want to examine direct paths:
    bool examineDirectPath = lineRef->doesHateCrossings() || 
            !(router->clusterRefs.empty());
    
    // TODO: Could be more efficient here.
    EdgeInf *directEdge = EdgeInf::existingEdge(src, tar);
    if ((start == src) && !(router->IncludeEndpoints) && directVis(src, tar))
    {
        Point p = src->point;
        Point q = tar->point;

        assert(directEdge == NULL);

        directEdge = new EdgeInf(src, tar);
        tar->pathNext = src;
        directEdge->setDist(dist(p, q));
        directEdge->addConn(flag);

        return;
    }
    else if ((start == src) && router->IncludeEndpoints && directEdge &&
            (directEdge->getDist() > 0) && !examineDirectPath)
    {
        tar->pathNext = src;
        directEdge->addConn(flag);
    }
    else
    {
        // Mark the path endpoints as not being able to see
        // each other.  This is true if we are here.
        if (!(router->IncludeEndpoints) && router->InvisibilityGrph &&
                !examineDirectPath)
        {
            if (!directEdge)
            {
                directEdge = new EdgeInf(src, tar);
            }
            directEdge->addBlocker(0);
        }

        if (router->UseAStarSearch)
        {
            aStarPath(lineRef, src, tar, start);
        }
        else
        {
            dijkstraPath(lineRef, src, tar);
        }

#if 0
        for (VertInf *t = vertices.connsBegin(); t != vertices.end();
                t = t->lstNext)
        {

            t->id.print();
            printf(" -> ");
            t->pathNext->id.print();
            printf("\n");
        }
#endif
    }
}


}


