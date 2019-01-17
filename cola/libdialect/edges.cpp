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

#include <algorithm>
#include <string>
#include <sstream>
#include <utility>
#include <vector>
#include <iterator>
#include <cmath>

#include "libavoid/libavoid.h"
#include "libdialect/util.h"
#include "libdialect/graphs.h"

using namespace dialect;

using std::max;
using std::min;
using std::string;
using std::vector;
using std::ostringstream;

using Avoid::Point;
using Avoid::ConnEnd;
using Avoid::ConnDirFlag;

Edge_SP Edge::allocate(const Node_SP &src, const Node_SP &tgt) {
    // Can't use make_shared since Edge constructor is private.
    Edge_SP edge = Edge_SP(new Edge(src, tgt));
    // Add the Edge to each of its endpoint Nodes.
    src->addEdge(edge);
    tgt->addEdge(edge);
    return edge;
}

Edge::Edge(const Node_SP &src, const Node_SP &tgt) : m_ID(nextID++), m_src(src), m_tgt(tgt) {}

Node_SP Edge::getOtherEnd(const Node &end1) const {
    // We don't bother to check that end1 is actually an endpt.
    // I can't think of any case where that would be a legitimate
    // error. If you get this wrong, just fix it.
    //
    // We also don't bother to use our weak pointers' lock method;
    // instead we assume you know what you're doing, and that if
    // you're using this method you have an Edge whose endpoints
    // still exist!
    Node_SP sp_src(m_src);
    Node_SP sp_tgt(m_tgt);
    return end1.id() == sp_src->id() ? sp_tgt : sp_src;
}

void Edge::sever(void) {
    Node_SP sp_src(m_src);
    Node_SP sp_tgt(m_tgt);
    sp_src->removeEdge(*this);
    sp_tgt->removeEdge(*this);
}

BoundingBox Edge::getBoundingBox(void) const {
    Node_SP sp_src(m_src);
    Node_SP sp_tgt(m_tgt);
    Point sc = sp_src->getCentre();
    Point tc = sp_tgt->getCentre();
    double sx = sc.x, sy = sc.y,
           tx = tc.x, ty = tc.y,
           x = min(sx, tx),
           X = max(sx, tx),
           y = min(sy, ty),
           Y = max(sy, ty);
    for (auto p : m_route) {
        x = min(x, p.x);
        X = max(X, p.x);
        y = min(y, p.y);
        Y = max(Y, p.y);
    }
    return BoundingBox(x, X, y, Y);
}

void Edge::addRoutePoint(double x, double y) {
    m_route.emplace_back(x, y);
}

string Edge::writeRouteTglf(void) const {
    ostringstream tglf;
    for (auto pt : m_route) {
        tglf << " " << pt.x << " " << pt.y;
    }
    return tglf.str();
}

std::pair<ConnEnd, ConnEnd> Edge::makeLibavoidConnEnds(Avoid::ConnDirFlags srcDirs, Avoid::ConnDirFlags tgtDirs) {
    Node_SP sp_src(m_src);
    Node_SP sp_tgt(m_tgt);
    Point srcPt = sp_src->getCentre(),
          tgtPt = sp_tgt->getCentre();
    ConnEnd srcEnd(srcPt, srcDirs),
            tgtEnd(tgtPt, tgtDirs);
    return {srcEnd, tgtEnd};
}

vector<Point> Edge::getRoutePoints(void) const {
    if (!m_route.empty()) return m_route;
    Node_SP sp_src(m_src);
    Node_SP sp_tgt(m_tgt);
    vector<Point> line{sp_src->getCentre(), sp_tgt->getCentre()};
    return line;
}

string Edge::writeSvg(void) const {
    // Get the route points.
    vector<Point> route = getRoutePoints();
    COLA_ASSERT(route.size() >= 2);
    // Decide if it's an orthogonal route.
    bool isOrtho = true;
    Point prevPt = route.front();
    for (auto it = route.begin() + 1; it != route.end(); ++it) {
        Point pt = *it,
               d = pt - prevPt;
        if (d.x != 0 && d.y != 0) {
            isOrtho = false;
            break;
        }
        prevPt = pt;
    }
    string d = isOrtho ? writeRoundedOrthoConnectorData() : writePolylineConnectorData();
    ostringstream ss;
    ss << "<path stroke=\"black\" stroke-width=\"1\" fill=\"none\" ";
    ss << "d=\"" << d << "\"/>\n";
    return ss.str();
}

string Edge::writePolylineConnectorData(void) const {
    vector<Point> route = getRoutePoints();
    COLA_ASSERT(route.size() >= 2);
    ostringstream ss;
    Point p = route.front();
    ss << string_format("M %.2f,%.2f", p.x, p.y);
    for (auto it = route.begin() + 1; it != route.end(); ++it) {
        Point p = *it;
        ss << string_format("L %.2f,%.2f", p.x, p.y);
    }
    return ss.str();
}

