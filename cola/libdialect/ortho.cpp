/*
 * vim: ts=4 sw=4 et tw=0 wm=0
 *
 * libdialect - A library for computing DiAlEcT layouts:
 *                 D = Decompose/Distribute
 *                 A = Arrange
 *                 E = Expand/Emend
 *                 T = Transform
 *
 * Copyright (C) 2018  Monash University
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
 * Author(s):   Steve Kieffer   <http://skieffer.info>
*/

#include <vector>
#include <set>
#include <math.h>
#include <memory>
#include <string>
#include <functional>
#include <stdexcept>

#include "libvpsc/assertions.h"
#include "libvpsc/rectangle.h"
#include "libavoid/libavoid.h"

#include "libdialect/graphs.h"
#include "libdialect/ortho.h"

using namespace dialect;

using std::vector;
using std::set;
using std::map;
using std::max;
using std::min;

using vpsc::Dim;
using Avoid::Point;


CompassDir Compass::compassDirection(Avoid::Point p0, Avoid::Point p1) {
    double dx = p1.x - p0.x,
           dy = p1.y - p0.y;
    if (dx == 0 && dy == 0) throw std::runtime_error("No compass direction between coincident points.");
    if (dx == 0) {
        return dy > 0 ? CompassDir::SOUTH : CompassDir::NORTH;
    }
    if (dy == 0) {
        return dx > 0 ? CompassDir::EAST : CompassDir::WEST;
    }
    if (dx > 0) {
        return dy > 0 ? CompassDir::SE : CompassDir::NE;
    } else {
        return dy > 0 ? CompassDir::SW : CompassDir::NW;
    }
}

CompassDir Compass::compassDirection(std::shared_ptr<Node> u0, std::shared_ptr<Node> u1) {
    Point p0 = u0->getCentre(),
          p1 = u1->getCentre();
    return Compass::compassDirection(p0, p1);
}

CardinalDir Compass::cardinalDirection(Avoid::Point p0, Avoid::Point p1) {
    double dx = p1.x - p0.x,
           dy = p1.y - p0.y;
    if (fabs(dy) <= fabs(dx)) {
        return dx > 0 ? CardinalDir::EAST : CardinalDir::WEST;
    } else {
        return dy > 0 ? CardinalDir::SOUTH : CardinalDir::NORTH;
    }
}

CardinalDir Compass::cardinalDirection(std::shared_ptr<Node> u0, std::shared_ptr<Node> u1) {
    Point p0 = u0->getCentre(),
          p1 = u1->getCentre();
    return Compass::cardinalDirection(p0, p1);
}

CardinalDirs Compass::cardinalComponents(CompassDir d) {
    switch(d) {
    case CompassDir::SE: return {CardinalDir::SOUTH, CardinalDir::EAST};
    case CompassDir::SW: return {CardinalDir::SOUTH, CardinalDir::WEST};
    case CompassDir::NW: return {CardinalDir::NORTH, CardinalDir::WEST};
    case CompassDir::NE: return {CardinalDir::NORTH, CardinalDir::EAST};
    default:
        // If the direction is already cardinal, then just return a vector containing
        // only this one direction.
        return {(CardinalDir) d};
    }
}

PlaneMap Compass::getRotationFunction(CardinalDir fromDir, CardinalDir toDir) {
    int a = (int) fromDir,
        b = (int) toDir,
        d = b - a;
    unsigned n = (unsigned)(d < 0 ? d + 4 : d);
    switch(n) {
    case 0:
        return [](const Point &p)->Point{return p;};
    case 1:
        return [](const Point &p)->Point{return Point(-p.y, p.x);};
    case 2:
        return [](const Point &p)->Point{return Point(-p.x, -p.y);};
    case 3:
        return [](const Point &p)->Point{return Point(p.y, -p.x);};
    default:
        // Cannot happen.
        COLA_ASSERT(false);
    }
}

