/*
 * vim: ts=4 sw=4 et tw=0 wm=0
 *
 * libavoid - Fast, Incremental, Object-avoiding Line Router
 * Copyright (C) 2004-2006  Michael Wybrow <mjwybrow@users.sourceforge.net>
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
    

PolyLine& ConnRef::route(void)
{
    return _route;
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
        // Thus, there should always be an as-short path that just
        // skips this bend point.
        bendOkay = false;
    }
    else // (abc != 0)
    {
        int abe = vecDir(a, b, e);
        int abd = vecDir(a, b, d);
        int cbe = vecDir(c, b, e);
        int cbd = vecDir(c, b, d);
#ifdef PATHDEBUG
        printf("&& (abe == %d) && (abd == %d) &&\n(cbe == %d) && (cbd == %d)",
                abe, abd, cbe, cbd);
#endif

        if ((abe != 0) && (abe != abc))
        {
            bendOkay = false;
        }
        else if ((abd != 0) && (abd != abc))
        {
            bendOkay = false;
        }
        else if (abd == cbd)
        {
            bendOkay = false;
        }
        else if (abe == cbe)
        {
            bendOkay = false;
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
    PolyLine& currRoute = route();
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
        path[j].id = i->id.objID;
        path[j].vn = i->id.vn;
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
    path[0].id = _srcVert->id.objID;
    path[0].vn = _srcVert->id.vn;

    // Would clear visibility for endpoints here if required.

    freeRoute();
    PolyLine& output_route = route();
    output_route.pn = pathlen;
    output_route.ps = path;
 
    if ( !(_router->IncludeEndpoints) )
    {
        assert(_initialised);
        unInitialise();
    }
   
#ifdef PATHDEBUG
    printf("Output route:\n");
    for (int i = 0; i < output_route.pn; ++i)
    {
        printf("%g, %g  ", output_route.ps[i].x, output_route.ps[i].y);
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


//============================================================================

}