string Edge::writeRoundedOrthoConnectorData(void) const {
    vector<Point> route = getRoutePoints();
    COLA_ASSERT(route.size() >= 2);
    // Set basic curve radius
    double cr0 = 10;
    ostringstream ss;
    Point p = route.front();
    ss << string_format("M %.2f,%.2f", p.x, p.y);
    double a, b, cr;
    for (auto it = route.begin() + 1; it != route.end() - 1; ++it) {
        Point q = *it,
              r = *(std::next(it));
        if (q.y < p.y) {
            //   q
            //   |
            //   p
            a = fabs(p.y-q.y);
            b = fabs(q.x-r.x);
            cr = min({cr0,a/2,b/2});
            ss << string_format(" V %.2f", q.y+cr);
            if (r.x < q.x) {
                // r -- q
                //      |
                //      p
                ss << string_format(" a %.2f,%.2f 0 0,0 %.2f,%.2f", cr, cr, -cr, -cr);
            } else {
                // q -- r
                // |
                // p
                ss << string_format(" a %.2f,%.2f 0 0,1 %.2f,%.2f", cr, cr, cr, -cr);
            }
        } else if (q.y > p.y) {
            //   p
            //   |
            //   q
            a = fabs(p.y-q.y);
            b = fabs(q.x-r.x);
            cr = min({cr0,a/2,b/2});
            ss << string_format(" V %.2f", q.y-cr);
            if (r.x < q.x) {
                //      p
                //      |
                // r -- q
                ss << string_format(" a %.2f,%.2f 0 0,1 %.2f,%.2f", cr, cr, -cr, cr);
            } else {
                // p
                // |
                // q -- r
                ss << string_format(" a %.2f,%.2f 0 0,0 %.2f,%.2f", cr, cr, cr, cr);
            }
        } else if (q.x > p.x) {
            //
            // p -- q
            //
            a = fabs(p.x-q.x);
            b = fabs(q.y-r.y);
            cr = min({cr0,a/2,b/2});
            ss << string_format(" H %.2f", q.x-cr);
            if (r.y < q.y) {
                //      r
                //      |
                // p -- q
                ss << string_format(" a %.2f,%.2f 0 0,0 %.2f,%.2f", cr, cr, cr, -cr);
            } else {
                // p -- q
                //      |
                //      r
                ss << string_format(" a %.2f,%.2f 0 0,1 %.2f,%.2f", cr, cr, cr, cr);
            }
        } else if (q.x < p.x) {
            //
            // q -- p
            //
            a = fabs(p.x-q.x);
            b = fabs(q.y-r.y);
            cr = min({cr0,a/2,b/2});
            ss << string_format(" H %.2f", q.x+cr);
            if (r.y < q.y) {
                // r
                // |
                // q -- p
                ss << string_format(" a %.2f,%.2f 0 0,1 %.2f,%.2f", cr, cr, -cr, -cr);
            } else {
                // q -- p
                // |
                // r
                ss << string_format(" a %.2f,%.2f 0 0,0 %.2f,%.2f", cr, cr, -cr, cr);
            }
        }
        p = q;
    }
    p = route.back();
    ss << string_format("L %.2f,%.2f", p.x, p.y);
    return ss.str();
}

void Edge::setRoute(std::vector<Avoid::Point> route) {
    m_route.clear();
    m_route.reserve(route.size());
    m_route.insert(m_route.begin(), route.begin(), route.end());
}

void Edge::rotate90cw(void) {
    auto r = Compass::getInplaceRotationFunction(CardinalDir::EAST, CardinalDir::SOUTH);
    for (Point &p : m_route) r(p);
}

void Edge::rotate90acw(void) {
    auto r = Compass::getInplaceRotationFunction(CardinalDir::EAST, CardinalDir::NORTH);
    for (Point &p : m_route) r(p);
}

void Edge::rotate180(void) {
    auto r = Compass::getInplaceRotationFunction(CardinalDir::EAST, CardinalDir::WEST);
    for (Point &p : m_route) r(p);
}

void Edge::translate(double dx, double dy) {
    for (Point &p : m_route) {
        p.x += dx;
        p.y += dy;
    }
}

void Edge::clearRouteAndBends(void) {
    m_route.clear();
    // Since we keep shared pointers to bend nodes, we need not destroy them.
    m_bendNodes.clear();
}

void Edge::buildRouteFromBends(void) {
    m_route.clear();
    Node_SP sp_src(m_src);
    Node_SP sp_tgt(m_tgt);
    m_route.push_back(sp_src->getCentre());
    for (Node_SP b : m_bendNodes) m_route.push_back(b->getCentre());
    m_route.push_back(sp_tgt->getCentre());
}
