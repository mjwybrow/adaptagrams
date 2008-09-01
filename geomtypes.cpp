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


#include <cmath>

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
    ts.clear();
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


static const unsigned int SHORTEN_NONE  = 0;
static const unsigned int SHORTEN_START = 1;
static const unsigned int SHORTEN_END   = 2;
static const unsigned int SHORTEN_BOTH  = SHORTEN_START | SHORTEN_END;

static void shorten_line(double& x1, double& y1, double& x2, double& y2, 
        const unsigned int mode, const double shorten_length)
{
    if (mode == SHORTEN_NONE)
    {
        return;
    }
    
    double rise = y1 - y2;
    double run = x1 - x2;
    double disty = fabs(rise);
    double distx = fabs(run);

    // Handle case where shorten length is greater than that for the
    // line segment.
    if ((mode == SHORTEN_BOTH) &&
            (((distx > disty) && ((shorten_length * 2) > distx)) ||
             ((disty >= distx) && ((shorten_length * 2) > disty))))
    {
        x1 = x2 = x1 - (run / 2); 
        y1 = y2 = y1 - (rise / 2); 
        return;
    }
    else if ((mode == SHORTEN_START) && 
            (((distx > disty) && (shorten_length > distx)) ||
             ((disty >= distx) && (shorten_length > disty))))
    {
        x1 = x2;
        y1 = y2;
        return;
    }
    else if ((mode == SHORTEN_END) && 
            (((distx > disty) && (shorten_length > distx)) ||
             ((disty >= distx) && (shorten_length > disty))))
    {
        x2 = x1;
        y2 = y1;
        return;
    }

    // Handle orthogonal line segments.
    if (x1 == x2)
    {
        // Vertical
        int sign = (y1 < y2) ? 1: -1;
        
        if (mode & SHORTEN_START)
        {
            y1 += (sign * shorten_length);
        }
        if (mode & SHORTEN_END)
        {
            y2 -= (sign * shorten_length);
        }
        return;
    }
    else if (y1 == y2)
    {
        // Horizontal
        int sign = (x1 < x2) ? 1: -1;
        
        if (mode & SHORTEN_START)
        {
            x1 += (sign * shorten_length);
        }
        if (mode & SHORTEN_END)
        {
            x2 -= (sign * shorten_length);
        }
        return;
    }
    
    int xpos = 1;
    int ypos = 1;
    if (x1 < x2)
    {
        xpos = -1;
    }
    if (y1 < y2)
    {
        ypos = -1;
    }
    
    double tangent = rise / run;
   
    if (mode & SHORTEN_END)
    {
        if (disty > distx)
        {
            y2 += shorten_length * ypos;
            x2 += shorten_length * ypos * (1 / tangent);
        }
        else if (disty < distx)
        {
            y2 += shorten_length * xpos * tangent;
            x2 += shorten_length * xpos;
        }
    }

    if (mode & SHORTEN_START)
    {
        if (disty > distx)
        {
            y1 -= shorten_length * ypos;
            x1 -= shorten_length * ypos * (1 / tangent);
        }
        else if (disty < distx)
        {
            y1 -= shorten_length * xpos * tangent;
            x1 -= shorten_length * xpos;
        }
    }
}


#define mid(a, b) ((a < b) ? a + ((b - a) / 2) : b + ((a - b) / 2))

DynamicPolygn DynamicPolygn::curvedPolyline(const double curve_amount) const
{
    DynamicPolygn curved;

    int num_of_points = size();
    if (num_of_points <= 2)
    {
        // There is only a single segment, do nothing.
        curved = *this;
        curved.ts.push_back('M');
        curved.ts.push_back('L');
        return curved;
    }

    curved._id = _id;
    curved.ps.push_back(ps[0]);
    curved.ts.push_back('M');
   
    double last_x = 0;
    double last_y = 0;
    for (int j = 1; j < size(); ++j)
    {
        double x1 = ps[j - 1].x;
        double y1 = ps[j - 1].y;
        double x2 = ps[j].x;
        double y2 = ps[j].y;

        double old_x = x1;
        double old_y = y1;
        
        unsigned int mode = SHORTEN_BOTH;
        if (j == 1)
        {
            mode = SHORTEN_END;
        }
        else if (j == (size() - 1))
        {
            mode = SHORTEN_START;
        }
        shorten_line(x1, y1, x2, y2, mode, curve_amount);

        if (j > 1)
        {
            curved.ts.insert(curved.ts.end(), 3, 'C');
            curved.ps.push_back(Point(mid(last_x, old_x), mid(last_y, old_y)));
            curved.ps.push_back(Point(mid(x1, old_x), mid(y1, old_y)));
            curved.ps.push_back(Point(x1, y1));
        }
        curved.ts.push_back('L');
        curved.ps.push_back(Point(x2, y2));
            
        last_x = x2;
        last_y = y2;
    }
    
    return curved;
}


}

