/*
 * vim: ts=4 sw=4 et tw=0 wm=0
 *
 * libavoid - Fast, Incremental, Object-avoiding Line Router
 *
 * Copyright (C) 2011  Monash University
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

//! @file    hyperedge.h
//! @brief   Contains the interface for the HyperedgeRerouter class.

#ifndef AVOID_HYPEREDGE_H
#define AVOID_HYPEREDGE_H

#include <cstdio>
#include <list>
#include <vector>
#include <set>

namespace Avoid {

class ConnRef;
class JunctionRef;
class Router;
class ConnEnd;
class VertInf;

typedef std::list<ConnEnd> ConnEndList;
typedef std::list<ConnRef *> ConnRefList;
typedef std::list<JunctionRef *> JunctionRefList;
typedef std::list<VertInf *> VertexList;

typedef std::set<ConnRef *> ConnRefSet;
typedef std::set<VertInf *> VertexSet;

typedef std::vector<JunctionRef *> JunctionRefVector;
typedef std::vector<ConnEndList> ConnEndListVector;
typedef std::vector<ConnRefList> ConnRefListVector;
typedef std::vector<JunctionRefList> JunctionRefListVector;
typedef std::vector<VertexSet> VertexSetVector;


//! @brief   The HyperedgeRerouter class is a convenience object that can be
//!          used to register hyperedges to be rerouted, improving the
//!          placement of their junctions and connector paths.
//!
//! To work with this class, you should get a copy from the router instance
//! via a call to Router::hyperedgeRerouter().
//!
//! If you would like a particular hyperedge to be completely rerouted with
//! new junction positions then you should register it with this class via a
//! call to registerHyperedgeForRerouting.  A hyperedge can either be
//! specified as a set of terminal vertices, or as a single JunctionRef.
//! Passing a JunctionRef will cause HyperedgeRerouter to follow the attached
//! connectors and junctions to determine the hyperedge.  When you register
//! a hyperedge you get an index number that can be used to later find
//! information about it.
//!
//! The Rerouting will actually occur the next time the Router processes a
//! transaction, see Router::processTransaction().  The rerouting will
//! effectively create new junctions (JunctionRefs) and connectors (ConnRefs)
//! for the hyperedge.  You can get a list of the new junctions by calling
//! newJunctionList() and the connectors by calling newConnectorList().
//!
//! If you specified a connector via a JunctionRef and it was therefore
//! composed of existing ConnRefs and JunctionRefs you can get a list of these
//! by calling deletedConnectorList() and deletedJunctionList().  After
//! the transaction has been processed you should not use these references
//! any more from your own code (the router will free them at it's
//! convenience) and you should refer only to the new connectors and junctions.
//!
class HyperedgeRerouter
{
    public:
        //! @brief  Constructor for hyperedge rerouter object.
        //!
        //! @note   You shouldn't create this object yourself.  The Router
        //!         instance has one that you can request a reference to
        //!         via Router::hyperedgeRerouter().
        //!
        HyperedgeRerouter();
        
        //! @brief  Registers a hyperedge to be fully rerouted the next time
        //!         the router processes a transaction.
        //!
        //! @param[in] terminals  The ConnEnds that form the endpoints of the
        //!                       hyperedge.
        //! @return An index that can be used to request information on the
        //!         resulting routing of the hyperedge.
        //!
        size_t registerHyperedgeForRerouting(ConnEndList terminals);

        //! @brief  Registers a hyperedge to be fully rerouted the next time
        //!         the router processes a transaction.
        //!
        //! In this case the connectors and junctions attached to the given
        //! junction will be traversed to determine the endpoints of the
        //! hyperedge.  These endpoints will then be used for the rerouting.
        //! The junctions and connectors forming the old route will be
        //! deleted.
        //!
        //! @param[in] junction  One of the junctions that forms the
        //!                      hyperedge.
        //! @return An index that can be used to request information on the
        //!         resulting routing of the hyperedge.
        //!
        size_t registerHyperedgeForRerouting(JunctionRef *junction);

        //! @brief  A list of the new junctions created for this hyperedge's
        //!         path during rerouting.
        //!
        //! This method will only return information once the router has
        //! processed the transaction.
        //!
        //! @param  index  The index of the hyperedge to return junctions for.
        //! @return A list of created JunctionRefs for the hyperedge.
        //!
        JunctionRefList newJunctionList(size_t index) const;

        //! @brief  A list of the new connectors created for this hyperedge's
        //!         path during rerouting.
        //!
        //! This method will only return information once the router has
        //! processed the transaction.
        //!
        //! @param  index  The index of the hyperedge to return connectors for.
        //! @return A list of created ConnRefs for the hyperedge.
        //!
        ConnRefList newConnectorList(size_t index) const;

        //! @brief  A list of the old junctions comprising a hyperedge's
        //!         path which were deleted during rerouting.
        //!
        //! Should should call this after the router has processed the
        //! transaction and then you should no longer refer to any of the
        //! objects returned.  The router will delete and free their memory
        //! at its convenience.
        //!
        //! @param  index  The index of the hyperedge to return junctions for.
        //! @return A list of JunctionRefs deleted from the hyperedge.
        //!
        JunctionRefList deletedJunctionList(size_t index) const;

        //! @brief  A list of the old connectors comprising a hyperedge's
        //!         path which were deleted during rerouting.
        //!
        //! Should should call this after the router has processed the
        //! transaction and then you should no longer refer to any of the
        //! objects returned.  The router will delete and free their memory
        //! at its convenience.
        //!
        //! @param  index  The index of the hyperedge to return connectors for.
        //! @return A list of ConnRefs deleted from the hyperedge.
        //!
        ConnRefList deletedConnectorList(size_t index) const;

    private:
        friend class Router;

        // @brief  The number of hyperedges that are being or have been
        //         rerouted.
        // @return The number of rerouted hyperedges.
        //
        size_t count(void) const;
        // @brief  Sets the router instance that this object operates on.
        //
        // @param[in] router  The router instance to operate on.
        //
        void setRouter(Router *router);

        ConnRefSet calcHyperedgeConnectors(void);
        // Called by Router during processTransaction().
        void performRerouting(void);
        void outputInstanceToSVG(FILE *fp);
        void findAttachedObjects(size_t index, ConnRef *connector,
                JunctionRef *ignore, ConnRefSet& hyperedgeConns);
        void findAttachedObjects(size_t index, JunctionRef *junction,
                ConnRef *ignore, ConnRefSet& hyperedgeConns);

        Router *m_router;
        ConnEndListVector m_terminals_vector;
        JunctionRefVector m_root_junction_vector;
        JunctionRefListVector m_new_junctions_vector;
        JunctionRefListVector m_deleted_junctions_vector;
        ConnRefListVector m_new_connectors_vector;
        ConnRefListVector m_deleted_connectors_vector;
        VertexSetVector m_terminal_vertices_vector;
        VertexList m_added_vertices;
};


}

#endif
