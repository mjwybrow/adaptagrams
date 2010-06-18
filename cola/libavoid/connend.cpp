/*
 * vim: ts=4 sw=4 et tw=0 wm=0
 *
 * libavoid - Fast, Incremental, Object-avoiding Line Router
 *
 * Copyright (C) 2004-2009  Monash University
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

#include <cstring>
#include <cmath>
#include <cstdlib>
#include <cfloat>

#include "libavoid/connend.h"
#include "libavoid/connector.h"
#include "libavoid/connectionpin.h"
#include "libavoid/junction.h"
#include "libavoid/shape.h"
#include "libavoid/debug.h"
#include "libavoid/graph.h"


namespace Avoid {

    
ConnEnd::ConnEnd(const Point& point) 
    : m_point(point),
      m_directions(ConnDirAll),
      m_connection_pin_class_id(CONNECTIONPIN_UNSET),
      m_shape_ref(NULL),
      m_junction_ref(NULL),
      m_conn_ref(NULL),
      m_active_pin(NULL)
{
}


ConnEnd::ConnEnd(const Point& point, const ConnDirFlags visDirs) 
    : m_point(point),
      m_directions(visDirs),
      m_connection_pin_class_id(CONNECTIONPIN_UNSET),
      m_shape_ref(NULL),
      m_junction_ref(NULL),
      m_conn_ref(NULL),
      m_active_pin(NULL)
{
}

ConnEnd::ConnEnd(ShapeRef *shapeRef, const double x_pos, const double y_pos,
        const double insideOffset, const ConnDirFlags visDirs) 
    : m_directions(visDirs),
      m_shape_x_position(x_pos),
      m_shape_y_position(y_pos),
      m_shape_inside_offset(insideOffset),
      m_connection_pin_class_id(CONNECTIONPIN_UNSET),
      m_shape_ref(shapeRef),
      m_junction_ref(NULL),
      m_conn_ref(NULL),
      m_active_pin(NULL)
{
}

ConnEnd::ConnEnd(ShapeRef *shapeRef, const unsigned int connectionPinClassID)
    : m_directions(ConnDirAll),
      m_shape_x_position(ATTACH_POS_CENTRE),
      m_shape_y_position(ATTACH_POS_CENTRE),
      m_shape_inside_offset(0.0),
      m_connection_pin_class_id(connectionPinClassID),
      m_shape_ref(shapeRef),
      m_junction_ref(NULL),
      m_conn_ref(NULL),
      m_active_pin(NULL)
{
    COLA_ASSERT(m_shape_ref != NULL);
    COLA_ASSERT(m_connection_pin_class_id != CONNECTIONPIN_UNSET);
}


ConnEnd::ConnEnd(JunctionRef *junctionRef)
    : m_directions(ConnDirAll),
      m_shape_x_position(ATTACH_POS_CENTRE),
      m_shape_y_position(ATTACH_POS_CENTRE),
      m_shape_inside_offset(0.0),
      m_connection_pin_class_id(CONNECTIONPIN_CENTRE),
      m_shape_ref(NULL),
      m_junction_ref(junctionRef),
      m_conn_ref(NULL),
      m_active_pin(NULL)
{
    COLA_ASSERT(m_junction_ref != NULL);
}


ConnEnd::~ConnEnd()
{
    disconnect();
    freeActivePin();
}


bool ConnEnd::isPinConnection(void) const
{
    return (m_shape_ref || m_junction_ref) &&
           (m_connection_pin_class_id != CONNECTIONPIN_UNSET);
}

unsigned int ConnEnd::type(void) const
{
    COLA_ASSERT(m_conn_ref != NULL);
    return (m_conn_ref->m_dst_connend == this) ? VertID::tar : VertID::src;
}


// Marks this ConnEnd as using a particular ShapeConnectionPin.
void ConnEnd::usePin(ShapeConnectionPin *pin)
{
    COLA_ASSERT(m_active_pin == NULL);
    
    m_active_pin = pin;
    if (m_active_pin)
    {
        m_active_pin->m_connend_users.insert(this);
    }
}


// Marks this ConnEnd as using a particular ShapeConnectionPin's vertex.
void ConnEnd::usePinVertex(VertInf *pinVert)
{
    COLA_ASSERT(m_active_pin == NULL);
    
    if (m_shape_ref)
    {
        for (std::set<ShapeConnectionPin *>::iterator curr = 
                m_shape_ref->m_connection_pins.begin(); 
                curr != m_shape_ref->m_connection_pins.end(); ++curr)
        {
            ShapeConnectionPin *currPin = *curr;
            if (currPin->m_vertex == pinVert)
            {
                usePin(currPin);
                break;
            }
        }
    }
    else if (m_junction_ref)
    {
        for (std::set<ShapeConnectionPin *>::iterator curr = 
                m_junction_ref->m_connection_pins.begin(); 
                curr != m_junction_ref->m_connection_pins.end(); ++curr)
        {
            ShapeConnectionPin *currPin = *curr;
            if (currPin->m_vertex == pinVert)
            {
                usePin(currPin);
                break;
            }
        }
    }
}


// Returns a vector of the possible Points for this ConnEnd
std::vector<Point> ConnEnd::possiblePinPoints(void) const
{
    std::vector<Point> points;
    if (!(m_shape_ref || m_junction_ref) || 
            (m_connection_pin_class_id == CONNECTIONPIN_UNSET))
    {
        return points;
    }

    if (m_shape_ref)
    {
        for (std::set<ShapeConnectionPin *>::iterator curr = 
                m_shape_ref->m_connection_pins.begin(); 
                curr != m_shape_ref->m_connection_pins.end(); ++curr)
        {
            ShapeConnectionPin *currPin = *curr;
            if ((currPin->m_class_id == m_connection_pin_class_id) && 
                    (!currPin->m_exclusive || currPin->m_connend_users.empty()))
            {
                points.push_back(currPin->m_vertex->point);
            }
        }
    }
    else if (m_junction_ref)
    {
        points.push_back(m_junction_ref->position());
    }
    return points;
}

// Marks this ConnEnd as no longer using the active ShapeConnectionPin
void ConnEnd::freeActivePin(void)
{
    if (m_active_pin)
    {
        m_active_pin->m_connend_users.erase(this);
    }
    m_active_pin = NULL;
}


// Creates the connection between a connector and a shape/junction.
void ConnEnd::connect(ConnRef *conn)
{
    COLA_ASSERT(m_conn_ref == NULL);

    if (m_shape_ref)
    {
        m_shape_ref->addFollowingConnEnd(this);
        m_conn_ref = conn;
    }
    else if (m_junction_ref)
    {
        m_junction_ref->addFollowingConnEnd(this);
        m_conn_ref = conn;
    }
}


// Removes the connection between a connector and a shape/junction.
void ConnEnd::disconnect(const bool shapeDeleted)
{
    if (m_conn_ref == NULL)
    {
        // Not connected.
        return;
    }

    m_point = position();
    if (m_shape_ref)
    {
        m_shape_ref->removeFollowingConnEnd(this);
    }
    else if (m_junction_ref)
    {
        m_junction_ref->removeFollowingConnEnd(this);
    }
    m_conn_ref = NULL;

    if (shapeDeleted)
    {
        // Turn this into a manual ConnEnd.
        m_point = position();
        m_shape_ref = NULL;
        m_junction_ref = NULL;
    }
}


const Point ConnEnd::position(void) const
{
    if (m_active_pin)  // Attached to a pin!
    {
        return m_active_pin->position();
    }
    else if (m_junction_ref)
    {
        return m_junction_ref->position();
    }
    else if (m_shape_ref)  // Not attached to a pin.
    {
        if (m_connection_pin_class_id != CONNECTIONPIN_UNSET)
        {
            db_printf("Warning: A valid pin connection could not be found.  "
                    "Using the shape centre.\n");
        }

        const Polygon& poly = m_shape_ref->polygon();

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
        // or possibly slightly inside them (if _insideOfset is set).

        point.vn = kUnassignedVertexNumber;
        if (m_shape_x_position == ATTACH_POS_LEFT)
        {
            point.x = x_min + m_shape_inside_offset;
            point.vn = 6;
        }
        else if (m_shape_x_position == ATTACH_POS_RIGHT)
        {
            point.x = x_max - m_shape_inside_offset;
            point.vn = 4;
        }
        else
        {
            point.x = x_min + (m_shape_x_position * (x_max - x_min));
        }

        if (m_shape_y_position == ATTACH_POS_TOP)
        {
            point.y = y_max - m_shape_inside_offset;
            point.vn = 5;
        }
        else if (m_shape_y_position == ATTACH_POS_BOTTOM)
        {
            point.y = y_min + m_shape_inside_offset;
            point.vn = 7;
        }
        else
        {
            point.y = y_min + (m_shape_y_position * (y_max - y_min));
            point.vn = kUnassignedVertexNumber;
        }

        return point;
    }
    else
    {
        return m_point;
    }
}


ConnDirFlags ConnEnd::directions(void) const
{
    if (m_active_pin)  // Attached to a pin!
    {
        return m_active_pin->directions();
    }
    else if (m_shape_ref)  // Not attached to a pin.
    {
        ConnDirFlags visDir = m_directions;
        if (m_directions == ConnDirNone)
        {
            // None is set, use the defaults:
            if (m_shape_x_position == ATTACH_POS_LEFT)
            {
                visDir |= ConnDirLeft;
            }
            else if (m_shape_x_position == ATTACH_POS_RIGHT)
            {
                visDir |= ConnDirRight;
            }

            if (m_shape_y_position == ATTACH_POS_TOP)
            {
                visDir |= ConnDirDown;
            }
            else if (m_shape_y_position == ATTACH_POS_BOTTOM)
            {
                visDir |= ConnDirUp;
            }

            if (visDir == ConnDirNone)
            {
                visDir = ConnDirAll;
            }
        }
        return visDir;
    }
    else
    {
        return m_directions;
    }
}


// Assign visibility to a dummy vertex representing all the possible pins
// for this pinClassId.
void ConnEnd::assignPinVisibilityTo(VertInf *dummyConnectionVert)
{
    COLA_ASSERT(m_shape_ref || m_junction_ref);
    COLA_ASSERT(m_connection_pin_class_id != CONNECTIONPIN_UNSET);

    if (m_shape_ref)
    {
        for (std::set<ShapeConnectionPin *>::iterator curr = 
                m_shape_ref->m_connection_pins.begin(); 
                curr != m_shape_ref->m_connection_pins.end(); ++curr)
        {
            ShapeConnectionPin *currPin = *curr;
            if ((currPin->m_class_id == m_connection_pin_class_id) && 
                    (!currPin->m_exclusive || currPin->m_connend_users.empty()))
            {
                // This has same ID and is either unconnected or not 
                // exclusive, so give it visibility.
                EdgeInf *edge = new EdgeInf(dummyConnectionVert,
                        currPin->m_vertex, true);
                // XXX Can't use a zero cost due to assumptions 
                //     elsewhere in code.
                edge->setDist(manhattanDist(dummyConnectionVert->point,
                            currPin->m_vertex->point) + 
                        std::max(0.001, currPin->m_connection_cost));
            }
        }
    }
    else if (m_junction_ref)
    {
        for (std::set<ShapeConnectionPin *>::iterator curr = 
                m_junction_ref->m_connection_pins.begin(); 
                curr != m_junction_ref->m_connection_pins.end(); ++curr)
        {
            ShapeConnectionPin *currPin = *curr;
            if ((currPin->m_class_id == m_connection_pin_class_id) && 
                    (!currPin->m_exclusive || currPin->m_connend_users.empty()))
            {
                // This has same ID and is either unconnected or not 
                // exclusive, so give it visibility.
                EdgeInf *edge = new EdgeInf(dummyConnectionVert,
                        currPin->m_vertex, true);
                // XXX Can't use a zero cost due to assumptions 
                //     elsewhere in code.
                edge->setDist(manhattanDist(dummyConnectionVert->point,
                            currPin->m_vertex->point) + 
                        std::max(0.001, currPin->m_connection_cost));
            }
        }
    }
}


void ConnEnd::outputCode(FILE *fp, const char *srcDst)
{
    if (m_junction_ref)
    {
        fprintf(fp, "    ConnEnd %sPt%u(junctionRef%u);\n", srcDst,
                m_conn_ref->id(), m_junction_ref->id());
    }
    else if (m_shape_ref)
    {
        fprintf(fp, "    ConnEnd %sPt%u(shapeRef%u, %u);\n", srcDst,
                m_conn_ref->id(), m_shape_ref->id(), m_connection_pin_class_id);
    }
}


//============================================================================

}


