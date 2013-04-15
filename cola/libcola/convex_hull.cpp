/*
 * vim: ts=4 sw=4 et tw=0 wm=0
 *
 * libcola - A library providing force-directed network layout using the 
 *           stress-majorization method subject to separation constraints.
 *
 * Copyright (C) 2006-2008  Monash University
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

#include <valarray>
#include <cfloat>
#include <algorithm>

#include "libvpsc/assertions.h"
#include "libcola/convex_hull.h"


namespace hull {
using namespace std;
/**
 * CrossProduct of three points: If the result is 0, the points are collinear; 
 * if it is positive, the three points (in order) constitute a "left turn", 
 * otherwise a "right turn".
 */
inline double crossProduct(
        double x0, double y0,
        double x1, double y1,
        double x2, double y2) {
    return (x1-x0)*(y2-y0)-(x2-x0)*(y1-y0);
}
struct CounterClockwiseOrder {
    CounterClockwiseOrder(
            const unsigned p,
            std::valarray<double> const & X, std::valarray<double> const & Y)
        :px(X[p]), py(Y[p]), X(X), Y(Y) {};
    bool operator() (unsigned i, unsigned j) {
        // o=crossProduct(px,py,X[i],Y[i],X[j],Y[j]);
        double xi=X[i]-px;
        double xj=X[j]-px;
        // since py is the min y pos, yi and yj must be positive
        double yi=Y[i]-py;
        double yj=Y[j]-py;
        
        // use cross product rule
        double o = xi*yj-xj*yi;
        if(o!=0) {
            return o>0;
        } 
        // in case of ties choose point farthest from p
        return (xi*xi+yi*yi) < (xj*xj+yj*yj);
    }
    const double px;
    const double py;
    std::valarray<double> const & X; 
    std::valarray<double> const & Y; 
};
void convex(const unsigned n, const double* X, const double* Y, vector<unsigned> & h) {
    const valarray<double> XA(X,n);
    const valarray<double> YA(Y,n);
    convex(XA,YA,h);
}
/**
 * Implementation of Graham's scan convex hull finding algorithm.
 * X and Y give the horizontal and vertical positions of the pointset.
 * The result is returned in hull as a list of indices referencing points in X and Y.
 */
void convex(valarray<double> const & X, valarray<double> const & Y, vector<unsigned> & h) {
    unsigned n=X.size();
    COLA_ASSERT(n==Y.size());
    unsigned p0=0;
    // find point p0 with min Y position, choose leftmost in case of tie.
    // This is our "pivot" point
    double minY=DBL_MAX,minX=DBL_MAX;
    for(unsigned i=0;i<n;i++) {
        if ( (Y[i] < minY) || ((Y[i] == minY) && (X[i] < minX)) ) 
        {
            p0=i;
            minY=Y[i];
            minX=X[i];
        }
    }
    // sort remaining points by the angle line p0-p1 (p1 in points) makes
    // with x-axis
    vector<unsigned> points;
    for(unsigned i=0;i<n;i++) { 
        if(i!=p0) points.push_back(i); 
    }
    CounterClockwiseOrder order(p0,X,Y);
    sort(points.begin(),points.end(),order);
    // now we maintain a stack in h, adding points while each successive
    // point is a "left turn", backtracking if we make a right turn.
    h.clear();
    h.push_back(p0);
    h.push_back(points[0]);
    for(unsigned i=1;i<points.size();i++) {
        double o=crossProduct(
                X[h[h.size()-2]],Y[h[h.size()-2]],
                X[h[h.size()-1]],Y[h[h.size()-1]],
                X[points[i]],Y[points[i]]);
        if(o==0) {
            h.pop_back();
            h.push_back(points[i]);
        } else if(o>0) {
            h.push_back(points[i]);
        } else {
            while(o<=0 && h.size()>2) {
                h.pop_back();
                o=crossProduct(
                    X[h[h.size()-2]],Y[h[h.size()-2]],
                    X[h[h.size()-1]],Y[h[h.size()-1]],
                    X[points[i]],Y[points[i]]);
            }
            h.push_back(points[i]);
        }
    }
}

} // namespace hull
