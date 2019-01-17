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

#ifndef DIALECT_ORTHO_H
#define DIALECT_ORTHO_H

#include <vector>
#include <set>
#include <memory>
#include <map>
#include <string>
#include <functional>
#include <algorithm>
#include <utility>

#include "libvpsc/rectangle.h"
#include "libavoid/libavoid.h"

#include "libdialect/commontypes.h"

namespace dialect {

class Node;

enum class CompassDir {
    EAST,
    SOUTH,
    WEST,
    NORTH,
    SE,
    SW,
    NW,
    NE
};

enum class CardinalDir {
    EAST,
    SOUTH,
    WEST,
    NORTH
};

typedef std::vector<CompassDir> CompassDirs;
typedef std::vector<CardinalDir> CardinalDirs;

struct Compass {

    //! @brief  Check whether a compass direction is vertical.
    static bool isVertical(CompassDir d) {
        return d == CompassDir::NORTH || d == CompassDir::SOUTH;
    }

    //! @brief  Check whether a compass direction is horizontal.
    static bool isHorizontal(CompassDir d) {
        return d == CompassDir::EAST || d == CompassDir::WEST;
    }

    //! @brief  Check whether a cardinal direction is vertical.
    static bool isVerticalCard(CardinalDir d) {
        return d == CardinalDir::NORTH || d == CardinalDir::SOUTH;
    }

    //! @brief  Check whether a cardinal direction is vertical.
    static bool isHorizontalCard(CardinalDir d) {
        return d == CardinalDir::EAST || d == CardinalDir::WEST;
    }

    //! @brief  Check whether a compass direction is in a given dimension.
    static bool isInDim(CompassDir d, vpsc::Dim dim) {
        return dim == vpsc::HORIZONTAL ? isHorizontal(d) : isVertical(d);
    }

    //! @brief  Check whether a compass direction is "increasing", i.e. the
    //!         variable coordinate increases in this direction.
    static bool isIncreasing(CompassDir d) {
        return d == CompassDir::EAST || d == CompassDir::SOUTH;
    }

    //! @brief  Check whether a compass direction is "decreasing", i.e. the
    //!         variable coordinate decreases in this direction.
    static bool isDecreasing(CompassDir d) {
        return d == CompassDir::NORTH || d == CompassDir::WEST;
    }

    //! @brief  Check whether a cardinal direction is "increasing", i.e. the
    //!         variable coordinate increases in this direction.
    static bool isIncreasingCard(CardinalDir d) {
        return d == CardinalDir::EAST || d == CardinalDir::SOUTH;
    }

    //! @brief  Check whether a cardinal direction is "decreasing", i.e. the
    //!         variable coordinate decreases in this direction.
    static bool isDecreasingCard(CardinalDir d) {
        return d == CardinalDir::WEST || d == CardinalDir::NORTH;
    }

    //! @brief  Check whether two cardinal directions are in the same dimension.
    //! @sa Compass::arePerpendicular
    static bool sameDimension(CardinalDir d0, CardinalDir d1) {
        return (unsigned)(d0) % 2 == (unsigned)(d1) % 2;
    }

    //! @brief  Check whether two cardinal directions are perpendicular.
    //! @sa Compass::sameDimension
    static bool arePerpendicular(CardinalDir d0, CardinalDir d1) {
        return (unsigned)(d0) % 2 != (unsigned)(d1) % 2;
    }

    //! @brief  Determine the predominant compass direction from point
    //!         p0 to point p1.
    static CompassDir compassDirection(Avoid::Point p0, Avoid::Point p1);

    //! @brief  Convenience method to determine the compass direction
    //!         from Node u0 to Node u1.
    static CompassDir compassDirection(std::shared_ptr<Node> u0, std::shared_ptr<Node> u1);

    //! @brief  Determine the predominant cardinal direction from point
    //!         p0 to point p1.
    static CardinalDir cardinalDirection(Avoid::Point p0, Avoid::Point p1);

