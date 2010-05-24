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


namespace Avoid {

    
ConnEnd::ConnEnd(const Point& point) 
    : _point(point),
      _directions(ConnDirAll),
      _shapeRef(NULL),
      _connRef(NULL)
{
}


ConnEnd::ConnEnd(const Point& point, const ConnDirFlags visDirs) 
    : _point(point),
      _directions(visDirs),
      _shapeRef(NULL),
      _connRef(NULL)
{
}

ConnEnd::ConnEnd(ShapeRef *shapeRef, const double x_pos, const double y_pos,
        const double insideOffset, const ConnDirFlags visDirs) 
    : _directions(visDirs),
      _xPosition(x_pos),
      _yPosition(y_pos),
      _insideOffset(insideOffset),
      _shapeRef(shapeRef),
      _connRef(NULL)
{
}


ConnEnd::~ConnEnd()
{
    disconnect();
}


unsigned int ConnEnd::type(void) const
{
    COLA_ASSERT(_connRef != NULL);
    return (_connRef->_dstConnEnd == this) ? VertID::tar : VertID::src;
}


// Creates the connection between a connector and a shape.
void ConnEnd::connect(ConnRef *conn)
{
    COLA_ASSERT(_connRef == NULL);

    if (_shapeRef == NULL)
    {
        // This is a manual connector end;
        return;
    }

    _shapeRef->addFollowingConnEnd(this);
    _connRef = conn;
}


// Removes the connection between a connector and a shape.
void ConnEnd::disconnect(const bool shapeDeleted)
{
    if (_connRef == NULL)
    {
        // Not connected.
        return;
    }

    _point = position();
    _shapeRef->removeFollowingConnEnd(this);
    _connRef = NULL;

    if (shapeDeleted)
    {
        // Turn this into a manual ConnEnd.
        _point = position();
        _shapeRef = NULL;
    }
}


ShapeRef *ConnEnd::containingShape(void) const
{
    return _shapeRef;
}


const Point ConnEnd::position(void) const
{
    if (_shapeRef)
    {
        const Polygon& poly = _shapeRef->polygon();

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
        if (_xPosition == ATTACH_POS_LEFT)
        {
            point.x = x_min + _insideOffset;
            point.vn = 6;
        }
        else if (_xPosition == ATTACH_POS_RIGHT)
        {
            point.x = x_max - _insideOffset;
            point.vn = 4;
        }
        else
        {
            point.x = x_min + (_xPosition * (x_max - x_min));
        }

        if (_yPosition == ATTACH_POS_TOP)
        {
            point.y = y_max - _insideOffset;
            point.vn = 5;
        }
        else if (_yPosition == ATTACH_POS_BOTTOM)
        {
            point.y = y_min + _insideOffset;
            point.vn = 7;
        }
        else
        {
            point.y = y_min + (_yPosition * (y_max - y_min));
            point.vn = kUnassignedVertexNumber;
        }

        return point;
    }
    else
    {
        return _point;
    }
}


ConnDirFlags ConnEnd::directions(void) const
{
    if (_shapeRef)
    {
        ConnDirFlags visDir = _directions;
        if (_directions == ConnDirNone)
        {
            // None is set, use the defaults:
            if (_xPosition == ATTACH_POS_LEFT)
            {
                visDir |= ConnDirLeft;
            }
            else if (_xPosition == ATTACH_POS_RIGHT)
            {
                visDir |= ConnDirRight;
            }

            if (_yPosition == ATTACH_POS_TOP)
            {
                visDir |= ConnDirDown;
            }
            else if (_yPosition == ATTACH_POS_BOTTOM)
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
        return _directions;
    }
}


//============================================================================

}


