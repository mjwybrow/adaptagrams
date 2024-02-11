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

#ifndef DIALECT_UTIL_H
#define DIALECT_UTIL_H

#include <cassert>
#include <cmath>
#include <string>
#include <limits>
#include <memory>
#include <cstdio>
#include <map>
#include <vector>
#include <utility>
#include <algorithm>
#include <functional>
#include <math.h>

#include "libavoid/geomtypes.h"

namespace dialect {

//! @brief  Tolerant equality test for doubles. Generates principled value
//!         for tolerance.
//!
//! @note  Thanks to: https://stackoverflow.com/a/4010279
inline bool logically_equal(double a, double b, double error_factor=1.0)
{
    return a==b || std::abs(a-b) < std::abs(std::min(a,b))*std::numeric_limits<double>::epsilon()*error_factor;
}

//! @brief  Tolerant equality test for doubles. Uses arbitrary tolerance.
inline bool approx_equal(double a, double b, double tol=0.000001)
{
    return a==b || std::abs(a-b) < tol;
}

//! @brief  String formatting
//!
//! @note  Thanks to: https://stackoverflow.com/a/26221725
template<typename ... Args>
std::string string_format( const std::string& format, Args ... args )
{
    size_t size = std::snprintf( nullptr, 0, format.c_str(), args ... ) + 1; // Extra space for '\0'
    std::unique_ptr<char[]> buf( new char[ size ] );
    std::snprintf( buf.get(), size, format.c_str(), args ... );
    return std::string( buf.get(), buf.get() + size - 1 ); // We don't want the '\0' inside
}

//! Dense 2d array, with integer indices.
template<typename T>
struct Matrix2d {
    int rows, cols;
    std::vector<T> data;
    Matrix2d() : rows(0), cols(0) {}
    Matrix2d(int rows, int cols) : rows(rows), cols(cols), data(rows*cols)
    { }

    T operator()(int i, int j) const
    {
        assert(i < rows);
        assert(j < cols);
        return data[i*cols+j];
    }
    T& operator()(int i, int j)
    {
        assert(i < rows);
        assert(j < cols);
        return data[i*cols+j];
    }

    std::string toString() const {
        std::string s = "";
        s += "\n  ";
        char buffer [21];
        for (int j=0; j<cols; j++) {
            sprintf(buffer," %2d",j);
            s += std::string(buffer);
        }
        for (int i=0; i<rows; i++) {
            s += "\n";
            sprintf(buffer,"%2d",i);
            s += std::string(buffer);
            for (int j=0; j<cols; j++) {
                sprintf(buffer," %2d",data[i*cols+j]);
                s += std::string(buffer);
            }
        }
        return s;
    }

};

//! Suppose you are working with some objects A1, A2, ... each of which
//! has a point (x, y) associated with it.
//! For each new object Ai, you want to check whether there is already another
//! object Aj whose coordinates are almost the same, within a given threshold.
//!
//! A NearbyObjectFinder can be used for this problem.
//! Construct it with the desired threshold.
//! Before adding any new object to it, use its 'findObject' method to
//! see whether it already has an object with both x and y within the
//! threshold of the x and y for the new object.
//! If not, then add the object using the 'addObject' method.
//!
//! Note: a NearbyObjectFinder looks for objects in the /open/ neighbourhood
//! of a given point, with radius equal to the threshold.
template <typename T>
class NearbyObjectFinder {
public:

    //! @brief  Construct a NearbyObjectFinder that looks for objects within a given threshold.
    //!
    //! @param[in] threshold  The radius of the /open/ neighbourhoods in which to search.
    NearbyObjectFinder(double threshold) : m_thresh(threshold) {}

    //! @brief  Add a new object, and say what its x,y-coords are
    //!
    //! @param[in] x  the x-coordinate of the object
    //! @param[in] y  the y-coordinate of the object
    //! @param[in] obj  the object
    void addObject(double x, double y, T obj) {
        // Round coords to integers in order to choose a bucket.
        int x0 = (int) std::round(x),
            y0 = (int) std::round(y);
        // Store the point and the object in the bucket.
        m_objects[x0][y0].push_back({Avoid::Point(x, y), obj});
    }

