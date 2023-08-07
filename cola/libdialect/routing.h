/*
 * vim: ts=4 sw=4 et tw=0 wm=0
 *
 * libdialect - A library for computing DiAlEcT layouts:
 *                 D = Decompose/Distribute
 *                 A = Arrange
 *                 E = Expand/Emend
 *                 T = Transform
 *
 * Copyright (C) 2018  Monash University
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * See the file LICENSE.LGPL distributed with the library.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * Author(s):   Steve Kieffer   <http://skieffer.info>
*/

#ifndef DIALECT_ROUTING_H
#define DIALECT_ROUTING_H

#include <map>
#include <string>
#include <vector>
#include <utility>

#include "libavoid/libavoid.h"

#include "libdialect/commontypes.h"
#include "libdialect/opts.h"
#include "libdialect/ortho.h"
#include "libdialect/logging.h"

namespace dialect {

typedef std::pair<Avoid::ConnDirFlags, Avoid::ConnDirFlags> EdgeConnDirs;
typedef std::map<id_type, EdgeConnDirs> EdgeConnDirsById;

//! Control how much processing should be done on connector routes by the RoutingAdapter.
enum class RouteProcessing {
    //! Don't do anything.
    NONE,
    //! Record the connector routes in the Edges, exactly as returned by the Router.
    RECORD,
    //! Record the connector routes in the Edges after first doing post-processing to
    //! remove extremely small S-bends.
    REFINE_AND_RECORD
};

//! @brief  Adapter to easily apply libavoid::Routers to libdialect::Graphs.
struct RoutingAdapter {

    //! @brief  Standard Constructor.
    //! @param[in] flag  A RouterFlag to say what kind of routing is desired (polyline or orthogonal).
    RoutingAdapter(Avoid::RouterFlag flag) : router(flag) {}

    //! @brief  Add nodes.
    //! @param[in] nodes  The Nodes to be added, given by a NodesById mapping.
    void addNodes(const NodesById &nodes);

    //! @brief  Add edges.
    //! @param[in] edges  The Edges to be added, given by an EdgesById mapping.
    //! @param[in] connDirs  Optional specification of the allowed connection directions at the source and
    //!                      target ends of the Edges. Specifically, an Edge's ID should map to a pair
    //!                      (srcConnDirs, tgtConnDirs) giving the allowed directions for source and target
    //!                      respectively. For any Edge whose ID is not present in the mapping, all connection
    //!                      directions are allowed at both ends.
    void addEdges(const EdgesById &edges, const EdgeConnDirsById *connDirs = nullptr);

    //! @brief  Do the routing.
    //! @param[in] processing  Set to the desired level of route processing. Default: routes are
    //!                        recorded in Edges, without any post-processing.
    //! @sa RouteProcessing enum.
    void route(RouteProcessing processing = RouteProcessing::RECORD);

    //! @brief  Record the routes in the Edges.
    //! @param[in] refine  Say whether the routes should be refined before recording.
    void recordRoutes(bool refine = false);

    //! The Router
    Avoid::Router router;
    //! Lookup for Edges
    EdgesById edges;
    //! Lookup an Edge's associated ConnRef* by the Edge's ID.
    std::map<id_type, Avoid::ConnRef*> edgeIdToConnRef;
    //! Lookup a Node's associated ShapeRef* by the Node's ID.
    std::map<id_type, Avoid::ShapeRef*> nodeIdToShapeRef;
};


//! @brief  Does a special orthogonal routing in a graph having no leaves,
//!         ensuring that at least two distinct sides of every node are
//!         used as connection points. This is useful if we later wish to
//!         planarise the layout, since it ensures that no node will become
//!         a leaf in that process.
class LeaflessOrthoRouter {
public:
    //! @brief  Standard constructor.
    //!
    //! @param[in]  G  the Graph in which the routing is to be done. This Graph must
    //!                not have any leaves, i.e. Nodes of degree 1.
    //! @param[in]  opts  a HolaOptions object from which we can read settings
    //!                   for the router.
    LeaflessOrthoRouter(Graph_SP G, const HolaOpts &opts);

    //! @brief  Destructor.
    ~LeaflessOrthoRouter(void) {
        /*  The only newly allocated objects are the ConnRefs and ShapeRefs allocated
         *  in the constructor.
         *
         *  But the libavoid::Router takes responsibility for these, and frees them
         *  all in its own destructor.
         *
         *  Since we keep a Router instance, not a pointer, our Router is automatically
         *  destroyed when this LeaflessOrthoRouter is destroyed.
         *
         *  So there is nothing for us to do.
         */
    }

    //! @brief  Do the routing.
    void route(Logger *logger = nullptr);

    //! @brief  Set the Router's shapeBufferDistance parameter as a scalar multiple of the
    //!         ideal edge length read from the Graph.
    //! @warning  This is provided only as a convenience for testing. In the HOLA algorithm
    //!           for which this is designed, shapeBufferDistance should always be zero.
    //!           Positive values there may result in failures to route.
    //!           HOLA uses its own node padding instead, for this purpose.
    void setShapeBufferDistanceIELScalar(double a);

    //! For testing, it is useful to be able to see the result of each
    //! attempt to do the routing.
    //! If desired, set recordEachAttempt to true.
    //! Then the TGLF for each routing attmept will be available in
    //! routingAttemptTglf.
    bool recordEachAttempt = false;
    std::vector<std::string> routingAttemptTglf;

private:

    //! @brief  Determine the compass direction in which the routed connector
    //!         for Edge e departs from Node u.
    CardinalDir departureDir(const Edge_SP &e, const Node_SP &u) const;

    //! @brief  Say whether a set of connection directions denotes just a
    //!         sole direction.
    inline bool isSoleDirec(Avoid::ConnDirFlags dirs) const {
        return dirs == Avoid::ConnDirUp || dirs == Avoid::ConnDirDown || dirs == Avoid::ConnDirLeft || dirs == Avoid::ConnDirRight;
    }

    Graph_SP m_graph;
    size_t m_n; // number of nodes in the graph
    RoutingAdapter m_ra;
    double m_iel;

    //! Lookup allowed connection directions by IDs [eID][nID], where
    //! eID is an Edge ID, and nID is the ID of either the source or target
    //! Node of that Edge.
    SparseIdMatrix2d<Avoid::ConnDirFlags>::type m_allowedConnDirs;

};




} // namespace dialect

#endif // DIALECT_ROUTING_H
