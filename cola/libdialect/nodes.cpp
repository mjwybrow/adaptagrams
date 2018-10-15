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

#include <cmath>
#include <sstream>

#include "libavoid/geomtypes.h"

#include "libdialect/commontypes.h"
#include "libdialect/util.h"
#include "libdialect/ortho.h"
#include "libdialect/graphs.h"

using std::vector;
using std::string;

using namespace dialect;

using Avoid::Polygon;
using Avoid::Point;

Node_SP Node::allocate(void) {
    // Cannot use make_shared because Node constructor is private.
    return Node_SP(new Node());
}

Node_SP Node::allocate(double w, double h) {
    return Node_SP(new Node(w, h));
}

Node_SP Node::allocate(double cx, double cy, double w, double h) {
    return Node_SP(new Node(cx, cy, w, h));
}

GhostNode_SP Node::makeGhost(void) const {
    return GhostNode::allocate(*this);
}

void Node::addEdge(const Edge_SP &edge) {
    // Record the Edge in our lookup tables.
    // (1) Edges by their own IDs:
    m_edges.emplace(edge->id(), edge);
    // (2) Edges by the ID of the Node at the opposite end:
    Node_SP opposite_end = edge->getOtherEnd(*this);
    m_edges_by_opp_id.emplace(opposite_end->id(), edge);
    // And increment the degree of this Node.
    ++m_degree;
}

void Node::removeEdge(const Edge &edge) {
    // Remove the Edge from our lookup tables.
    // (1) Edges by their own IDs:
    m_edges.erase(edge.id());
    // (2) Edges by the ID of the Node at the opposite end:
    Node_SP opposite_end = edge.getOtherEnd(*this);
    m_edges_by_opp_id.erase(opposite_end->id());
    // And decrement the degree of this Node.
    --m_degree;
}

void Node::setCentre(double cx, double cy) {
    m_cx = cx;
    m_cy = cy;
}

void Node::translate(double dx, double dy) {
    m_cx += dx;
    m_cy += dy;
}

void Node::applyPlaneMap(PlaneMap map) {
    Point p = map(Point(m_cx, m_cy));
    m_cx = p.x;
    m_cy = p.y;
}

void Node::setDims(double w, double h) {
    m_w = w;
    m_h = h;
}

void Node::addPadding(double dw, double dh) {
    m_w += dw;
    m_h += dh;
}

void Node::setBoundingBox(double x, double X, double y, double Y) {
    COLA_ASSERT(x < X && y < Y);
    m_w = X - x;
    m_h = Y - y;
    m_cx = x + m_w/2.0;
    m_cy = y + m_h/2.0;
}

void Node::copyGeometry(const Node &other) {
    m_cx = other.m_cx;
    m_cy = other.m_cy;
    m_w  = other.m_w;
    m_h  = other.m_h;
}

void Node::copyOtherGhostProperties(const Node &other) {
    m_isRoot = other.m_isRoot;
}

dimensions Node::getHalfDimensions(void) const {
    return dimensions(m_w/2.0, m_h/2.0);
}

dimensions Node::getDimensions(void) const {
    return dimensions(m_w, m_h);
}

Avoid::Point Node::getCentre(void) const {
    return Avoid::Point(m_cx, m_cy);
}

BoundingBox Node::getBoundingBox(void) const {
    dimensions hd = getHalfDimensions();
    return BoundingBox(
        m_cx - hd.first, m_cx + hd.first,
        m_cy - hd.second, m_cy + hd.second
    );
}

void Node::updatePosnFromRect(vpsc::Rectangle *r) {
    m_cx = r->getCentreX();
    m_cy = r->getCentreY();
}

void Node::updateXCoordFromRect(vpsc::Rectangle *r) {
    m_cx = r->getCentreX();
}

void Node::updateYCoordFromRect(vpsc::Rectangle *r) {
    m_cy = r->getCentreY();
}

Nodes Node::getNeighbours(void) const {
    Nodes nbrs;
    for (auto p : m_edges) {
        Node_SP other = p.second->getOtherEnd(*this);
        nbrs.push_back(other);
    }
    return nbrs;
}

Nodes Node::getNeighboursCwCyclic(void) const {
    Nodes nbrs = getNeighbours();
    std::sort(nbrs.begin(), nbrs.end(), [this](const Node_SP &a, const Node_SP &b) -> bool {
        Point c_a = a->getCentre(),
              c_b = b->getCentre();
        double A = std::atan2(c_a.y - m_cy, c_a.x - m_cx),
               B = std::atan2(c_b.y - m_cy, c_b.x - m_cx);
        return A < B;
    });
    return nbrs;
}

Nodes Node::getChildren(void) const {
    Nodes children;
    // Look through all incident Edges to this Node.
    for (auto p : m_edges) {
        // When the target end of the Edge is different from
        // this Node itself, we have a child of this Node.
        Node_SP tgt = p.second->getTargetEnd();
        if (tgt->id() != m_ID) {
            children.push_back(tgt);
        }
    }
    return children;
}

Nodes GhostNode::getChildren(void) const {
    Nodes children;
    // Look through all incident Edges to this Node.
    for (auto p : m_edges) {
        // When the target end of the Edge is different from
        // this Node itself, we have a child of this Node.
        Node_SP tgt = p.second->getTargetEnd();
        // Since this is a GhostNode, whose purpose is sometimes to report a
        // "fake ID", we must call the instance's own id() function here, rather
        // than simply accessing the member field m_ID, as we can do in the base Node class.
        if (tgt->id() != id()) {
            children.push_back(tgt);
        }
    }
    return children;
}

string Node::writeSvg(bool useExternalId) const {
    BoundingBox b = getBoundingBox();
    string label = string_format("%d", (useExternalId ? m_externalID : m_ID));
    double dy = 5;
    double L = (double) label.length();
    double dx = -(4*L + 1);
    std::ostringstream ss;
    ss << "<g>\n";
    ss << "<rect stroke=\"black\" stroke-width=\"1\" fill=\"gray\" opacity=\"0.5\" ";
    ss << string_format("x=\"%.2f\" y=\"%.2f\" width=\"%.2f\" height=\"%.2f\"/>\n", b.x, b.y, b.w(), b.h());
    ss << "<text font-family=\"sans-serif\" ";
    ss << string_format("x=\"%.2f\" y=\"%.2f\">\n", m_cx + dx, m_cy + dy);
    ss << label << "\n";
    ss << "</text>\n";
    ss << "</g>\n";
    return ss.str();
}

Polygon Node::makeLibavoidPolygon(void) const {
    Polygon poly(4);
    BoundingBox b = getBoundingBox();
    poly.setPoint(0, Point(b.x, b.y));
    poly.setPoint(1, Point(b.X, b.y));
    poly.setPoint(2, Point(b.X, b.Y));
    poly.setPoint(3, Point(b.x, b.Y));
    return poly;
}

bool Node::liesOppositeSegment(const LineSegment &seg, bool openInterval) {
    BoundingBox bb = getBoundingBox();
    double a = seg.varDim == vpsc::XDIM ? bb.x : bb.y,
           b = seg.varDim == vpsc::XDIM ? bb.X : bb.Y;
    return openInterval ? seg.openIntervalIntersects(a, b) :
                          seg.closedIntervalIntersects(a, b);
}

Point Node::getBoundaryCompassPt(CompassDir dir) const {
    Point sgns = Compass::vectorSigns(dir);
    double hw = m_w/2.0,
           hh = m_h/2.0;
    return Point(m_cx + sgns.x * hw, m_cy + sgns.y * hh);
}