    //! @brief  Check to see if any object has been stored yet in the open neighbourhood centred
    //!         at the given coordinates, and with radius equal to the threshold.
    //!         If so, return it. Otherwise return a value-initialized object of type T.
    //! @param[in] x  target x-coord
    //! @param[in] y  target y-coord
    //! @return  copy of object stored under nearby coords, or value-initialised object
    //!          of type T if none present.
    T findObject(double x, double y) {
        // Consider the range of all integer coords under which a nearby
        // point could have been stored, according to the threshold.
        int u0 = (int) std::round(x - m_thresh),
            u1 = (int) std::round(x + m_thresh),
            v0 = (int) std::round(y - m_thresh),
            v1 = (int) std::round(y + m_thresh);
        // Iterate over the range, looking for a match, and returning first one found, if any.
        for (int u = u0; u <= u1; ++u) {
            for (int v = v0; v <= v1; ++v) {
                for (auto pair : m_objects[u][v]) {
                    Avoid::Point p = pair.first;
                    if (fabs(x - p.x) < m_thresh && fabs(y - p.y) < m_thresh) return pair.second;
                }
            }
        }
        // If we didn't find any match, then we return a value-initialised object.
        return T();
    }

private:
    double m_thresh;
    // Object storage format:
    // We will be given an object of type T to store, under a point (x, y).
    // We will round the coordinates to integers x0, y0, and store the object
    // under a "bucket" for these integer coords. Thus under [x0][y0] we have a vector
    // of pairs, giving the original point (x, y), and the object of type T.
    // So it looks like:
    //              (int, int) --> [ {(x,y), T}, {(x,y), T}, ... ]
    std::map<int, std::map<int, std::vector<std::pair<Avoid::Point, T>>>> m_objects;
};

//! @brief Partition a vector of items according to a key value.
//!
//! @param[in] items  The vector of objects of type T that are to be partitioned.
//! @param[in] key  A function returning a floating point key value on objects of type T.
//! @param[in] tolerance  If positive, put items into the same part provided their key
//!                       is within tolerance of a running average key value for that part.
//!
//! @note  As a by-product, each part will be sorted in ascending order according to the given key function.
//!
//! @return vector of vectors of objects of type T.
template <typename T>
std::vector<std::vector<T>> partition(std::vector<T> items, std::function<double(T)> key, double tolerance=0) {
    // Prepare the return value
    std::vector<std::vector<T>> parts;
    // If there are no items, there is nothing to do.
    if (items.size() == 0) return parts;
    // Else sort by the given key.
    std::sort(items.begin(), items.end(),
        [&key](const T &a, const T &b) -> bool {
            return key(a) < key(b);
        }
    );
    // Initialise first part with first item.
    auto it = items.cbegin();
    T firstItem = *it;
    std::vector<T> part{firstItem};
    // Initialise the average key value for the part.
    double avg = key(firstItem);
    // Let n record how many keys are in the average.
    size_t n = 1;
    // Consider the remaining items.
    for (auto jt = ++it; jt != items.cend(); ++jt) {
        T item = *jt;
        double k = key(item);
        double dk = k - avg;
        // NB: Using <= (not strict <) in this test is necessary so that the case
        // where tolerance == 0 works!
        if (fabs(dk) <= tolerance) {
            // Difference of new key with current avg is within
            // tolerance. Add item to current part.
            part.push_back(item);
            // Update n and avg. Note that when tolerance is 0, then k == avg, so avg stays constant.
            avg = (n*avg + k)/(n+1);
            ++n;
        } else {
            // Difference is too much. Record the current part
            // and begin a new one for this item.
            parts.push_back(part);
            part = {item};
            avg = k;
            n = 1;
        }
    }
    // At this point there is always a nonempty part that has not
    // yet been appended to the part list.
    parts.push_back(part);
    return parts;
}

}  // namespace dialect

#endif  // DIALECT_UTIL_H
