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


#include "libavoid/geomtypes.h"
#include "libavoid/shape.h"
#include "libavoid/router.h"

namespace Avoid
{

    
Point::Point() :
    id(0),
    vn(8)
{
}


Point::Point(const double xv, const double yv) :
    x(xv),
    y(yv),
    id(0),
    vn(8)
{
}


bool Point::operator==(const Point& rhs) const
{
    if ((x == rhs.x) && (y == rhs.y))
    {
        return true;
    }
    return false;
}


bool Point::operator!=(const Point& rhs) const
{
    if ((x != rhs.x) || (y != rhs.y))
    {
        return true;
    }
    return false;
}


// Just defined to allow std::set<Point>.  Not particularly meaningful!
bool Point::operator<(const Point& rhs) const
{
    if (x == rhs.x)
    {
        return (y < rhs.y);
    }
    return (x < rhs.x);
}


Polygn::Polygn()
    : PolygnInterface()
{
}


void Polygn::clear(void)
{
    if (ps != NULL)
    {
        std::free(ps);
        ps = NULL;
        pn = 0;
    }
}


const bool Polygn::empty(void) const
{
    return (pn == 0);
}


const int Polygn::size(void) const
{
    return pn;
}


int Polygn::id(void) const
{
    return _id;
}


const Point& Polygn::at(int index) const
{
    assert(index < size());
    
    return ps[index];
}


ReferencingPolygn::ReferencingPolygn(const Polygn& poly, const Router *router)
    : PolygnInterface(),
      _id(poly._id),
      ps(poly.pn)
{
    assert(router != NULL);
    for (int i = 0; i < poly.pn; ++i)
    {
        const Polygn *polyPtr = NULL;
        for (ShapeRefList::const_iterator sh = router->shapeRefs.begin();
                sh != router->shapeRefs.end(); ++sh) 
        {
            if ((*sh)->id() == poly.ps[i].id)
            {
                const Polygn& poly = (*sh)->poly();
                polyPtr = &poly;
                break;
            }
        }
        assert(polyPtr != NULL);
        ps[i] = std::make_pair(polyPtr, poly.ps[i].vn);
    }
}


ReferencingPolygn::ReferencingPolygn()
    : PolygnInterface()
{
    clear();
}


void ReferencingPolygn::clear(void)
{
    ps.clear();
}


const bool ReferencingPolygn::empty(void) const
{
    return ps.empty();
}


const int ReferencingPolygn::size(void) const
{
    return ps.size();
}


int ReferencingPolygn::id(void) const
{
    return _id;
}


const Point& ReferencingPolygn::at(int index) const 
{
    assert(index < size());
    const Polygn& poly = *(ps[index].first);
    unsigned short poly_index = ps[index].second;
    assert(poly_index < poly.pn);

    return poly.ps[poly_index];
}


DynamicPolygn::DynamicPolygn()
    : PolygnInterface()
{
    clear();
}


DynamicPolygn::DynamicPolygn(const Polygn& poly)
    : PolygnInterface(),
      _id(poly._id),
      ps(poly.pn)
{
    for (int i = 0; i < poly.pn; ++i)
    {
        ps[i] = poly.ps[i];
    }
}


DynamicPolygn::DynamicPolygn(ReferencingPolygn& poly)
    : PolygnInterface(),
      _id(poly._id),
      ps(poly.size())
{
    for (int i = 0; i < poly.size(); ++i)
    {
        ps[i] = poly.at(i);
    }
}


void DynamicPolygn::clear(void)
{
    ps.clear();
}


const bool DynamicPolygn::empty(void) const
{
    return ps.empty();
}


const int DynamicPolygn::size(void) const
{
    return ps.size();
}


int DynamicPolygn::id(void) const
{
    return _id;
}


const Point& DynamicPolygn::at(int index) const
{
    assert(index < size());

    return ps[index];
}


}

