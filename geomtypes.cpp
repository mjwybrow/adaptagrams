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


#include <cmath>
#include <cstdlib>
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


ReferencingPolygon::ReferencingPolygon(const Polygon& poly, const Router *router)
    : PolygonInterface(),
      _id(poly._id),
      ps(poly.size())
{
    assert(router != NULL);
    for (int i = 0; i < poly.size(); ++i)
    {
        const Polygon *polyPtr = NULL;
        for (ShapeRefList::const_iterator sh = router->shapeRefs.begin();
                sh != router->shapeRefs.end(); ++sh) 
        {
            if ((*sh)->id() == poly.ps[i].id)
            {
                const Polygon& poly = (*sh)->poly();
                polyPtr = &poly;
                break;
            }
        }
        assert(polyPtr != NULL);
        ps[i] = std::make_pair(polyPtr, poly.ps[i].vn);
    }
}


ReferencingPolygon::ReferencingPolygon()
    : PolygonInterface()
{
    clear();
}


void ReferencingPolygon::clear(void)
{
    ps.clear();
}


const bool ReferencingPolygon::empty(void) const
{
    return ps.empty();
}


const int ReferencingPolygon::size(void) const
{
    return ps.size();
}


int ReferencingPolygon::id(void) const
{
    return _id;
}


const Point& ReferencingPolygon::at(int index) const 
{
    assert(index < size());
    const Polygon& poly = *(ps[index].first);
    unsigned short poly_index = ps[index].second;
    assert(poly_index < poly.size());

    return poly.ps[poly_index];
}


Polygon::Polygon()
    : PolygonInterface()
{
    clear();
}


Polygon::Polygon(const int pn)
    : PolygonInterface(),
      ps(pn)
{
}


Polygon::Polygon(const PolygonInterface& poly)
    : PolygonInterface(),
      _id(poly.id()),
      ps(poly.size())
{
    for (int i = 0; i < poly.size(); ++i)
    {
        ps[i] = poly.at(i);
    }
}


void Polygon::clear(void)
{
    ps.clear();
    ts.clear();
}


const bool Polygon::empty(void) const
{
    return ps.empty();
}


const int Polygon::size(void) const
{
    return ps.size();
}


int Polygon::id(void) const
{
    return _id;
}


const Point& Polygon::at(int index) const
{
    assert(index < size());

    return ps[index];
}


static const unsigned int SHORTEN_NONE  = 0;
static const unsigned int SHORTEN_START = 1;
static const unsigned int SHORTEN_END   = 2;
static const unsigned int SHORTEN_BOTH  = SHORTEN_START | SHORTEN_END;

// shorten_line():
//     Given the two endpoints of a line segment, this function adjusts the
//     endpoints of the line to shorten the line by shorten_length at either
//     or both ends.
//
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

    // Handle case where shorten length is greater than the length of the
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
    
    int xpos = (x1 < x2) ? -1 : 1;
    int ypos = (y1 < y2) ? -1 : 1;
    
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


// curvedPolyline():
//     Returns a curved approximation of this multi-segment PolyLine, with 
//     the corners replaced by smooth Bezier curves.  curve_amount describes
//     how large to make the curves.
//     The ts value for each point in the returned Polygon describes the 
//     drawing operation: 'M' (move) marks the first point, a line segment 
//     is marked with an 'L' and three 'C's (along with the previous point) 
//     describe the control points of a Bezier curve.
//
Polygon Polygon::curvedPolyline(const double curve_amount) const
{
    Polygon curved;

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
