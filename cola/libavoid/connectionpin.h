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
 * Author(s):   Michael Wybrow <mjwybrow@users.sourceforge.net>
*/

//! @file    connectionpin.h
//! @brief   Contains the interface for the ShapeConnectionPin class.


#ifndef AVOID_CONNECTIONPIN_H
#define AVOID_CONNECTIONPIN_H

#include <cstdio>
#include <set>
#include <climits>

#include "libavoid/connend.h"
#include "libavoid/geomtypes.h"

namespace Avoid {


static const unsigned int CONNECTIONPIN_UNSET  = INT_MAX;
static const unsigned int CONNECTIONPIN_CENTRE = INT_MAX - 1;

class Router;
class ShapeRef;
class JunctionRef;
class ConnEnd;
class VertInf;

//! @brief  The ShapeConnectionPin class represents a fixed point or "pin" 
//!         on a shape that can be connected to.
//!
//! Pins have a visibility direction and numeric class ID (e.g., to specify 
//! types of pins such as "input" or "output").  They may optionally have a 
//! priority which means when routing connectors to a particular class ID then
//! one pin will be selected over another.
//! 
//! Pins have a position that is specified relative to its parent shape.  
//! Then the shape is moved or resized, The pin will be automatically moved 
//! accordingly.
//!
//! Pins with a specified direction are exclusive by default, those with
//! visibility in all directions are non-exclusive by default.  This 
//! behaviour can be changed by calling the ShapeConnectionPin::setExclusive() 
//! method.  Exclusive pins may only have a single connector attached to them.
//!
class ShapeConnectionPin
{
    public:
        //! @brief Constructs a ShapeConnectionPin at a specified position on a
        //!        parent shape.
        //!
        //! Ownership of this ShapeConnectionPin is passed to the parent shape.
        //!
        //! The connection point position should be specified as proportion
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
        //! Importantly, shape connection pins will be moved automatically when
        //! the parent shapes are moved or resized.  Attachment for connectors 
        //! will be chosen based on the classId specified to ConnEnd and these
        //! connectors will be subsequently rerouted.
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
        //! @code
        //! ShapeConnectionPin(shapeRef, pinClassId, ATTACH_POS_RIGHT, ATTACH_POS_CENTRE, 5);
        //! @endcode
        //! rather than
        //! @code
        //! ShapeConnectionPin(shapeRef, pinClassId, 1 - 5/shapeWidth, ATTACH_POS_CENTRE, 0, ConnDirRight);
        //! @endcode
        //! 
        //! @param[in]  shape          A pointer to the containing parent 
        //!                            shape's ShapeRef.
        //! @param[in]  classId        A non-zero integer used to denote the
        //!                            class or group of this connection
        //!                            point, used for specifying attachment
        //!                            to ConnEnd.
        //! @param[in]  xPortionOffset The X position within the shape, 
        //!                            specified as a proportion of the width
        //!                            between 0 (left) and 1 (right).
        //! @param[in]  yPortionOffset The Y position within the shape, 
        //!                            specified as a proportion of the height
        //!                            between 0 (top) and 1 (bottom).  
        //! @param[in]  insideOffset   A distance to offset the connection 
        //!                            point inside the shape if it lies on 
        //!                            the boundary.
        //! @param[in]  visDirs        One or more Avoid::ConnDirFlag options 
        //!                            specifying the directions that this 
        //!                            connection point has visibility. 
        //!
        ShapeConnectionPin(ShapeRef *shape, const unsigned int classId,
                const double xPortionOffset, const double yPortionOffset, 
                const double insideOffset = 0.0,
                const ConnDirFlags visDirs = ConnDirNone);
        //! @brief Constructs a ShapeConnectionPin on a JunctionRef.
        //!
        //! Ownership of this ShapeConnectionPin is passed to the parent 
        //! junction.
        //!
        //! This will usually be automatically called by the JunctionRef 
        //! constructor to give the Junction four ShapeConnectionPins, 
        //! facing up, down, left and right.
        //!
        //! @param[in]  junction       A pointer to the containing parent 
        //!                            junction's JunctionRef.
        //! @param[in]  classId        An integer used to mark the class or 
        //!                            group of this connection point, used 
        //!                            for specifying attachment to ConnEnd.
        //! @param[in]  visDirs        One or more Avoid::ConnDirFlag options 
        //!                            specifying the directions that this 
        //!                            connection point has visibility. 
        //!
        ShapeConnectionPin(JunctionRef *junction, const unsigned int classId,
                const ConnDirFlags visDirs = ConnDirNone);
        
        ~ShapeConnectionPin();
        
        //! @brief sets a cost used when selecting whether connectors should be
        //!        be attached to this connection pin.
        //!
        //! @param[in]  cost  A routing cost applied to a route when selecting
        //!                   this connection pin.
        //!
        void setConnectionCost(const double cost);

        //! @brief Returns the position of this connection pin.
        //!
        //! @return The position of this connection pin.
        //!
        const Point position(const Polygon& newPoly = Polygon()) const;

        //! @brief Returns the directions in which this connection pin
        //!        has visibility.
        //!
        //! @return The visibility directions for this connection pin.
        //!
        ConnDirFlags directions(void) const;
        
        //! @brief  Sets whether the pin is exclusive, i.e., only one connector
        //!         can attach to it.
        //!
        //! @param[in]  exclusive  A bool representing whether this pin should
        //!                        be exclusive.
        //!
        void setExclusive(const bool exclusive);

        bool operator==(const ShapeConnectionPin& rhs) const;
        bool operator<(const ShapeConnectionPin& rhs) const;
    private:
        friend class ShapeRef;
        friend class JunctionRef;
        friend class Obstacle;
        friend class ConnEnd;
        friend class Router;
        
        void updatePosition(const Point& newPosition);
        void updatePosition(const Polygon& newPoly);
        void updatePositionAndVisibility(void);
        void updateVisibility(void);
        void outputCode(FILE *fp) const;
        unsigned int containingObjectId(void) const;

        // Unique properties
        Router *m_router;
        ShapeRef *m_shape;
        JunctionRef *m_junction;
        unsigned int m_class_id;
        double m_x_portion_offset;
        double m_y_portion_offset;
        double m_inside_offset;
        ConnDirFlags m_visibility_directions;
        
        // Some extra properties.
        bool m_exclusive;
        double m_connection_cost;
        // The set of connends using this pin.
        std::set<ConnEnd *> m_connend_users;
        VertInf *m_vertex;
};

class CmpConnPinPtr
{
    public:
        CmpConnPinPtr()
        {
        }
        bool operator()(const ShapeConnectionPin *lhs, 
                const ShapeConnectionPin *rhs) const 
        {
            return (*lhs) < (*rhs);
        }
};

typedef std::set<ShapeConnectionPin *, CmpConnPinPtr> ShapeConnectionPinSet;

}


#endif


