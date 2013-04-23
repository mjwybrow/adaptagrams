/*
 * vim: ts=4 sw=4 et tw=0 wm=0
 *
 * libavoid - Fast, Incremental, Object-avoiding Line Router
 *
 * Copyright (C) 2010  Monash University
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
 * Author(s):  Michael Wybrow
*/

#include <cfloat>

#include "libavoid/connectionpin.h"
#include "libavoid/shape.h"
#include "libavoid/junction.h"
#include "libavoid/vertices.h"
#include "libavoid/router.h"
#include "libavoid/visibility.h"
#include "libavoid/debug.h"


namespace Avoid {

ShapeConnectionPin::ShapeConnectionPin(ShapeRef *shape, 
        const unsigned int classId, const double xPortionOffset,
        const double yPortionOffset, const double insideOffset, 
        const ConnDirFlags visDirs)
    : m_shape(shape),
      m_junction(NULL),
      m_class_id(classId),
      m_x_portion_offset(xPortionOffset),
      m_y_portion_offset(yPortionOffset),
      m_inside_offset(insideOffset),
      m_visibility_directions(visDirs),
      m_exclusive(true),
      m_connection_cost(0.0),
      m_vertex(NULL)
{
    COLA_ASSERT(m_shape != NULL);
    COLA_ASSERT(m_class_id > 0);

    // Parameter checking
    if ((xPortionOffset < 0) || (xPortionOffset > 1))
    {
        err_printf("xPortionOffset value (%g) in ShapeConnectionPin constructor not "
                "between 0 and 1.\n", xPortionOffset);
    }
    if ((yPortionOffset < 0) || (yPortionOffset > 1))
    {
        err_printf("yPortionOffset value (%g) in ShapeConnectionPin constructor not "
                "between 0 and 1.\n", yPortionOffset);
    }

    m_router = m_shape->router();
    m_shape->addConnectionPin(this);
    
    // Create a visibility vertex for this ShapeConnectionPin.
    VertID id(m_shape->id(), kShapeConnectionPin, 
            VertID::PROP_ConnPoint | VertID::PROP_ConnectionPin);
    m_vertex = new VertInf(m_router, id, this->position());
    m_vertex->visDirections = this->directions();
    
    if (m_vertex->visDirections == ConnDirAll)
    {
        // A pin with visibility in all directions is not exclusive 
        // by default.
        m_exclusive = false;
    }

    if (m_router->m_allows_polyline_routing)
    {
        vertexVisibility(m_vertex, NULL, true, true);
    }
}


ShapeConnectionPin::ShapeConnectionPin(JunctionRef *junction, 
        const unsigned int classId, const ConnDirFlags visDirs)
    : m_shape(NULL),
      m_junction(junction),
      m_class_id(classId),
      m_x_portion_offset(0.0),
      m_y_portion_offset(0.0),
      m_inside_offset(0.0),
      m_visibility_directions(visDirs),
      m_exclusive(true),
      m_connection_cost(0.0),
      m_vertex(NULL)
{
    COLA_ASSERT(m_junction != NULL);
    m_router = m_junction->router();
    m_junction->addConnectionPin(this);
    
    // Create a visibility vertex for this ShapeConnectionPin.
    VertID id(m_junction->id(), kShapeConnectionPin, 
            VertID::PROP_ConnPoint | VertID::PROP_ConnectionPin);
    m_vertex = new VertInf(m_router, id, m_junction->position());
    m_vertex->visDirections = visDirs;

    if (m_router->m_allows_polyline_routing)
    {
        vertexVisibility(m_vertex, NULL, true, true);
    }
}


ShapeConnectionPin::~ShapeConnectionPin()
{
    COLA_ASSERT(m_shape || m_junction);
    if (m_shape)
    {
        m_shape->removeConnectionPin(this);
    }
    else if (m_junction)
    {
        m_junction->removeConnectionPin(this);
    }

    // Disconnect connend using this pin.
    while (!m_connend_users.empty())
    {
        ConnEnd *connend = *(m_connend_users.begin());
        connend->freeActivePin();
    }

    if (m_vertex)
    {
        m_vertex->removeFromGraph();
        m_router->vertices.removeVertex(m_vertex);
        delete m_vertex;
        m_vertex = NULL;
    }
}

void ShapeConnectionPin::updatePositionAndVisibility(void)
{
    m_vertex->Reset(this->position());
    m_vertex->visDirections = this->directions();
    updateVisibility();
}

void ShapeConnectionPin::updateVisibility(void)
{
    m_vertex->removeFromGraph();
    if (m_router->m_allows_polyline_routing)
    {
        vertexVisibility(m_vertex, NULL, true, true);
    }
}

void ShapeConnectionPin::setConnectionCost(const double cost)
{
    COLA_ASSERT(cost >= 0);

    m_connection_cost = cost;
}
 

void ShapeConnectionPin::setExclusive(const bool exclusive)
{
    m_exclusive = exclusive;
}


void ShapeConnectionPin::updatePosition(const Point& newPosition)
{
    m_vertex->Reset(newPosition);
}


void ShapeConnectionPin::updatePosition(const Polygon& newPoly)
{
    m_vertex->Reset(position(newPoly));
}


const Point ShapeConnectionPin::position(const Polygon& newPoly) const
{
    if (m_junction)
    {
        return m_junction->position();
    }

    const Polygon& poly = (newPoly.empty()) ? m_shape->polygon() : newPoly;

    double x_min = DBL_MAX;
    double x_max = -DBL_MAX;
    double y_min = DBL_MAX;
    double y_max = -DBL_MAX;
    for (size_t i = 0; i < poly.size(); ++i)
    {
        x_min = std::min(x_min, poly.ps[i].x);
        x_max = std::max(x_max, poly.ps[i].x);
        y_min = std::min(y_min, poly.ps[i].y);
        y_max = std::max(y_max, poly.ps[i].y);
    }

    Point point;

    // We want to place connection points exactly on the edges of shapes, 
    // or possibly slightly inside them (if m_insideOfset is set).
    if (m_x_portion_offset == ATTACH_POS_LEFT)
    {
        point.x = x_min + m_inside_offset;
        point.vn = 6;
    }
    else if (m_x_portion_offset == ATTACH_POS_RIGHT)
    {
        point.x = x_max - m_inside_offset;
        point.vn = 4;
    }
    else
    {
        point.x = x_min + (m_x_portion_offset * (x_max - x_min));
    }

    if (m_y_portion_offset == ATTACH_POS_TOP)
    {
        point.y = y_min + m_inside_offset;
        point.vn = 5;
    }
    else if (m_y_portion_offset == ATTACH_POS_BOTTOM)
    {
        point.y = y_max - m_inside_offset;
        point.vn = 7;
    }
    else
    {
        point.y = y_min + (m_y_portion_offset * (y_max - y_min));
    }

    return point;
}


ConnDirFlags ShapeConnectionPin::directions(void) const
{
    ConnDirFlags visDir = m_visibility_directions;
    if (m_visibility_directions == ConnDirNone)
    {
        // None is set, use the defaults:
        if (m_x_portion_offset == ATTACH_POS_LEFT)
        {
            visDir |= ConnDirLeft;
        }
        else if (m_x_portion_offset == ATTACH_POS_RIGHT)
        {
            visDir |= ConnDirRight;
        }

        if (m_y_portion_offset == ATTACH_POS_TOP)
        {
            visDir |= ConnDirUp;
        }
        else if (m_y_portion_offset == ATTACH_POS_BOTTOM)
        {
            visDir |= ConnDirDown;
        }

        if (visDir == ConnDirNone)
        {
            visDir = ConnDirAll;
        }
    }
    return visDir;
}

void ShapeConnectionPin::outputCode(FILE *fp) const
{
    COLA_ASSERT(m_shape || m_junction);
    if (m_shape)
    {
        fprintf(fp, "    new ShapeConnectionPin(shapeRef%u, %u, %g, %g, %g, "
                "(ConnDirFlags) %u);\n", m_shape->id(), m_class_id, 
                m_x_portion_offset, m_y_portion_offset, m_inside_offset,
                (unsigned int) m_visibility_directions);
    }
    else if (m_junction)
    {
        fprintf(fp, "    new ShapeConnectionPin(junctionRef%u, %u, "
                "(ConnDirFlags) %u);\n", m_junction->id(), m_class_id, 
                (unsigned int) m_visibility_directions);
    }
}

unsigned int ShapeConnectionPin::containingObjectId(void) const
{
    COLA_ASSERT(m_shape || m_junction);
    return (m_shape) ? m_shape->id() : m_junction->id();
}

bool ShapeConnectionPin::operator==(const ShapeConnectionPin& rhs) const
{
    COLA_ASSERT(m_router == rhs.m_router);

    if (containingObjectId() != rhs.containingObjectId())
    {
        return false;
    }

    // The shape/junction is equal, so examine the unique members.
    if (m_class_id != rhs.m_class_id)
    {
        return false;
    }
    if (m_visibility_directions != rhs.m_visibility_directions)
    {
        return false;
    }
    if (m_x_portion_offset != rhs.m_x_portion_offset)
    {
       return false;
    }
    if (m_y_portion_offset != rhs.m_y_portion_offset)
    {
       return false;
    }
    if (m_inside_offset != rhs.m_inside_offset)
    {
       return false;
    } 
    return true;  
}

bool ShapeConnectionPin::operator<(const ShapeConnectionPin& rhs) const
{
    COLA_ASSERT(m_router == rhs.m_router);

    if (containingObjectId() != rhs.containingObjectId())
    {
        return containingObjectId() < rhs.containingObjectId();
    }
    // Note: operator< is used for set ordering within each shape or junction,
    // so the m_shape/m_junction values should match and we needn't perform the
    // above test in most cases and could just assert the following:
    // COLA_ASSERT(m_shape == rhs.m_shape);
    // COLA_ASSERT(m_junction == rhs.m_junction);

    if (m_class_id != rhs.m_class_id)
    {
        return m_class_id < rhs.m_class_id;
    }
    if (m_visibility_directions != rhs.m_visibility_directions)
    {
        return m_visibility_directions < rhs.m_visibility_directions;
    }
    if (m_x_portion_offset != rhs.m_x_portion_offset)
    {
        return m_x_portion_offset < rhs.m_x_portion_offset;
    }
    if (m_y_portion_offset != rhs.m_y_portion_offset)
    {
        return m_y_portion_offset < rhs.m_y_portion_offset;
    }
    if (m_inside_offset != rhs.m_inside_offset)
    {
        return m_inside_offset < rhs.m_inside_offset;
    }

    // Otherwise, they are considered the same.
    return false;
} 


//============================================================================

}