    //! @brief  Convenience method to determine the cardinal direction
    //!         from Node u0 to Node u1.
    static CardinalDir cardinalDirection(std::shared_ptr<Node> u0, std::shared_ptr<Node> u1);

    //! @brief  Say whether a CompassDir is cardinal.
    static bool isCardinal(CompassDir d) { return ((unsigned) d) < 4; }

    //! @brief Rotate a cardinal direction clockwise 90 degrees.
    static CardinalDir cardRotateCw90(CardinalDir d);

    //! @brief Rotate a cardinal direction anticlockwise 90 degrees.
    static CardinalDir cardRotateAcw90(CardinalDir d);

    //! @brief Flip a cardinal direction.
    static CardinalDir cardFlip(CardinalDir d);

    //! @brief Write the name of a CompassDir.
    static std::string dirToString(CompassDir d);

    //! @brief Write the name of a CardinalDir.
    static std::string cardToString(CardinalDir d);

    //! @brief Get the cardinal components of a given direction.
    static CardinalDirs cardinalComponents(CompassDir d);

    //! @brief Get a function that rotates points by the angular displacement
    //!        from one cardinal direction to another.
    //! @param[in] fromDir  The starting cardinal direction.
    //! @param[in] toDir  The ending cardinal direction.
    //! @return  A function from Avoid::Points to Avoid::Points.
    //! @sa Compass::getInplaceRotationFunction
    static PlaneMap getRotationFunction(CardinalDir fromDir, CardinalDir toDir);

    //! @brief Get a function that rotates points by the angular displacement
    //!        from one cardinal direction to another.
    //! @param[in] fromDir  The starting cardinal direction.
    //! @param[in] toDir  The ending cardinal direction.
    //! @return  A function that alters Avoid::Points in-place.
    //! @sa Compass::getRotationFunction
    static InplacePlaneMap getInplaceRotationFunction(CardinalDir fromDir, CardinalDir toDir);

    //! @brief Get the signs of the coordinates of a vector pointing in the given direction.
    //! @param[in] d  The direction.
    //! @return  An Avoid::Point (xs, ys), where xs in {-1, 0, 1} represents the sign of the
    //!          x-coordinate of a vector lying in the "octant" represented by the given direction,
    //!          and likewise for ys. Here an "octant" is a semiaxis for a cardinal direction,
    //!          and an open quadrant for an ordinal direction.
    static Avoid::Point vectorSigns(CompassDir d);

    //! Cardinal directions in clockwise order, starting with EAST
    static const CardinalDirs cwCards;

    //! Cardinal directions in anticlockwise order, starting with EAST
    static const CardinalDirs acwCards;

    //! All compass directions, in clockwise order, with the list doubled, i.e.
    //! given twice. The doubling is useful when seeking intervals in this cyclic order.
    //! The list starts with EAST.
    static const CompassDirs cwAllDoubled;

    //! Like cwAllDoubled, only this time in anticlockwise order.
    static const CompassDirs acwAllDoubled;

    //! The vertical cardinal directions:
    static const std::set<CardinalDir> vertical;

    //! The horizontal cardinal directions:
    static const std::set<CardinalDir> horizontal;

    //! Map cardinal directions to corresponding libavoid connection direction flags.
    static const std::map<CardinalDir, Avoid::ConnDirFlag> libavoidConnDirs;

    //! Rotate clockwise 90 degrees.
    static const std::map<CompassDir, CompassDir> rotateCw90;

    //! Flip a given direction (or rotate 180 degrees).
    static const std::map<CompassDir, CompassDir> flip;

    //! Map cardinal direcs to their variable dimension.
    static const std::map<CardinalDir, vpsc::Dim> varDim;

