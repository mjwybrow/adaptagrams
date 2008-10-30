/*
 * vim: ts=4 sw=4 et tw=0 wm=0
 *
 * libavoid - Fast, Incremental, Object-avoiding Line Router
 *
 * Copyright (C) 2004-2007  Michael Wybrow <mjwybrow@users.sourceforge.net>
 * Copyright (C) 2008  Monash University
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

#include <cstdlib>
#include <cstring>
#include "libavoid/graph.h"
#include "libavoid/connector.h"
#include "libavoid/makepath.h"
#include "libavoid/visibility.h"
#include "libavoid/debug.h"
#include "libavoid/router.h"


namespace Avoid {

    
ConnRef::ConnRef(Router *router, const unsigned int id)
    : _router(router)
    , _id(id)
    , _type(ConnType_PolyLine)
    , _srcId(0)
    , _dstId(0)
    , _needs_reroute_flag(true)
    , _false_path(false)
    , _active(false)
    , _route_dist(0)
    , _srcVert(NULL)
    , _dstVert(NULL)
    , _startVert(NULL)
    , _initialised(false)
    , _callback(NULL)
    , _connector(NULL)
    , _hateCrossings(false)
{
    assert(id > 0);

    // TODO: Store endpoints and details.
    _route.pn = 0;
    _route.ps = NULL;
}


ConnRef::ConnRef(Router *router, const unsigned int id,
        const Point& src, const Point& dst)
    : _router(router)
    , _id(id)
    , _type(ConnType_PolyLine)
    , _srcId(0)
    , _dstId(0)
    , _needs_reroute_flag(true)
    , _false_path(false)
    , _active(false)
    , _route_dist(0)
    , _srcVert(NULL)
    , _dstVert(NULL)
    , _initialised(false)
    , _callback(NULL)
    , _connector(NULL)
    , _hateCrossings(false)
{
    assert(id > 0);

    _route.pn = 0;
    _route.ps = NULL;

    if (_router->IncludeEndpoints)
    {
        bool isShape = false;
        _srcVert = new VertInf(_router, VertID(id, isShape, 1), src);
        _dstVert = new VertInf(_router, VertID(id, isShape, 2), dst);
        _router->vertices.addVertex(_srcVert);
        _router->vertices.addVertex(_dstVert);
        makeActive();
        _initialised = true;
    }
}


ConnRef::~ConnRef()
{
    freeRoute();

    if (_srcVert)
    {
        _router->vertices.removeVertex(_srcVert);
        delete _srcVert;
        _srcVert = NULL;
    }

    if (_dstVert)
    {
        _router->vertices.removeVertex(_dstVert);
        delete _dstVert;
        _dstVert = NULL;
    }

    if (_active)
    {
        makeInactive();
    }
}


void ConnRef::setType(unsigned int type)
{
    _type = type;
}


void ConnRef::updateEndPoint(const unsigned int type, const Point& point)
{
    //printf("updateEndPoint(%d,(pid=%d,vn=%d,(%f,%f)))\n",type,point.id,point.vn,point.x,point.y);
    assert((type == (unsigned int) VertID::src) ||
           (type == (unsigned int) VertID::tar));
    
    // XXX: This was commented out.  Is there a case where it isn't true? 
    assert(_router->IncludeEndpoints);

    if (!_initialised)
    {
        makeActive();
        _initialised = true;
    }
    
    VertInf *altered = NULL;
    VertInf *partner = NULL;
    bool isShape = false;

    if (type == (unsigned int) VertID::src)
    {
        if (_srcVert)
        {
            _srcVert->Reset(point);
        }
        else
        {
            _srcVert = new VertInf(_router, VertID(_id, isShape, type), point);
            _router->vertices.addVertex(_srcVert);
        }
        
        altered = _srcVert;
        partner = _dstVert;
    }
    else // if (type == (unsigned int) VertID::dst)
    {
        if (_dstVert)
        {
            _dstVert->Reset(point);
        }
        else
        {
            _dstVert = new VertInf(_router, VertID(_id, isShape, type), point);
            _router->vertices.addVertex(_dstVert);
        }
        
        altered = _dstVert;
        partner = _srcVert;
    }
    
    // XXX: Seems to be faster to just remove the edges and recreate
    bool isConn = true;
    altered->removeFromGraph(isConn);
    bool knownNew = true;
    bool genContains = true;
    vertexVisibility(altered, partner, knownNew, genContains);
}


bool ConnRef::updateEndPoint(const unsigned int type, const VertID& pointID,
        Point *pointSuggestion)
{
    VertInf *vInf = _router->vertices.getVertexByID(pointID);
    if (vInf == NULL)
    {
        return false;
    }
    Point& point = vInf->point;
    if (pointSuggestion)
    {
        if (dist(point, *pointSuggestion) > 0.5)
        {
            return false;
        }
    }

    //printf("updateEndPoint(%d,(pid=%d,vn=%d,(%f,%f)))\n",
    //        type, point.id, point.vn, point.x, point.y);
    assert((type == (unsigned int) VertID::src) ||
           (type == (unsigned int) VertID::tar));
    
    // XXX: This was commented out.  Is there a case where it isn't true? 
    assert(_router->IncludeEndpoints);

    if (!_initialised)
    {
        makeActive();
        _initialised = true;
    }
    
    VertInf *altered = NULL;
    VertInf *partner = NULL;
    bool isShape = false;

    if (type == (unsigned int) VertID::src)
    {
        if (_srcVert)
        {
            _srcVert->Reset(point);
        }
        else
        {
            _srcVert = new VertInf(_router, VertID(_id, isShape, type), point);
            _router->vertices.addVertex(_srcVert);
        }
        
        altered = _srcVert;
        partner = _dstVert;
    }
    else // if (type == (unsigned int) VertID::dst)
    {
        if (_dstVert)
        {
            _dstVert->Reset(point);
        }
        else
        {
            _dstVert = new VertInf(_router, VertID(_id, isShape, type), point);
            _router->vertices.addVertex(_dstVert);
        }
        
        altered = _dstVert;
        partner = _srcVert;
    }
    
    bool isConn = true;
    altered->removeFromGraph(isConn);

    // Give this visibility just to the point it is over.
    EdgeInf *edge = new EdgeInf(altered, vInf);
    // XXX: We should be able to set this to zero, but can't due to 
    //      assumptions elsewhere in the code.
    edge->setDist(0.001);

    return true;
}


void ConnRef::setEndPointId(const unsigned int type, const unsigned int id)
{
    if (type == (unsigned int) VertID::src)
    {
        _srcId = id;
    }
    else  // if (type == (unsigned int) VertID::dst)
    {
        _dstId = id;
    }
}


unsigned int ConnRef::getSrcShapeId(void)
{
    return _srcId;
}


unsigned int ConnRef::getDstShapeId(void)
{
    return _dstId;
}


void ConnRef::makeActive(void)
{
    assert(!_active);
    
    // Add to connRefs list.
    _pos = _router->connRefs.insert(_router->connRefs.begin(), this);
    _active = true;
}


void ConnRef::makeInactive(void)
{
    assert(_active);
    
    // Remove from connRefs list.
    _router->connRefs.erase(_pos);
    _active = false;
}


void ConnRef::freeRoute(void)
{
    if (_route.ps)
    {
        _route.pn = 0;
        std::free(_route.ps);
        _route.ps = NULL;
    }
}
    

const PolyLine& ConnRef::route(void)
{
    return _route;
}


void ConnRef::set_route(const PolyLine& route)
{
    if (&_route == &route)
    {
        fprintf(stderr, 
                "ERROR:\tTrying to update libavoid route with itself.\n");
        return;
    }

    if (_route.ps)
    {
        std::free(_route.ps);
        _route.ps = NULL;
    }
    int bytesize = route.pn * sizeof(Point);
    _route.pn = route.pn;
    _route.ps = (Point *) malloc(bytesize);
    memcpy(_route.ps, route.ps, bytesize);

    _display_route.clear();
}


DynamicPolygn& ConnRef::display_route(void)
{
    if (_display_route.empty())
    {
        _display_route = DynamicPolygn(_route);
    }
    return _display_route;
}


void ConnRef::calcRouteDist(void)
{
    _route_dist = 0;
    for (int i = 1; i < _route.pn; i++)
    {
        _route_dist += dist(_route.ps[i], _route.ps[i - 1]);
    }
}


bool ConnRef::needsReroute(void)
{
    return (_false_path || _needs_reroute_flag);
}


void ConnRef::lateSetup(const Point& src, const Point& dst)
{
    assert(!_initialised);

    bool isShape = false;
    _srcVert = new VertInf(_router, VertID(_id, isShape, 1), src);
    _dstVert = new VertInf(_router, VertID(_id, isShape, 2), dst);
    _router->vertices.addVertex(_srcVert);
    _router->vertices.addVertex(_dstVert);
    makeActive();
    _initialised = true;
}


unsigned int ConnRef::id(void)
{
    return _id;
}


VertInf *ConnRef::src(void)
{
    return _srcVert;
}

    
VertInf *ConnRef::dst(void)
{
    return _dstVert;
}


VertInf *ConnRef::start(void)
{
    return _startVert;
}


bool ConnRef::isInitialised(void)
{
    return _initialised;
}


void ConnRef::unInitialise(void)
{
    _router->vertices.removeVertex(_srcVert);
    _router->vertices.removeVertex(_dstVert);
    makeInactive();
    _initialised = false;
}


void ConnRef::removeFromGraph(void)
{
    for (VertInf *iter = _srcVert; iter != NULL; )
    {
        VertInf *tmp = iter;
        iter = (iter == _srcVert) ? _dstVert : NULL;
        
        // For each vertex.
        EdgeInfList& visList = tmp->visList;
        EdgeInfList::iterator finish = visList.end();
        EdgeInfList::iterator edge;
        while ((edge = visList.begin()) != finish)
        {
            // Remove each visibility edge
            delete (*edge);
        }

        EdgeInfList& invisList = tmp->invisList;
        finish = invisList.end();
        while ((edge = invisList.begin()) != finish)
        {
            // Remove each invisibility edge
            delete (*edge);
        }
    }
}


void ConnRef::setCallback(void (*cb)(void *), void *ptr)
{
    _callback = cb;
    _connector = ptr;
}


void ConnRef::handleInvalid(void)
{
    if (_false_path || _needs_reroute_flag) {
        if (_callback) {
            _callback(_connector);
        }
    }
}


void ConnRef::makePathInvalid(void)
{
    _needs_reroute_flag = true;
}


Router *ConnRef::router(void)
{
    return _router;
}


int ConnRef::generatePath(Point p0, Point p1)
{
    if (!_false_path && !_needs_reroute_flag) {
        // This connector is up to date.
        return (int) false;
    }

    if ( !(_router->IncludeEndpoints) )
    {
        lateSetup(p0, p1);
        
        bool knownNew = true;
        bool genContains = true;
        vertexVisibility(_srcVert, _dstVert, knownNew, genContains);
        vertexVisibility(_dstVert, _srcVert, knownNew, genContains);
    }

    return generatePath();
}


// Validates a bend point on a path to check it does not form a zigzag corner.
// a, b, c are consecutive points on the path.  d and e are b's neighbours,
// forming the shape corner d-b-e.
//
bool validateBendPoint(VertInf *aInf, VertInf *bInf, VertInf *cInf)
{
    bool bendOkay = true;

    if ((aInf == NULL) || (cInf == NULL))
    {
        // Not a bendpoint, i.e., the end of the connector, so don't test.
        return bendOkay;
    }

    assert(bInf != NULL);
    VertInf *dInf = bInf->shPrev;
    VertInf *eInf = bInf->shNext;
    assert(dInf != NULL);
    assert(eInf != NULL);

    Point& a = aInf->point;
    Point& b = bInf->point;
    Point& c = cInf->point;
    Point& d = dInf->point;
    Point& e = eInf->point;

    if ((a == b) || (b == c))
    {
        return bendOkay;
    }

#ifdef PATHDEBUG
    printf("a=(%g, %g)\n", a.x, a.y);
    printf("b=(%g, %g)\n", b.x, b.y);
    printf("c=(%g, %g)\n", c.x, c.y);
    printf("d=(%g, %g)\n", d.x, d.y);
    printf("e=(%g, %g)\n", e.x, e.y);
#endif
    // Check angle:
    int abc = vecDir(a, b, c);
#ifdef PATHDEBUG
    printf("(abc == %d) ", abc);
#endif
   
    if (abc == 0)
    {
        // The three consecutive point on the path are in a line.
        // Thus, there should always be an equally short path that
        // skips this bend point.
        bendOkay = false;
    }
    else // (abc != 0)
    {
        assert(vecDir(d, b, e) > 0);
        int abe = vecDir(a, b, e);
        int abd = vecDir(a, b, d);
        int bce = vecDir(b, c, e);
        int bcd = vecDir(b, c, d);
#ifdef PATHDEBUG
        printf("&& (abe == %d) && (abd == %d) &&\n(bce == %d) && (bcd == %d)",
                abe, abd, bce, bcd);
#endif

        bendOkay = false;
        if (abe > 0)
        {
            if ((abc > 0) && (abd >= 0) && (bce >= 0))
            {
                bendOkay = true;
            }
        }
        else if (abd < 0)
        {
            if ((abc < 0) && (abe <= 0) && (bcd <= 0))
            {
                bendOkay = true;
            }
        }
    }
#ifdef PATHDEBUG
    printf("\n");
#endif
    return bendOkay;
}


int ConnRef::generatePath(void)
{
    if (!_false_path && !_needs_reroute_flag) {
        // This connector is up to date.
        return (int) false;
    }

    //assert(_srcVert->point != _dstVert->point);

    _false_path = false;
    _needs_reroute_flag = false;

    VertInf *tar = _dstVert;
    _startVert = _srcVert;

    bool *flag = &(_needs_reroute_flag);
   
    int existingPathStart = 0;
    const PolyLine& currRoute = route();
    if (_router->RubberBandRouting)
    {
        assert(_router->IgnoreRegions == true);

#ifdef PATHDEBUG
        printf("\n");
        _srcVert->id.print(stdout);
        printf(": %g, %g\n", _srcVert->point.x, _srcVert->point.y);
        tar->id.print(stdout);
        printf(": %g, %g\n", tar->point.x, tar->point.y);
        for (int i = 0; i < currRoute.pn; ++i)
        {
            printf("%g, %g  ", currRoute.ps[i].x, currRoute.ps[i].y);
        }
        printf("\n");
#endif
        if (currRoute.pn > 2)
        {
            if (_srcVert->point == currRoute.ps[0])
            {
                existingPathStart = currRoute.pn - 2;
                assert(existingPathStart != 0);
                Point& pnt = currRoute.ps[existingPathStart];
                bool isShape = true;
                VertID vID(pnt.id, isShape, pnt.vn);

                _startVert = _router->vertices.getVertexByID(vID);
            }
        }
    }
    //printf("GO\n");
    //printf("src: %X strt: %X dst: %x\n", (int) _srcVert, (int) _startVert, (int) _dstVert);
    bool found = false;
    while (!found)
    {
        makePath(this, flag);
        for (VertInf *i = tar; i != NULL; i = i->pathNext)
        {
            if (i == _srcVert)
            {
                found = true;
                break;
            }
        }
        if (!found)
        {
            if (existingPathStart == 0)
            {
                break;
            }
#ifdef PATHDEBUG
            printf("BACK\n");
#endif
            existingPathStart--;
            Point& pnt = currRoute.ps[existingPathStart];
            bool isShape = (existingPathStart > 0);
            VertID vID(pnt.id, isShape, pnt.vn);

            _startVert = _router->vertices.getVertexByID(vID);
            assert(_startVert);
        }
        else if (_router->RubberBandRouting)
        {
            // found.
            bool unwind = false;

#ifdef PATHDEBUG
            printf("\n\n\nSTART:\n\n");
#endif
            VertInf *prior = NULL;
            for (VertInf *curr = tar; curr != _startVert->pathNext; 
                    curr = curr->pathNext)
            {
                if (!validateBendPoint(curr->pathNext, curr, prior))
                {
                    unwind = true;
                    break;
                }
                prior = curr;
            }
            if (unwind)
            {
#ifdef PATHDEBUG
                printf("BACK II\n");
#endif
                if (existingPathStart == 0)
                {
                    break;
                }
                existingPathStart--;
                Point& pnt = currRoute.ps[existingPathStart];
                bool isShape = (existingPathStart > 0);
                VertID vID(pnt.id, isShape, pnt.vn);

                _startVert = _router->vertices.getVertexByID(vID);
                assert(_startVert);

                found = false;
            }
        }
    }

    
    bool result = true;
    
    int pathlen = 1;
    for (VertInf *i = tar; i != _srcVert; i = i->pathNext)
    {
        pathlen++;
        if (i == NULL)
        {
            db_printf("Warning: Path not found...\n");
            pathlen = 2;
            tar->pathNext = _srcVert;
            if (_router->InvisibilityGrph)
            {
                // TODO:  Could we know this edge already?
                EdgeInf *edge = EdgeInf::existingEdge(_srcVert, tar);
                assert(edge != NULL);
                edge->addCycleBlocker();
            }
            result = false;
            break;
        }
        if (pathlen > 100)
        {
            fprintf(stderr, "ERROR: Should never be here...\n");
            exit(1);
        }
    }
    Point *path = (Point *) malloc(pathlen * sizeof(Point));

    int j = pathlen - 1;
    for (VertInf *i = tar; i != _srcVert; i = i->pathNext)
    {
        if (_router->InvisibilityGrph)
        {
            // TODO: Again, we could know this edge without searching.
            EdgeInf *edge = EdgeInf::existingEdge(i, i->pathNext);
            edge->addConn(flag);
        }
        else
        {
            _false_path = true;
        }
        path[j] = i->point;
        if (i->id.isShape)
        {
            path[j].id = i->id.objID;
            path[j].vn = i->id.vn;
        }
        else
        {
            path[j].id = _id;
            path[j].vn = kUnassignedVertexNumber;
        }
        j--;

        if (i->pathNext && (i->pathNext->point == i->point))
        {
            if (i->pathNext->id.isShape && i->id.isShape)
            {
                // Check for consecutive points on opposite 
                // corners of two touching shapes.
                assert(abs(i->pathNext->id.objID - i->id.objID) != 2);
            }
        }
    }
    path[0] = _srcVert->point;
    // Use topbit to differentiate between start and end point of connector.
    // They need unique IDs for nudging.
    unsigned int topbit = 1 << 31;
    path[0].id = _id | topbit; 
    path[0].vn = kUnassignedVertexNumber;

    // Would clear visibility for endpoints here if required.

    freeRoute();
    PolyLine& output_route = _route;
    output_route.pn = pathlen;
    output_route.ps = path;
    _display_route.clear();
 
    if ( !(_router->IncludeEndpoints) )
    {
        assert(_initialised);
        unInitialise();
    }
   
#ifdef PATHDEBUG
    printf("Output route:\n");
    for (int i = 0; i < output_route.pn; ++i)
    {
        printf("[%d,%d] %g, %g   ", output_route.ps[i].id, output_route.ps[i].vn,
                output_route.ps[i].x, output_route.ps[i].y);
    }
    printf("\n\n");
#endif

    return (int) result;
}


void ConnRef::setHateCrossings(bool value)
{
    _hateCrossings = value;
}


bool ConnRef::doesHateCrossings(void)
{
    return _hateCrossings;
}


PtOrder::~PtOrder()
{
    // Free the PointRep list.
    PointRepList::iterator curr = connList.begin();
    while (curr != connList.end())
    {
        PointRep *doomed = *curr;
        curr = connList.erase(curr);
        delete doomed;
    }
}

bool PointRep::follow_inner(PointRep *target)
{
    if (this == target)
    {
        return true;
    }
    else
    {
        for (PointRepSet::iterator curr = inner_set.begin(); 
                curr != inner_set.end(); ++curr)
        {
            if ((*curr)->follow_inner(target))
            {
                return true;
            }
        }
    }
    return false;
}


bool PtOrder::addPoints(Point *innerArg, Point *outerArg, bool swapped)
{
    Point *inner = (swapped) ? outerArg : innerArg;
    Point *outer = (swapped) ? innerArg : outerArg;
    assert( inner != outer);

    PointRep *innerPtr = NULL;
    PointRep *outerPtr = NULL;
    for (PointRepList::iterator curr = connList.begin(); 
            curr != connList.end(); ++curr)
    {
        if ((*curr)->point == inner)
        {
            innerPtr = *curr;
        }
        if ((*curr)->point == outer)
        {
            outerPtr = *curr;
        }
    }
    
    if (innerPtr == NULL)
    {
        innerPtr = new PointRep(inner);
        connList.push_back(innerPtr);
    }
    
    if (outerPtr == NULL)
    {
        outerPtr = new PointRep(outer);
        connList.push_back(outerPtr);
    }
    assert(innerPtr->inner_set.find(outerPtr) == innerPtr->inner_set.end());
    bool cycle = innerPtr->follow_inner(outerPtr);
    if (cycle)
    {
        // Must reverse to avoid a cycle.
        innerPtr->inner_set.insert(outerPtr);
    }
    else
    {
        outerPtr->inner_set.insert(innerPtr);
    }
    return cycle;
}


// Assuming that addPoints has been called for each pair of points in the 
// shared path at that corner, then the contents of inner_set can be used 
// to determine the correct ordering.
static bool pointRepLessThan(PointRep *r1, PointRep *r2)
{
    int r1less = r1->inner_set.size();
    int r2less = r2->inner_set.size();
    assert(r1less != r2less);

    return (r1less > r2less);
}


void PtOrder::sort(void)
{
    connList.sort(pointRepLessThan);
}


// Returns a vertex number representing a point on the line between 
// two shape corners, represented by p0 and p1.
//
static int midVertexNumber(const Point& p0, const Point& p1, const Point& c)
{
    if (c.vn != kUnassignedVertexNumber)
    {
        // The split point is a shape corner, so doesn't need its 
        // vertex number adjusting.
        return c.vn;
    }
    if ((p0.vn >= 4) && (p0.vn < kUnassignedVertexNumber))
    {
        // The point next to this has the correct nudging direction,
        // so use that.
        return p0.vn;
    }
    if ((p1.vn >= 4) && (p1.vn < kUnassignedVertexNumber))
    {
        // The point next to this has the correct nudging direction,
        // so use that.
        return p1.vn;
    }
    if ((p0.vn < 4) && (p1.vn < 4))
    {
        if (p0.vn != p1.vn)
        {
            return p0.vn;
        }
        // Splitting between two ordinary shape corners.
        int vn_mid = std::min(p0.vn, p1.vn);
        if ((std::max(p0.vn, p1.vn) == 3) && (vn_mid == 0))
        {
            vn_mid = 3; // Next vn is effectively 4.
        }
        return vn_mid + 4;
    }
    assert((p0.x == p1.x) || (p0.y == p1.y));
    if (p0.vn != kUnassignedVertexNumber)
    {
        if (p0.x == p1.x)
        {
            if ((p0.vn == 2) || (p0.vn == 3))
            {
                return 6;
            }
            return 4;
        }
        else
        {
            if ((p0.vn == 0) || (p0.vn == 3))
            {
                return 7;
            }
            return 5;
        }
    }
    else if (p1.vn != kUnassignedVertexNumber)
    {
        if (p0.x == p1.x)
        {
            if ((p1.vn == 2) || (p1.vn == 3))
            {
                return 6;
            }
            return 4;
        }
        else
        {
            if ((p1.vn == 0) || (p1.vn == 3))
            {
                return 7;
            }
            return 5;
        }
    }

    // Shouldn't both be new (kUnassignedVertexNumber) points.
    fprintf(stderr, "midVertexNumber(): p0.vn and p1.vn both = "
            "kUnassignedVertexNumber\n");
    fprintf(stderr, "p0.vn %d p1.vn %d\n", p0.vn, p1.vn);
    return kUnassignedVertexNumber;
}


// Break up overlapping parallel segments that are not the same edge in 
// the visibility graph, i.e., where one segment is a subsegment of another.
void splitBranchingSegments(Avoid::DynamicPolygn& poly, bool polyIsConn,
        Avoid::DynamicPolygn& conn)
{
    for (std::vector<Avoid::Point>::iterator i = conn.ps.begin(); 
            i != conn.ps.end(); ++i)
    {
        if (i == conn.ps.begin())
        {
            // Skip the first point.
            // There are points-1 segments in a connector.
            continue;
        }
        Point& c0 = *(i - 1);
        Point& c1 = *i;

        // XXX When doing the pointOnLine test we allow the points to be 
        // slightly non-collinear.  This addresses a problem with clustered
        // routing where connectors could otherwise route cheaply through
        // shape corners that were not quite on the cluster boundary, but
        // reported to be on there by the line segment intersection code,
        // which I suspect is not numerically accurate enough.  This occured
        // for points that only differed by about 10^-12 in the y-dimension.
        double tolerance = 0.00001;
        
        for (std::vector<Avoid::Point>::iterator j = poly.ps.begin(); 
                j != poly.ps.end(); )
        {
            if (polyIsConn && (j == poly.ps.begin()))
            {
                // Skip the first point.
                // There are points-1 segments in a connector.
                ++j;
                continue;
            }
            Point& p0 = (j == poly.ps.begin()) ? poly.ps.back() : *(j - 1);
            Point& p1 = *j;

            // Check the first point of the first segment.
            if (((i - 1) == conn.ps.begin()) && 
                    pointOnLine(p0, p1, c0, tolerance))
            {
                //printf("add to poly %g %g\n", c0.x, c0.y);
                
                c0.vn = midVertexNumber(p0, p1, c0);
                j = poly.ps.insert(j, c0);
                if (j != poly.ps.begin())
                {
                    --j;
                }
                continue;
            }
            // And the second point of every segment.
            if (pointOnLine(p0, p1, c1, tolerance))
            {
                //printf("add to poly %g %g\n", c1.x, c1.y);
                
                c1.vn = midVertexNumber(p0, p1, c1);
                j = poly.ps.insert(j, c1);
                if (j != poly.ps.begin())
                {
                    --j;
                }
                continue;
            }

            // Check the first point of the first segment.
            if (polyIsConn && ((j - 1) == poly.ps.begin()) && 
                        pointOnLine(c0, c1, p0, tolerance))
            {
                //printf("add to conn %g %g\n", p0.x, p0.y);

                p0.vn = midVertexNumber(c0, c1, p0);
                i = conn.ps.insert(i, p0);
                --i;
                break;
            }
            // And the second point of every segment.
            if (pointOnLine(c0, c1, p1, tolerance))
            {
                //printf("add to conn %g %g\n", p1.x, p1.y);

                p1.vn = midVertexNumber(c0, c1, p1);
                i = conn.ps.insert(i, p1);
                --i;
                break;
            }
            ++j;
        }
    }
}


// Works out if the segment conn[cIndex-1]--conn[cIndex] really crosses poly.
// This does not not count non-crossing shared paths as crossings.
// poly can be either a connector (polyIsConn = true) or a cluster
// boundary (polyIsConn = false).
//
int countRealCrossings(Avoid::DynamicPolygn& poly, bool polyIsConn,
        Avoid::DynamicPolygn& conn, int cIndex, bool checkForBranchingSegments,
        PointSet *crossingPoints, PtOrderMap *pointOrders, bool *touches)
{
    if (checkForBranchingSegments)
    {
        int conn_pn = conn.size();
        splitBranchingSegments(poly, polyIsConn, conn);
        // cIndex is going to be the last, so take into account added points.
        cIndex += (conn.size() - conn_pn);
    }
    assert(cIndex >= 1);

    int crossingCount = 0;

    Avoid::Point& a1 = conn.ps[cIndex - 1];
    Avoid::Point& a2 = conn.ps[cIndex];
    //printf("a1: %g %g\n", a1.x, a1.y);
    //printf("a2: %g %g\n", a2.x, a2.y);

    for (int j = ((polyIsConn) ? 1 : 0); j < poly.size(); ++j)
    {
        Avoid::Point& b1 = poly.ps[(j - 1 + poly.size()) % poly.size()];
        Avoid::Point& b2 = poly.ps[j];
        //printf("b1: %g %g\n", b1.x, b1.y);
        //printf("b2: %g %g\n", b2.x, b2.y);

        if (((a1 == b1) && (a2 == b2)) ||
            ((a2 == b1) && (a1 == b2)))
        {
            // Route along same segment: no penalty.  We detect
            // crossovers when we see the segments diverge.
            continue;
        }

        if ((a2 == b2) || (a2 == b1) || (b2 == a1))
        {
            // Each crossing that is at a vertex in the 
            // visibility graph gets noticed four times.
            // We ignore three of these cases.
            // This also catches the case of a shared path,
            // but this is one that terminates at a common
            // endpoint, so we don't care about it.
            continue;
        }
    
        if (a1 == b1)
        {
            if (polyIsConn && (j == 1))
            {
                // common source point.
                continue;
            }
            Avoid::Point& b0 = poly.ps[(j - 2 + poly.size()) % poly.size()];
            // The segments share an endpoint -- a1==b1.
            if (a2 == b0)
            {
                // a2 is not a split, continue.
                continue;
            }
            
            // If here, then we know that a2 != b2
            // And a2 and its pair in b are a split.
            assert(a2 != b2);

            if (cIndex < 2)
            {
                // There is no a0;
                continue;
            }
            Avoid::Point& a0 = conn.ps[cIndex - 2];
            
            //printf("a0: %g %g\n", a0.x, a0.y);
            //printf("b0: %g %g\n", b0.x, b0.y);

            if ( (a0 == b2) || (a0 == b0) )
            {
                //printf("Shared path... ");
                bool normal = (a0 == b2) ? false : true;
                // Determine direction we have to look through
                // the points of connector b.
                int dir = normal ? -1 : 1;
                
                int traceI = cIndex - 2;
                int traceJ = j - 1 + dir;
                
                int endCornerSide = Avoid::cornerSide(a0, a1, a2, 
                        normal ? b2 : b0);
                int prevTurnDir = vecDir(a0, a1, a2);
                bool reversed = (endCornerSide != -prevTurnDir);
                bool orderSwapped;
                if (pointOrders)
                {
                    VertID vID(a1.id, true, a1.vn);
                    orderSwapped = (*pointOrders)[vID].addPoints(
                            &b1, &a1, reversed);
                    if (orderSwapped)
                    {
                        // Reverse the order for later points.
                        reversed = !reversed;
                    }
                }
                while ((traceI >= 0) &&
                    (!polyIsConn || ((traceJ >= 0) && (traceJ < poly.size()))) )
                {
                    int indexJ = (traceJ + (2 * poly.size())) % poly.size();
                    assert(indexJ >= 0);
                    assert(indexJ < poly.size());
                    if (conn.ps[traceI] != poly.ps[indexJ])
                    {
                        // Points don't match, so break out of loop.
                        break;
                    }
                    if (pointOrders)
                    {
                        Avoid::Point& an = conn.ps[traceI];
                        Avoid::Point& bn = poly.ps[indexJ];
                        int currTurnDir = (traceI > 0) ?  
                                vecDir(conn.ps[traceI - 1], an,
                                       conn.ps[traceI + 1]) : 0;
                        VertID vID(an.id, true, an.vn);
                        if ( (currTurnDir == (-1 * prevTurnDir)) &&
                                (currTurnDir != 0) && (prevTurnDir != 0) )
                        {
                            // The connector turns the opposite way around 
                            // this shape as the previous bend on the path,
                            // so reverse the order so that the inner path
                            // become the outer path and vice verca.
                            reversed = !reversed;
                        }
                        orderSwapped = (*pointOrders)[vID].addPoints(
                                &bn, &an, reversed);
                        assert(!orderSwapped);
                        prevTurnDir = currTurnDir;
                    }
                    traceI--;
                    traceJ += dir;
                }
            
                if (touches)
                {
                    *touches = true;
                }

                if ((traceI < 0) || (polyIsConn && ((traceJ < 0) || 
                        (traceJ >= poly.size()))))
                {
                    //printf("common source or destination.\n");
                    // The connectors have a shared path, but it
                    // comes from a common source point.
                    // XXX: There might be a better way to
                    //      check this by asking the connectors
                    //      for the IDs of the attached shapes.
                    continue;
                }
               
                int startCornerSide = Avoid::cornerSide(
                        conn.ps[traceI], conn.ps[traceI + 1],
                        conn.ps[traceI + 2], 
                        poly.ps[(traceJ + poly.size()) % poly.size()]);
                            
                if (endCornerSide != startCornerSide)
                {
                    //printf("shared path crosses.\n");
                    crossingCount += 1;
                    if (crossingPoints)
                    {
                        crossingPoints->insert(a1);
                    }
                }
            }
            else
            {
                // The connectors cross or touch at this point.
                //printf("Cross or touch at point... \n");
            
                int side1 = Avoid::cornerSide(a0, a1, a2, b0);
                int side2 = Avoid::cornerSide(a0, a1, a2, b2);
                if (side1 != side2)
                {
                    // The connectors cross at this point.
                    //printf("cross.\n");
                    crossingCount += 1;
                    if (crossingPoints)
                    {
                        crossingPoints->insert(a1);
                    }
                }

                if (touches)
                {
                    *touches = true;
                }
                if (pointOrders)
                {
                    int turnDirA = vecDir(a0, a1, a2); 
                    int turnDirB = vecDir(b0, b1, b2); 
                    bool reversed = (side1 != -turnDirA); 
                    if (side1 != side2) 
                    { 
                        // Interesting case where a connector routes round the 
                        // edge of a shape and intersects a connector which is 
                        // connected to a port on the edge of the shape. 
                        if (turnDirA == 0) 
                        { 
                            // We'll make B the outer by preference,  
                            // because the points of A are collinear. 
                            reversed = false; 
                        } 
                        else if (turnDirB == 0) 
                        { 
                            reversed = true; 
                        } 
                        assert((turnDirB != 0) || (turnDirA != 0)); 
                    }
                    VertID vID(b1.id, true, b1.vn);
                    (*pointOrders)[vID].addPoints(&b1, &a1, reversed);
                }
            }
        }
        else
        {
            Point cPt;
            int intersectResult = Avoid::segmentIntersectPoint(
                    a1, a2, b1, b2, &(cPt.x), &(cPt.y));

            if (intersectResult == Avoid::DO_INTERSECT)
            {
                if ((a1 == cPt) || (a2 == cPt) || (b1 == cPt) || (b2 == cPt))
                {
                    // XXX: This shouldn't actually happen, because these
                    //      points should be added as bends to each line by
                    //      splitBranchingSegments().  Thus, lets ignore them.
                    assert(a1 != cPt);
                    assert(a2 != cPt);
                    assert(b1 != cPt);
                    assert(b2 != cPt);
                    return crossingCount;
                }                
                //printf("crossing lines:\n");
                //printf("cPt: %g %g\n", cPt.x, cPt.y);
                crossingCount += 1;
                if (crossingPoints)
                {
                    crossingPoints->insert(cPt);
                }
            }
        }
    }
    //printf("crossingcount %d\n", crossingCount);
    return crossingCount;
}


//============================================================================

}


