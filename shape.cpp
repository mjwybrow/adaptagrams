/*
 * vim: ts=4 sw=4 et tw=0 wm=0
 *
 * libavoid - Fast, Incremental, Object-avoiding Line Router
 *
 * Copyright (C) 2004-2008  Monash University
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


#include "libavoid/shape.h"
#include "libavoid/graph.h"  // For alertConns
#include "libavoid/vertices.h"
#include "libavoid/router.h"
#include "libavoid/connend.h"
#include "libavoid/debug.h"
#include "libavoid/assertions.h"
#include "libavoid/connectionpin.h"


namespace Avoid {


ShapeRef::ShapeRef(Router *router, Polygon& ply, const unsigned int id)
    : m_router(router)
    , m_polygon(ply)
    , m_active(false)
    , m_in_move_list(false)
    , m_first_vert(NULL)
    , m_last_vert(NULL)
{
    COLA_ASSERT(m_router != NULL);
    m_id = m_router->assignId(id);

    VertID i = VertID(m_id, 0);
    
    const bool addToRouterNow = false;
    VertInf *last = NULL;
    VertInf *node = NULL;
    for (size_t pt_i = 0; pt_i < m_polygon.size(); ++pt_i)
    {
        node = new VertInf(m_router, i, m_polygon.ps[pt_i], addToRouterNow);

        if (!m_first_vert)
        {
            m_first_vert = node;
        }
        else
        {
            node->shPrev = last;
            last->shNext = node;
            //node->lstPrev = last;
            //last->lstNext = node;
        }
        
        last = node;
        i++;
    }
    m_last_vert = node;
    
    m_last_vert->shNext = m_first_vert;
    m_first_vert->shPrev = m_last_vert;
}


ShapeRef::~ShapeRef()
{
    COLA_ASSERT(!m_router->shapeInQueuedActionList(this));

    if (m_active)
    {
        // Destroying a shape without calling removeShape(), so do it now.
        m_router->removeShape(this);
        m_router->processTransaction();
    }

    COLA_ASSERT(m_first_vert != NULL);
    
    VertInf *it = m_first_vert;
    do
    {
        VertInf *tmp = it;
        it = it->shNext;

        delete tmp;
    }
    while (it != m_first_vert);
    m_first_vert = m_last_vert = NULL;

    // Free and clear any connection pins.
    while (!m_connection_pins.empty())
    {
        delete *(m_connection_pins.begin());
    }
}


void ShapeRef::setNewPoly(const Polygon& poly)
{
    COLA_ASSERT(m_first_vert != NULL);
    COLA_ASSERT(m_polygon.size() == poly.size());
    
    VertInf *curr = m_first_vert;
    for (size_t pt_i = 0; pt_i < m_polygon.size(); ++pt_i)
    {
        COLA_ASSERT(curr->visListSize == 0);
        COLA_ASSERT(curr->invisListSize == 0);

        // Reset with the new polygon point.
        curr->Reset(poly.ps[pt_i]);
        curr->pathNext = NULL;
        
        curr = curr->shNext;
    }
    COLA_ASSERT(curr == m_first_vert);
        
    m_polygon = poly;
}


void ShapeRef::moveAttachedConns(const Polygon& newPoly)
{
    // Update positions of attached connector ends.
    for (std::set<ConnEnd *>::iterator curr = m_following_conns.begin();
            curr != m_following_conns.end(); ++curr)
    {
        ConnEnd *connEnd = *curr;
        COLA_ASSERT(connEnd->m_conn_ref != NULL);
        m_router->modifyConnector(connEnd->m_conn_ref, connEnd->type(), 
                *connEnd);
    }
    for (std::set<ShapeConnectionPin *>::iterator curr = 
            m_connection_pins.begin(); curr != m_connection_pins.end(); ++curr)
    {
        ShapeConnectionPin *pin = *curr;
        pin->updatePosition(newPoly);
    }
}


void ShapeRef::makeActive(void)
{
    COLA_ASSERT(!m_active);
    
    // Add to shapeRefs list.
    m_shaperefs_pos = m_router->shapeRefs.insert(m_router->shapeRefs.begin(), this);

    // Add points to vertex list.
    VertInf *it = m_first_vert;
    do
    {
        VertInf *tmp = it;
        it = it->shNext;

        m_router->vertices.addVertex(tmp);
    }
    while (it != m_first_vert);
    
    m_active = true;
}


void ShapeRef::makeInactive(void)
{
    COLA_ASSERT(m_active);
    
    // Remove from shapeRefs list.
    m_router->shapeRefs.erase(m_shaperefs_pos);

    // Remove points from vertex list.
    VertInf *it = m_first_vert;
    do
    {
        VertInf *tmp = it;
        it = it->shNext;

        m_router->vertices.removeVertex(tmp);
    }
    while (it != m_first_vert);
    
    m_active = false;
    
    // Turn attached ConnEnds into manual points.
    bool deletedShape = true;
    while (!m_following_conns.empty())
    {
        ConnEnd *connEnd = *(m_following_conns.begin());
        connEnd->disconnect(deletedShape);
    }
}


unsigned int ShapeRef::addConnectionPin(ShapeConnectionPin *pin)
{
    m_connection_pins.insert(pin);
    m_router->modifyConnectionPin(pin);

    return m_connection_pins.size();
}

void ShapeRef::removeConnectionPin(ShapeConnectionPin *pin)
{
    m_connection_pins.erase(pin);
    m_router->modifyConnectionPin(pin);
}


bool ShapeRef::isActive(void) const
{
    return m_active;
}


VertInf *ShapeRef::firstVert(void)
{
    return m_first_vert;
}


VertInf *ShapeRef::lastVert(void)
{
    return m_last_vert;
}


unsigned int ShapeRef::id(void) const
{
    return m_id;
}


const Polygon& ShapeRef::polygon(void) const
{
    return m_polygon;
}


Router *ShapeRef::router(void) const
{
    return m_router;
}


void ShapeRef::boundingBox(BBox& bbox)
{
    COLA_ASSERT(!m_polygon.empty());

    bbox.a = bbox.b = m_polygon.ps[0];
    Point& a = bbox.a;
    Point& b = bbox.b;

    for (size_t i = 1; i < m_polygon.size(); ++i)
    {
        const Point& p = m_polygon.ps[i];

        a.x = std::min(p.x, a.x);
        a.y = std::min(p.y, a.y);
        b.x = std::max(p.x, b.x);
        b.y = std::max(p.y, b.y);
    }
}


Point ShapeRef::shapeCentre(void)
{
    BBox bb;
    boundingBox(bb);

    Point centre;

    centre.x = bb.a.x + (0.5 * (bb.b.x - bb.a.x));
    centre.y = bb.a.y + (0.5 * (bb.b.y - bb.a.y));

    return centre;
}


void ShapeRef::removeFromGraph(void)
{
    bool isConnPt = false;
    for (VertInf *iter = firstVert(); iter != lastVert()->lstNext; )
    {
        VertInf *tmp = iter;
        iter = iter->lstNext;
 
        tmp->removeFromGraph(isConnPt);
    }
}


void ShapeRef::markForMove(void)
{
    if (!m_in_move_list)
    {
        m_in_move_list = true;
    }
    else
    {
        db_printf("WARNING: two moves queued for same shape prior to rerouting."
                "\n         This is not safe.\n");
    }
}


void ShapeRef::clearMoveMark(void)
{
    m_in_move_list = false;
}


VertInf *ShapeRef::getPointVertex(const Point& point)
{
    VertInf *curr = m_first_vert;
    do
    {
        if (curr->point == point)
        {
            return curr;
        }
        curr = curr->shNext;
    }
    while (curr != m_first_vert);

    return NULL;
}


void ShapeRef::addFollowingConnEnd(ConnEnd *connEnd)
{
    m_following_conns.insert(connEnd);
}


void ShapeRef::removeFollowingConnEnd(ConnEnd *connEnd)
{
    m_following_conns.erase(connEnd);
}


}


