/*
 * vim: ts=4 sw=4 et tw=0 wm=0
 *
 * libavoid - Fast, Incremental, Object-avoiding Line Router
 *
 * Copyright (C) 2004-2010  Monash University
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

//! @file    obstacle.h
//! @brief   Contains the interface for the Obstacle class, 
//!          the superclass for ShapeRef and JunctionRef.


#ifndef AVOID_OBSTACLE_H
#define AVOID_OBSTACLE_H

#include <list>
#include <set>

#include <cstdio>

#include "libavoid/geometry.h"
#include "libavoid/connectionpin.h"


namespace Avoid {

class VertInf;
class Router;
class Obstacle;
class ConnEnd;
typedef std::list<Obstacle *> ObstacleList;


// @brief   The Obstacle class represents an obstacle that must be 
//          routed around..
//
class Obstacle
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
        Obstacle(Router *router, Polygon poly, const unsigned int id = 0);
        //! @brief  Shape reference destructor.
        //!
        //! This will call Router::removeShape() for this shape, if this has
        //! not already be called.
        virtual ~Obstacle();
        
        //! @brief   Returns the ID of this shape.
        //! @returns The ID of the shape. 
        unsigned int id(void) const;
        //! @brief   Returns a reference to the polygon boundary of this shape.
        //! @returns A reference to the polygon boundary of the shape.
        const Polygon& polygon(void) const;
        //! @brief   Returns a pointer to the router scene this shape is in.
        //! @returns A pointer to the router scene for this shape.
        Router *router(void) const;
        //! @brief   Returns the position of this junction.
        //! @returns A point representing the position of this junction.
        virtual Point position(void) const = 0;
        
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
       
        virtual void outputCode(FILE *fp) const = 0;
        void makeActive(void);
        void makeInactive(void);
        bool isActive(void) const;
        void updatePinPolyLineVisibility(void);

        void removeFromGraph(void);
        void markForMove(void);
        void clearMoveMark(void);
        Point shapeCentre(void);

        VertInf *getPointVertex(const Point& point);

        void addFollowingConnEnd(ConnEnd *connEnd);
        void removeFollowingConnEnd(ConnEnd *connEnd);
        unsigned int addConnectionPin(ShapeConnectionPin *pin);
        void removeConnectionPin(ShapeConnectionPin *pin);
        void assignPinVisibilityTo(const unsigned int pinClassId, 
                VertInf *dummyConnectionVert);
        std::vector<Point> possiblePinPoints(unsigned int pinClassId) const;

    protected:
        Router *m_router;
        unsigned int m_id;
        Polygon m_polygon;
        bool m_active;
        bool m_in_move_list;
        ObstacleList::iterator m_router_obstacles_pos;
        VertInf *m_first_vert;
        VertInf *m_last_vert;
        std::set<ConnEnd *> m_following_conns;
        ShapeConnectionPinSet m_connection_pins;
};


}


#endif


