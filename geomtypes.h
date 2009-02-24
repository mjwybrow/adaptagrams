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


#ifndef AVOID_GEOMTYPES_H
#define AVOID_GEOMTYPES_H

#include <vector>
#include <utility>


namespace Avoid
{

    
class Point
{
    public:
        Point();
        Point(const double xv, const double yv);
        bool operator==(const Point& rhs) const;
        bool operator!=(const Point& rhs) const;
        bool operator<(const Point& rhs) const;

        double x;
        double y;
        unsigned int id;
        unsigned short vn;

};

static const unsigned short kUnassignedVertexNumber = 8;


typedef Point Vector;


class PolygonInterface
{
    public:
        PolygonInterface() { }
        virtual ~PolygonInterface() { }
        virtual void clear(void) = 0;
        virtual const bool empty(void) const = 0;
        virtual const int size(void) const = 0;
        virtual int id(void) const = 0;
        virtual const Point& at(int index) const = 0;
};


typedef struct
{
    Point a;
    Point b;
} Edge;

typedef Edge BBox;


class Router;
class ReferencingPolygon;


// A dynamic Polygon, to which points can be easily added 
// and removed.
//
class Polygon : public PolygonInterface
{
    public:
        Polygon();
        Polygon(const int pn);
        Polygon(const PolygonInterface& poly);
        void clear(void);
        const bool empty(void) const;
        const int size(void) const;
        int id(void) const;
        const Point& at(int index) const;
        Polygon curvedPolyline(const double curve_amount) const;

        int _id;
        std::vector<Point> ps;
        // If used, denotes whether the corresponding point in ps is 
        // a move-to operation or a Bezier curve-to. 
        std::vector<char> ts;
};


typedef Polygon PolyLine;

// A Polygon which just references its points from other Polygons.
// This is used to represent cluster boundaries made up of shape corners.
//
class ReferencingPolygon : public PolygonInterface
{
    public:
        ReferencingPolygon();
        ReferencingPolygon(const Polygon& poly, const Router *router);
        void clear(void);
        const bool empty(void) const;
        const int size(void) const;
        int id(void) const;
        const Point& at(int index) const;

        int _id;
        std::vector<std::pair<const Polygon *, unsigned short> > ps;
};


}

#endif