InplacePlaneMap Compass::getInplaceRotationFunction(CardinalDir fromDir, CardinalDir toDir) {
    int a = (int) fromDir,
        b = (int) toDir,
        d = b - a;
    unsigned n = (unsigned)(d < 0 ? d + 4 : d);
    switch(n) {
    case 0:
        return [](Point &p)->void{COLA_UNUSED(p);};
    case 1:
        return [](Point &p)->void{double t = p.x; p.x = -p.y; p.y = t;};
    case 2:
        return [](Point &p)->void{p.x = -p.x; p.y = -p.y;};
    case 3:
        return [](Point &p)->void{double t = p.x; p.x = p.y; p.y = -t;};
    default:
        // Cannot happen.
        COLA_ASSERT(false);
    }
}

Point Compass::vectorSigns(CompassDir d) {
    switch(d) {
    case CompassDir::EAST:  return Point( 1,  0);
    case CompassDir::SE:    return Point( 1,  1);
    case CompassDir::SOUTH: return Point( 0,  1);
    case CompassDir::SW:    return Point(-1,  1);
    case CompassDir::WEST:  return Point(-1,  0);
    case CompassDir::NW:    return Point(-1, -1);
    case CompassDir::NORTH: return Point( 0, -1);
    case CompassDir::NE:    return Point( 1, -1);
    default:
        COLA_ASSERT(false);
    }
}

const CardinalDirs Compass::cwCards{
    CardinalDir::EAST, CardinalDir::SOUTH, CardinalDir::WEST, CardinalDir::NORTH
};

const CardinalDirs Compass::acwCards{
    CardinalDir::EAST, CardinalDir::NORTH, CardinalDir::WEST, CardinalDir::SOUTH
};

const CompassDirs Compass::cwAllDoubled{
    CompassDir::EAST, CompassDir::SE, CompassDir::SOUTH, CompassDir::SW,
    CompassDir::WEST, CompassDir::NW, CompassDir::NORTH, CompassDir::NE,
    CompassDir::EAST, CompassDir::SE, CompassDir::SOUTH, CompassDir::SW,
    CompassDir::WEST, CompassDir::NW, CompassDir::NORTH, CompassDir::NE
};

const CompassDirs Compass::acwAllDoubled{
    CompassDir::EAST, CompassDir::NE, CompassDir::NORTH, CompassDir::NW,
    CompassDir::WEST, CompassDir::SW, CompassDir::SOUTH, CompassDir::SE,
    CompassDir::EAST, CompassDir::NE, CompassDir::NORTH, CompassDir::NW,
    CompassDir::WEST, CompassDir::SW, CompassDir::SOUTH, CompassDir::SE
};

const set<CardinalDir> Compass::vertical{
    CardinalDir::NORTH, CardinalDir::SOUTH
};

const set<CardinalDir> Compass::horizontal{
    CardinalDir::EAST, CardinalDir::WEST
};

const map<CardinalDir, Avoid::ConnDirFlag> Compass::libavoidConnDirs{
    {CardinalDir::EAST,  Avoid::ConnDirRight},
    {CardinalDir::SOUTH, Avoid::ConnDirDown},
    {CardinalDir::WEST,  Avoid::ConnDirLeft},
    {CardinalDir::NORTH, Avoid::ConnDirUp}
};

const map<CompassDir, CompassDir> Compass::rotateCw90{
    {CompassDir::EAST,  CompassDir::SOUTH},
    {CompassDir::SE,    CompassDir::SW},
    {CompassDir::SOUTH, CompassDir::WEST},
    {CompassDir::SW,    CompassDir::NW},
    {CompassDir::WEST,  CompassDir::NORTH},
    {CompassDir::NW,    CompassDir::NE},
    {CompassDir::NORTH, CompassDir::EAST},
    {CompassDir::NE,    CompassDir::SE}
};

