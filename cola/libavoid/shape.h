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

//! @file    shape.h
//! @brief   Contains the interface for the ShapeRef class.


#ifndef AVOID_SHAPE_H
#define AVOID_SHAPE_H

#include <list>
#include <set>

#include "libavoid/geometry.h"


namespace Avoid {

class VertInf;
class Router;
class ShapeRef;
class ConnEnd;
class ShapeConnectionPin;
typedef std::list<ShapeRef *> ShapeRefList;


//! @brief   The ShapeRef class represents a shape object.
//!
//! Shapes are obstacles that connectors must be routed around.  They can be 
//! placed into a Router scene and can be repositioned or resized (via
//! Router::moveShape()).
//! 
//! Usually, it is expected that you would create a ShapeRef for each shape 
//! in your diagram and keep that reference in your own shape class.
//!
class ShapeRef
{
    public:
        //! @brief  Shape reference constructor.
        //!
        //! Creates a shape obect reference, but does not yet place it into the
        //! Router scene.  You can add or remove the shape to/from the scene 
        //! with Router::addJunction() and Router::removeJunction().  The 
        //! junction can be moved with Router::moveJunction().
        //!
        //! The poly argument will usually be the boundary of the shape in your 
        //! application with additional buffer of several pixels on each side.
        //! Specifying such a buffer results in connectors leaving a small 
        //! amount of space around shapes, rather than touching them on the 
        //! corners or edges.
        //!
        //! If an ID is not specified, then one will be assigned to the shape.
        //! If assigning an ID yourself, note that it should be a unique 
        //! positive integer.  Also, IDs are given to all objects in a scene,
        //! so the same ID cannot be given to a shape and a connector for 
        //! example.
        //!
        //! @param[in]  router  The router scene to place the shape into.
        //! @param[in]  poly    A Polygon representing the boundary of the 
        //!                     shape.
        //! @param[in]  id      A unique positive integer ID for the shape.  
        ShapeRef(Router *router, Polygon& poly, const unsigned int id = 0);
        //! @brief  Shape reference destructor.
        //!
        //! This will call Router::removeShape() for this shape, if this has
        //! not already be called.
        ~ShapeRef();
        
        //! @brief   Returns the ID of this shape.
        //! @returns The ID of the shape. 
        unsigned int id(void) const;
        //! @brief   Returns a reference to the polygon boundary of this shape.
        //! @returns A reference to the polygon boundary of the shape.
        const Polygon& polygon(void) const;
        //! @brief   Returns a pointer to the router scene this shape is in.
        //! @returns A pointer to the router scene for this shape.
        Router *router(void) const;
        
#if 0
        //! @brief Constructs a ShapeConnectionPin at a specified position on a
        //!        parent shape.
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
        //! <pre>  createConnectionPin(shapeRef, pinClassId, ATTACH_POS_RIGHT, ATTACH_POS_CENTRE, 5);</pre>
        //! rather than
        //! <pre>  createConnectionPin(shapeRef, pinClassId, 1 - 5/shapeWidth, ATTACH_POS_CENTRE, 0, ConnDirRight);</pre>
        //! 
        //! @param[in]  classId        An integer used to mark the class or group
        //!                            of this connection point, used for 
        //!                            specifying attachment to ConnEnd.
        //! @param[in]  xPortionOffset The X position within the shape, 
        //!                            specified as a proportion of the width
        //!                            between 0 (left) and 1 (right).
        //! @param[in]  yPortionOffset The Y position within the shape, 
        //!                            specified as a proportion of the height
        //!                            between 0 (top) and 1 (bottom).  
        //! @param[in]  insideOffset   A distance to offset the connection point 
        //!                            inside the shape if it lies on the 
        //!                            boundary.
        //! @param[in]  visDirs        One or more Avoid::ConnDirFlag options 
        //!                            specifying the directions that this 
        //!                            connection point has visibility. 
        //!
        ShapeConnectionPin *createConnectionPin(const unsigned int classId,
                const double xPortionOffset, const double yPortionOffset, 
                const double insideOffset = 0.0,
                const ConnDirFlags visDirs = ConnDirNone);
#endif
 
        void setNewPoly(const Polygon& poly);
        VertInf *firstVert(void);
        VertInf *lastVert(void);
        void boundingBox(BBox& bbox);

    private:
        friend class Router;
        friend class ConnEnd;
        friend class ShapeConnectionPin;

        // Defined in visibility.cpp:
        void computeVisibilityNaive(void);
        void computeVisibilitySweep(void);

        void makeActive(void);
        void makeInactive(void);
        bool isActive(void) const;

        void removeFromGraph(void);
        void markForMove(void);
        void clearMoveMark(void);

        VertInf *getPointVertex(const Point& point);

        void addFollowingConnEnd(ConnEnd *connEnd);
        void removeFollowingConnEnd(ConnEnd *connEnd);
        void moveAttachedConns(const Polygon& newPoly);
        unsigned int addConnectionPin(ShapeConnectionPin *pin);
        void removeConnectionPin(ShapeConnectionPin *pin);
        void assignPinVisibilityTo(const unsigned int pinClassId, 
                VertInf *dummyConnectionVert);

        Router *m_router;
        unsigned int m_id;
        Polygon m_polygon;
        bool m_active;
        bool m_in_move_list;
        ShapeRefList::iterator m_shaperefs_pos;
        VertInf *m_first_vert;
        VertInf *m_last_vert;
        std::set<ConnEnd *> m_following_conns;
        std::set<ShapeConnectionPin *> m_connection_pins;
};


}


#endif


