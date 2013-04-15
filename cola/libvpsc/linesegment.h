/*
 * vim: ts=4 sw=4 et tw=0 wm=0
 *
 * libvpsc - A solver for the problem of Variable Placement with 
 *           Separation Constraints.
 *
 * Copyright (C) 2005-2008  Monash University
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * See the file LICENSE.LGPL distributed with the library.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
*/

////////////////////////////////////////////////////////////////////////////////
// 
// 2D Line Segment Intersection example
// Implementation of the theory provided by Paul Bourke
// 
// Written by Damian Coventry
// Tuesday, 9 January 2007
// 
////////////////////////////////////////////////////////////////////////////////

#ifndef VPSC_LINESEGMENT_H
#define VPSC_LINESEGMENT_H

namespace linesegment {
class Vector
{
public:
    double x_, y_;

    Vector(double f = 0.0f)
        : x_(f), y_(f) {}

    Vector(double x, double y)
        : x_(x), y_(y) {}
};

class LineSegment
{
public:
    Vector begin_;
    Vector end_;

    LineSegment(const Vector& begin, const Vector& end)
        : begin_(begin), end_(end) {}

    enum IntersectResult { PARALLEL, COINCIDENT, NOT_INTERSECTING, INTERSECTING };

    IntersectResult Intersect(const LineSegment& other_line, Vector& intersection)
    {
        double dx1=end_.x_ - begin_.x_;
        double dy1=end_.y_ - begin_.y_;
        double dx2=other_line.end_.x_ - other_line.begin_.x_;
        double dy2=other_line.end_.y_ - other_line.begin_.y_;

        double denom = dy2 * dx1 - dy1 * dx2;

        double nume_a = dx2 * (begin_.y_ - other_line.begin_.y_) -
                        dy2 * (begin_.x_ - other_line.begin_.x_);

        double nume_b = dx1 * (begin_.y_ - other_line.begin_.y_) -
                        dy1 * (begin_.x_ - other_line.begin_.x_);

        if(denom == 0.0f)
        {
            if(nume_a == 0.0f && nume_b == 0.0f)
            {
                return COINCIDENT;
            }
            return PARALLEL;
        }

        double ua = nume_a / denom;
        double ub = nume_b / denom;

        if(ua >= 0.0f && ua <= 1.0f && ub >= 0.0f && ub <= 1.0f)
        {
            // Get the intersection point.
            intersection.x_ = begin_.x_ + ua*dx1;
            intersection.y_ = begin_.y_ + ua*dy1;

            return INTERSECTING;
        }

        return NOT_INTERSECTING;
    }
};

void DoLineSegmentIntersection(const Vector& p0, const Vector& p1, const Vector& p2, const Vector& p3)
{
    LineSegment linesegment0(p0, p1);
    LineSegment linesegment1(p2, p3);

    Vector intersection;

    std::cout << "Line Segment 0: (" << p0.x_ << ", " << p0.y_ << ") to (" << p1.x_ << ", " << p1.y_ << ")\n"
              << "Line Segment 1: (" << p2.x_ << ", " << p2.y_ << ") to (" << p3.x_ << ", " << p3.y_ << ")\n";

    switch(linesegment0.Intersect(linesegment1, intersection))
    {
    case LineSegment::PARALLEL:
        std::cout << "The lines are parallel\n\n";
        break;
    case LineSegment::COINCIDENT:
        std::cout << "The lines are coincident\n\n";
        break;
    case LineSegment::NOT_INTERSECTING:
        std::cout << "The lines do not intersect\n\n";
        break;
    case LineSegment::INTERSECTING:
        std::cout << "The lines intersect at (" << intersection.x_ << ", " << intersection.y_ << ")\n\n";
        break;
    }
}

int test()
{
    DoLineSegmentIntersection(Vector(0.0f, 0.0f), Vector(5.0f, 5.0f), Vector(5.0f, 0.0f), Vector(0.0f, 5.0f));
    DoLineSegmentIntersection(Vector(1.0f, 3.0f), Vector(9.0f, 3.0f), Vector(0.0f, 1.0f), Vector(2.0f, 1.0f));
    DoLineSegmentIntersection(Vector(1.0f, 5.0f), Vector(6.0f, 8.0f), Vector(0.5f, 3.0f), Vector(6.0f, 4.0f));
    DoLineSegmentIntersection(Vector(1.0f, 1.0f), Vector(3.0f, 8.0f), Vector(0.5f, 2.0f), Vector(4.0f, 7.0f));
    DoLineSegmentIntersection(Vector(1.0f, 2.0f), Vector(3.0f, 6.0f), Vector(2.0f, 4.0f), Vector(4.0f, 8.0f));
    DoLineSegmentIntersection(Vector(3.5f, 9.0f), Vector(3.5f, 0.5f), Vector(3.0f, 1.0f), Vector(9.0f, 1.0f));
    DoLineSegmentIntersection(Vector(2.0f, 3.0f), Vector(7.0f, 9.0f), Vector(1.0f, 2.0f), Vector(5.0f, 7.0f));
    return 0;
}
} // namespace linesegment

#endif // VPSC_LINESEGMENT_H