const map<CompassDir, CompassDir> Compass::flip{
    {CompassDir::EAST,  CompassDir::WEST},
    {CompassDir::SE,    CompassDir::NW},
    {CompassDir::SOUTH, CompassDir::NORTH},
    {CompassDir::SW,    CompassDir::NE},
    {CompassDir::WEST,  CompassDir::EAST},
    {CompassDir::NW,    CompassDir::SE},
    {CompassDir::NORTH, CompassDir::SOUTH},
    {CompassDir::NE,    CompassDir::SW}
};

const map<CardinalDir, Dim> Compass::varDim{
    {CardinalDir::EAST,  Dim::XDIM},
    {CardinalDir::WEST,  Dim::XDIM},
    {CardinalDir::SOUTH, Dim::YDIM},
    {CardinalDir::NORTH, Dim::YDIM}
};

const map<CardinalDir, Dim> Compass::constDim{
    {CardinalDir::EAST,  Dim::YDIM},
    {CardinalDir::WEST,  Dim::YDIM},
    {CardinalDir::SOUTH, Dim::XDIM},
    {CardinalDir::NORTH, Dim::XDIM}
};

CardinalDir Compass::cardRotateCw90(CardinalDir d) {
    return (CardinalDir) Compass::rotateCw90.at( (CompassDir) d );
}

CardinalDir Compass::cardRotateAcw90(CardinalDir d) {
    return (CardinalDir) Compass::rotateCw90.at( Compass::flip.at( (CompassDir) d ) );
}

CardinalDir Compass::cardFlip(CardinalDir d) {
    return (CardinalDir) Compass::flip.at( (CompassDir) d );
}

std::string Compass::dirToString(CompassDir d) {
    switch(d) {
    case CompassDir::EAST: return "EAST";
    case CompassDir::SOUTH: return "SOUTH";
    case CompassDir::WEST: return "WEST";
    case CompassDir::NORTH: return "NORTH";
    case CompassDir::SE: return "SE";
    case CompassDir::SW: return "SW";
    case CompassDir::NW: return "NW";
    case CompassDir::NE: return "NE";
    default :
        COLA_ASSERT(false);
    }
}

std::string Compass::cardToString(CardinalDir d) {
    switch(d) {
    case CardinalDir::EAST: return "EAST";
    case CardinalDir::SOUTH: return "SOUTH";
    case CardinalDir::WEST: return "WEST";
    case CardinalDir::NORTH: return "NORTH";
    default:
        COLA_ASSERT(false);
    }
}

const double LineSegment::EPSILON = 0.1;

bool LineSegment::closedIntervalIntersects(double a, double b) const {
    return wl <= b && a <= wh;
}

bool LineSegment::openIntervalIntersects(double a, double b) const {
    return b - wl > LineSegment::EPSILON && wh - a > LineSegment::EPSILON;
}

bool LineSegment::closedIntervalIncludesCoord(double a) const {
    return wl <= a && a <= wh;
}

bool LineSegment::openIntervalIncludesCoord(double a) const {
    return a - wl > LineSegment::EPSILON && wh - a > LineSegment::EPSILON;
}

interval LineSegment::closedIntervalIntersection(double a, double b) const {
    // If no intersection, return empty interval.
    if (!closedIntervalIntersects(a, b)) return {1, 0};
    // Otherwise there is some intersection.
    return {max(a, wl), min(b, wh)};
}

interval LineSegment::openIntervalIntersection(double a, double b) const {
    // If no intersection, return empty interval.
    if (!openIntervalIntersects(a, b)) return {1, 0};
    // Otherwise there is some intersection.
    return {max(a, wl), min(b, wh)};
}

int LineSegment::ptOnWhichSide(Avoid::Point p) const {
    double z1 = Compass::isVerticalCard(direc) ? p.x : p.y;
    return coordOnWhichSide(z1);
}

int LineSegment::coordOnWhichSide(double z1) const {
    return z1 < z ? -1 : (z1 > z ? 1 : 0);
}
