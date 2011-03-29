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

//! @file    junction.h
//! @brief   Contains the interface for the JunctionRef class.


#ifndef AVOID_JUNCTION_H
#define AVOID_JUNCTION_H

#include <list>
#include <set>

#include "libavoid/geomtypes.h"
#include "libavoid/obstacle.h"

namespace Avoid {

class Router;
class VertInf;
class ConnEnd;
class ConnRef;
class ShapeConnectionPin;
class JunctionRef;
typedef std::list<JunctionRef *> JunctionRefList;


//! @brief  The JunctionRef class represents a fixed or free-floating point that 
//!         connectors can be attached to.  
//!
//! A JunctionRef represents a junction between multiple connectors, or could
//! be used to specify an intermediate point that a single connector must route
//! through.
//!
class JunctionRef : public Obstacle
{
    public:
        //! @brief  Junction reference constructor.
        //!
        //! Creates a junction object reference, and adds it to the router
        //! scene.  This junction will be considered to be an obstacle.
        //! This will cause connectors intersecting the newly added shape
        //! to be marked as needing to be rerouted.
        //!
        //! If the router is using transactions, then changes will occur
        //! the next time Router::processTransaction() is called.  See
        //! Router::setTransactionUse() for more information.
        //!
        //! The junction can be moved with Router::moveJunction() and removed
        //! from the scene and freed with Router::deleteJunction().
        //!
        //! If an ID is not specified, then one will be assigned to the 
        //! junction.  If assigning an ID yourself, note that it should be 
        //! a unique positive integer.  Also, IDs are given to all objects 
        //! in a scene, so the same ID cannot be given to a shape and a 
        //! connector for example.
        //!
        //! @param[in]  router   The router scene to place the junction into.
        //! @param[in]  position A Point representing the position of the 
        //!                      junction.
        //! @param[in]  id       A unique positive integer ID for the junction.  
        JunctionRef(Router *router, Point position, const unsigned int id = 0);

        //! @brief  Junction reference destructor.
        //!
        //! Do not call this yourself, instead call Router::deleteJunction().
        //! Ownership of this object belongs to the router scene.
        virtual ~JunctionRef();

        //! @brief  Removes a junction that has only two connectors attached
        //!         to it and merges them into a single connector.
        //!
        //! The junction and one of the connectors will be removed from the
        //! router scene and the connector deleted.  A pointer to the 
        //! remaining (merged) connector will be returned by this method.
        //!
        //! Currently this method does not delete and free the Junction itself.
        //! The user needs to do this after the transaction has been 
        //! processed by the router.
        //!
        //! If there are more than two connectors attached to the junction
        //! then nothing will be changed and this method will return NULL.
        //!
        //! @return  The merged connector, or NULL if the junction was not
        //!          removed.
        ConnRef *removeJunctionAndMergeConnectors(void);

        //! @brief   Returns the position of this junction.
        //! @returns A point representing the position of this junction.
        Point position(void) const;

        Rectangle makeRectangle(Router *router, const Point& position);
        void preferOrthogonalDimension(const size_t dim);

    private:
        friend class Router;
        friend class ShapeConnectionPin;
        friend class ConnEnd;

        void outputCode(FILE *fp) const;
        void setPosition(const Point& position);
        void moveAttachedConns(const Point& newPosition);
        void assignPinVisibilityTo(const unsigned int pinClassId, 
                VertInf *dummyConnectionVert);

        Point m_position;
};


}


#endif