    //! Map cardinal direcs to their constant dimension.
    static const std::map<CardinalDir, vpsc::Dim> constDim;

};

typedef std::pair<double, double> interval;

struct LineSegment {
    LineSegment(Avoid::Point p0, Avoid::Point p1)
        : p0(p0), p1(p1),
          x0(p0.x), y0(p0.y),
          x1(p1.x), y1(p1.y),
          direc(Compass::cardinalDirection(p0, p1)),
          varDim(Compass::varDim.at(direc)),
          constDim(Compass::constDim.at(direc))
    {
        if (Compass::isVerticalCard(direc)) {
            z = x0; w0 = y0; w1 = y1;
        } else {
            z = y0; w0 = x0; w1 = x1;
        }
        auto p = std::minmax(w0, w1);
        wl = p.first;
        wh = p.second;
        length = wh - wl;
    }

    //! @brief  Get the constant coordinate.
    double getConstCoord(void) const { return z; }

    //! @brief  Check whether the closed interval spanned by this segment
    //!         intersects a given closed interval [a, b].
    //! @param a  Low end of the interval.
    //! @param b  High end of the interval.
    bool closedIntervalIntersects(double a, double b) const;

    //! @brief  Check whether the open interval spanned by this segment
    //!         intersects a given open interval (a, b).
    //! @param a  Low end of the interval.
    //! @param b  High end of the interval.
    bool openIntervalIntersects(double a, double b) const;

    //! @brief  Check whether the closed interval spanned by this segment
    //!         includes a given value.
    //! @param a  The value in question.
    bool closedIntervalIncludesCoord(double a) const;

    //! @brief  Check whether the open interval spanned by this segment
    //!         includes a given value.
    //! @param a  The value in question.
    bool openIntervalIncludesCoord(double a) const;

    //! @brief  Compute the intersection of this segment's closed interval, with
    //!         the given interval.
    //! @param a  Low end of the interval.
    //! @param b  High end of the interval.
    //! @return  an interval representing the intersection.
    //!
    //! @note  If the intersection is empty, we return an "empty interval" in which
    //!        the second coordinate is less than the first.
    interval closedIntervalIntersection(double a, double b) const;

    //! @brief  Compute the intersection of this segment's open interval, with
    //!         the given interval.
    //! @param a  Low end of the interval.
    //! @param b  High end of the interval.
    //! @return  an interval representing the intersection.
    //!
    //! @note  If the intersection is empty, we return an "empty interval" in which
    //!        the second coordinate is less than the first.
    interval openIntervalIntersection(double a, double b) const;

    //! @brief  Check on which side of the line segment a given point lies.
    //! @param p  The point in question.
    //! @return  An integer in {-1, 0, 1} indicating, respectively, that in the dimension in
    //!          which the line segment is constant, the point's coordinate is less than,
    //!          equal to, or greater than the constant coord of the segment.
    int ptOnWhichSide(Avoid::Point p) const;

    //! @brief  Check whether a given coord is less than, equal to, or greater than the
    //!         line segment's constant coord.
    //! @param z  The coordinate in question.
    //! @return  An integer in {-1, 0, 1} indicating, respectively, that the given coordinate
    //!          is less than, equal to, or greater that the segment's constant coord.
    int coordOnWhichSide(double z1) const;

    static const double EPSILON;

    Avoid::Point p0;
    Avoid::Point p1;
    //! Coords of point p0:
    double x0;
    double y0;
    //! Coords of point p1:
    double x1;
    double y1;
    //! Direction from p0 to p1:
    CardinalDir direc;
    //! Variable dimension:
    vpsc::Dim varDim;
    //! Constant dimension:
    vpsc::Dim constDim;
    //! Constant coordinate:
    double z;
    //! Variable coordinate of point p0:
    double w0;
    //! Variable coordinate of point p1:
    double w1;
    //! Low variable coordinate:
    double wl;
    //! High variable coordinate:
    double wh;
    //! Length:
    double length;
};


} // namespace dialect

#endif // DIALECT_ORTHO_H
