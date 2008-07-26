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


typedef Point Vector;


class PolygnInterface
{
    public:
        PolygnInterface() { }
        virtual ~PolygnInterface() { }
        virtual void clear(void) = 0;
        virtual const bool empty(void) const = 0;
        virtual const int size(void) const = 0;
        virtual int id(void) const = 0;
        virtual const Point& at(int index) const = 0;
};


class Polygn : public PolygnInterface
{
    public:
        Polygn();
        void clear(void);
        const bool empty(void) const;
        const int size(void) const;
        int id(void) const;
        const Point& at(int index) const;

        int _id;
        Point *ps;
        int pn;
};

typedef Polygn PolyLine;


typedef struct
{
    Point a;
    Point b;
} Edge;

typedef Edge BBox;


class Router;

// A Polygn which just references its points from other Polygns.
// This is used to represent cluster boundaries made up of shape corners.
//
class ReferencingPolygn : public PolygnInterface
{
    public:
        ReferencingPolygn();
        ReferencingPolygn(const Polygn& poly, const Router *router);
        void clear(void);
        const bool empty(void) const;
        const int size(void) const;
        int id(void) const;
        const Point& at(int index) const;

        int _id;
        std::vector<std::pair<const Polygn *, unsigned short> > ps;
};


// A dynamic version of Polygn, to which points can be easily added 
// and removed.
//
class DynamicPolygn : public PolygnInterface
{
    public:
        DynamicPolygn();
        DynamicPolygn(const Polygn& poly);
        DynamicPolygn(ReferencingPolygn& poly);
        void clear(void);
        const bool empty(void) const;
        const int size(void) const;
        int id(void) const;
        const Point& at(int index) const;

        int _id;
        std::vector<Point> ps;
};


}

#endif
