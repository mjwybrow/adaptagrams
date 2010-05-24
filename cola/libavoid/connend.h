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

//! @file    connend.h
//! @brief   Contains the interface for the ConnRef class.


#ifndef AVOID_CONNEND_H
#define AVOID_CONNEND_H


#include <list>
#include <vector>

#include "libavoid/geometry.h"


namespace Avoid {

class Router;
class ConnRef;
class ShapeRef;
typedef std::list<ConnRef *> ConnRefList;


//! @brief  Flags that can be passed to the ConnEnd constructor to specify
//!         which sides of a shape this point should have visibility to if
//!         it is located within the shape's area.
//!
//! Like SVG, libavoid considers the Y-axis to point downwards, that is, 
//! like screen coordinates the coordinates increase from left-to-right and 
//! also from top-to-bottom.
//!
enum ConnDirFlag {
    ConnDirNone  = 0,
    //! @brief  This option specifies the point should be given visibility 
    //!         to the top of the shape that it is located within.
    ConnDirUp    = 1,
    //! @brief  This option specifies the point should be given visibility 
    //!         to the bottom of the shape that it is located within.
    ConnDirDown  = 2,
    //! @brief  This option specifies the point should be given visibility 
    //!         to the left side of the shape that it is located within.
    ConnDirLeft  = 4,
    //! @brief  This option specifies the point should be given visibility 
    //!         to the right side of the shape that it is located within.
    ConnDirRight = 8,
    //! @brief  This option, provided for convenience, specifies the point 
    //!         should be given visibility to all four sides of the shape 
    //!         that it is located within.
    ConnDirAll   = 15
};
//! @brief  One or more Avoid::ConnDirFlag options.
//!
typedef unsigned int ConnDirFlags;


//! @brief  The ConnEnd class represents different possible endpoints for 
//!         connectors.
//!
//! Currently this class just allows free-floating endpoints, but in future
//! will be capable of representing attachment to connection points on shapes.
//! 
class ConnEnd 
{
    public:
        //! @brief Constructs a ConnEnd from a free-floating point.
        //!
        //! @param[in]  point  The position of the connector endpoint.
        //!
        ConnEnd(const Point& point);

        //! @brief Constructs a ConnEnd from a free-floating point as well
        //!        as a set of flags specifying visibility for this point 
        //!        if it is located inside a shape.
        //!
        //! @param[in]  point    The position of the connector endpoint.
        //! @param[in]  visDirs  One or more Avoid::ConnDirFlag options 
        //!                      specifying the directions that this point 
        //!                      should be given visibility if it is inside 
        //!                      a shape.  Currently has no effect if outside
        //!                      of shapes.
        //!
        ConnEnd(const Point& point, const ConnDirFlags visDirs);

        //! @brief Constructs a ConnEnd attached to a position on a shape.
        //!
        //! The connection position should be specified as proportion
        //! of the shape's total width and height using a floating point
        //! value between 0 and 1.
        //!
        //! There are some predefined values for specifying the xPortionOffset
        //! and yPortionOffset arguments:
        //!  -  ATTACH_POS_TOP = 0
        //!  -  ATTACH_POS_LEFT = 0
        //!  -  ATTACH_POS_CENTRE = 0.5
        //!  -  ATTACH_POS_BOTTOM = 1
        //!  -  ATTACH_POS_RIGHT = 1
        //!
        //! Importantly, when this type of ConnEnd is passed to the 
        //! ConnEnd::setEndpoint() methods, that endpoint will subsequently 
        //! get automatically moved whenever the containing shape is moved 
        //! or resized, thus causing the connector to be rerouted in response
        //! to shape movement.  To subsequently detach the connector from the
        //! shape or attach it to a different shape, call one of the 
        //! ConnEnd::setEndpoint() methods with a new ConnEnd.
        //!
        //! If no value is given for the visDirs argument, then visibility is 
        //! automatically determined based on the position of the connection
        //! point.  Points on the shape boundary will have visibility from the
        //! shape out of that edge while points in the interior will have
        //! visibility in all directions.
        //!
        //! The insideOffset argument can be used to set a distance to 
        //! automatically offset the point within the shape.  This is useful
        //! for orthogonal routing, where you usually want the connection 
        //! point to lie inside the shape rather than exactly on its boundary.
        //! While you could specify an exact position with xPortionOffset and 
        //! yPortionOffset, it is usually much easier and more readable to not 
        //! have to specify the visibility directions manually and to write 
        //! something like
        //! <pre>  ConnEnd(shapeRef, ATTACH_POS_RIGHT, ATTACH_POS_CENTRE, 5);</pre>
        //! rather than
        //! <pre>  ConnEnd(shapeRef, 1 - 5/shapeWidth, ATTACH_POS_CENTRE, 0, ConnDirRight);</pre>
        //! 
        //! @param[in]  shapeRef       A pointer to the containing shape's
        //!                            ShapeRef.
        //! @param[in]  xPortionOffset The X position within the shape, 
        //!                            specified as a proportion of the width
        //!                            between 0 (left) and 1 (right).
        //! @param[in]  yPortionOffset The Y position within the shape, 
        //!                            specified as a proportion of the height
        //!                            between 0 (top) and 1 (bottom).  
        //! @param[in]  insideOffset   A distance to offset the point inside
        //!                            the shape if it lies on the boundary.
        //! @param[in]  visDirs        One or more Avoid::ConnDirFlag options 
        //!                            specifying the directions that this 
        //!                            point should be given visibility. 
        //!
        ConnEnd(ShapeRef *shapeRef, const double xPortionOffset, 
                const double yPortionOffset,
                const double insideOffset = 0.0,
                const ConnDirFlags visDirs = ConnDirNone);

        //! @brief Returns the position of this connector endpoint
        //!
        //! @return The position of this connector endpoint.
        //!
        const Point position(void) const;

        //! @brief Returns the directions in which this connector endpoint
        //!        should be given visibility.
        //!
        //! @return The visibility directions for this connector endpoint.
        //!
        ConnDirFlags directions(void) const;

        ~ConnEnd();
    private:
        void connect(ConnRef *conn);
        void disconnect(const bool shapeDeleted = false);
        ShapeRef *containingShape(void) const;
        unsigned int type(void) const;

        Point _point;
        ConnDirFlags _directions;
        double _xPosition;
        double _yPosition;
        double _insideOffset;
        
        // For referencing ConnEns
        ShapeRef *_shapeRef;  // The shape this is attached to.
        ConnRef *_connRef;    // The parent connector.

        friend class ShapeRef;
        friend class ConnRef;
};


}


#endif


