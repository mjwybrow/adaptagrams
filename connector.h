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

//! @file    connector.h
//! @brief   Contains the interface for the ConnRef class.


#ifndef AVOID_CONNECTOR_H
#define AVOID_CONNECTOR_H

#include <list>
#include <vector>

#include "libavoid/vertices.h"
#include "libavoid/geometry.h"
#include "libavoid/connend.h"


namespace Avoid {

class Router;
class ConnRef;
class JunctionRef;
class ShapeRef;
typedef std::list<ConnRef *> ConnRefList;


//! @brief  Describes the type of routing that is performed for each 
//!         connector.
enum ConnType {
    ConnType_None       = 0,
    //! @brief  The connector path will be a shortest-path poly-line that
    //!         routes around obstacles.
    ConnType_PolyLine   = 1,
    //! @brief  The connector path will be a shortest-path orthogonal 
    //!         poly-line (only vertical and horizontal line segments) that
    //!         routes around obstacles.
    ConnType_Orthogonal = 2
};


//! @brief   The ConnRef class represents a connector object.
//!
//! Connectors are a (possible multi-segment) line between two points.
//! They are routed intelligently so as not to overlap any of the shape
//! objects in the Router scene.
//! 
//! Routing penalties can be applied, resulting in more aesthetically pleasing
//! connector paths with fewer segments or less severe bend-points.
//!
//! You can set a function to be called when the connector has been rerouted
//! and needs to be redrawn.  Alternatively, you can query the connector's
//! needsRepaint() function to determine this manually.
//!
//! Usually, it is expected that you would create a ConnRef for each connector 
//! in your diagram and keep that reference in your own connector class.
//!
class ConnRef
{
    public:
        //! @brief Constructs a connector with no endpoints specified.
        //!
        //! @param[in]  router  The router scene to place the connector into.
        //! @param[in]  id      A unique positive integer ID for the connector.  
        //!
        //! If an ID is not specified, then one will be assigned to the shape.
        //! If assigning an ID yourself, note that it should be a unique 
        //! positive integer.  Also, IDs are given to all objects in a scene,
        //! so the same ID cannot be given to a shape and a connector for 
        //! example.
        //!
        ConnRef(Router *router, const unsigned int id = 0);
        //! @brief Constructs a connector with endpoints specified.
        //!
        //! @param[in]  router  The router scene to place the connector into.
        //! @param[in]  id      A unique positive integer ID for the connector.
        //! @param[in]  src     The source endpoint of the connector.
        //! @param[in]  dst     The destination endpoint of the connector.
        //!
        //! If an ID is not specified, then one will be assigned to the shape.
        //! If assigning an ID yourself, note that it should be a unique 
        //! positive integer.  Also, IDs are given to all objects in a scene,
        //! so the same ID cannot be given to a shape and a connector for 
        //! example.
        //!
        ConnRef(Router *router, const ConnEnd& src, const ConnEnd& dst,
                const unsigned int id = 0);
        //! @brief  Destuctor.
        ~ConnRef();
        
        //! @brief  Sets both a new source and destination endpoint for this 
        //!         connector.
        //!
        //! If the router is using transactions, then this action will occur
        //! the next time Router::processTransaction() is called.  See
        //! Router::setTransactionUse() for more information.
        //!
        //! @param[in]  srcPoint  New source endpoint for the connector.
        //! @param[in]  dstPoint  New destination endpoint for the connector.
        void setEndpoints(const ConnEnd& srcPoint, const ConnEnd& dstPoint);
        
        //! @brief  Sets just a new source endpoint for this connector.
        //!
        //! If the router is using transactions, then this action will occur
        //! the next time Router::processTransaction() is called.  See
        //! Router::setTransactionUse() for more information.
        //!
        //! @param[in]  srcPoint  New source endpoint for the connector.
        void setSourceEndpoint(const ConnEnd& srcPoint);
        
        //! @brief  Sets just a new destination endpoint for this connector.
        //!
        //! If the router is using transactions, then this action will occur
        //! the next time Router::processTransaction() is called.  See
        //! Router::setTransactionUse() for more information.
        //!
        //! @param[in]  dstPoint  New destination endpoint for the connector.
        void setDestEndpoint(const ConnEnd& dstPoint);
        
        //! @brief   Returns the ID of this connector.
        //! @returns The ID of the connector. 
        unsigned int id(void) const;
        
        //! @brief   Returns a pointer to the router scene this connector is in.
        //! @returns A pointer to the router scene for this connector.
        Router *router(void) const;

        //! @brief   Returns an indication of whether this connector has a 
        //!          new route and thus needs to be repainted.
        //!
        //! If the connector has been rerouted and need repainting, the  
        //! displayRoute() method can be called to get a reference to the 
        //! new route.
        //!
        //! @returns Returns true if the connector requires repainting, or 
        //!          false if it does not.
        bool needsRepaint(void) const;
        
        //! @brief   Returns a reference to the current route for the connector.
        //!
        //! This is a "raw" version of the route, where each line segment in
        //! the route may be made up of multiple collinear line segments.  It
        //! will also not have post-processing (like curved corners) applied
        //! to it.  The simplified route for display can be obtained by calling
        //! displayRoute().
        //!
        //! @returns The PolyLine route for the connector.
        //! @note    You can obtain a modified version of this poly-line 
        //!          route with curved corners added by calling 
        //!          PolyLine::curvedPolyline().
        const PolyLine& route(void) const;
        
        //! @brief   Returns a reference to the current display version of the
        //!          route for the connector.
        //! 
        //! The display version of a route has been simplified to collapse all
        //! collinear line segments into single segments.  It may also have 
        //! post-processing applied to the route, such as curved corners or
        //! nudging.
        //! 
        //! @returns The PolyLine display route for the connector.
        PolyLine& displayRoute(void);
        
        //! @brief   Sets a callback function that will called to indicate that
        //!          the connector needs rerouting.
        //!
        //! The cb function will be called when shapes are added to, removed 
        //! from or moved about on the page.  The pointer ptr will be passed 
        //! as an argument to the callback function.
        //!
        //! @param[in]  cb   A pointer to the callback function.
        //! @param[in]  ptr  A generic pointer that will be passed to the 
        //!                  callback function.
        void setCallback(void (*cb)(void *), void *ptr);
        
        //! @brief   Returns the type of routing performed for this connector.
        //! @return  The type of routing performed.
        //!
        ConnType routingType(void) const;
        
        //! @brief       Sets the type of routing to be performed for this 
        //!              connector.
        //! 
        //! If a call to this method changes the current type of routing 
        //! being used for the connector, then it will get rerouted during
        //! the next processTransaction() call, or immediately if 
        //! transactions are not being used.
        //!
        //! @param type  The type of routing to be performed.
        //!
        void setRoutingType(ConnType type);
       
        //! @brief   Splits a connector in the centre of the segmentNth 
        //!          segment and creates a junction point there as well 
        //!          as a second connector.
        //!
        //! The new junction and connector will be automatically added to 
        //! the router scene.  A slight preference will be given to the 
        //! connectors connecting to the junction in the same orientation
        //! the line segment already existed in.
        //!
        //! @return  A pair containing pointers to the new JunctioRef and 
        //!          ConnRef.
        std::pair<JunctionRef *, ConnRef *> splitAtSegment(
                const size_t segmentN);

        // @brief   Returns the source endpoint vertex in the visibility graph.
        // @returns The source endpoint vertex.
        VertInf *src(void);
        // @brief   Returns the destination endpoint vertex in the 
        //          visibility graph.
        // @returns The destination endpoint vertex.
        VertInf *dst(void);
        
        //! @brief  Allows the user to specify a set of checkpoints that this
        //!         connector will route via.
        //!
        //! When routing, the connector will attempt to visit each of the 
        //! points in the checkpoints list in order.  It will route from the
        //! source point to the first checkpoint, to the second checkpoint, 
        //! etc.  If a checkpoint is unreachable because it lies inside an
        //! obstacle, then that checkpoint will be skipped.
        //! 
        //! @param[in] checkpoints  An ordered list of points that the 
        //!                         connector will attempt to route via.
        void setRoutingCheckpoints(const std::vector<Point>& checkpoints);

        //! @brief   Returns the current set of routing checkpoints for this
        //!          connector.
        //! @returns The ordered list of Points that this connecotr will 
        //!          route via.
        std::vector<Point> routingCheckpoints(void) const;

        void set_route(const PolyLine& route);
        void calcRouteDist(void);
        void makeActive(void);
        void makeInactive(void);
        VertInf *start(void);
        void removeFromGraph(void);
        bool isInitialised(void);
        void makePathInvalid(void);
        void setHateCrossings(bool value);
        bool doesHateCrossings(void);
        void setEndpoint(const unsigned int type, const ConnEnd& connEnd);
        bool setEndpoint(const unsigned int type, const VertID& pointID, 
                Point *pointSuggestion = NULL);
        std::vector<Point> possibleDstPinPoints(void) const;
    
    private:
        friend class Router;
        friend class ConnEnd;
        friend class JunctionRef;

        PolyLine& routeRef(void);
        void freeRoutes(void);
        void performCallback(void);
        bool generatePath(void);
        void generateCheckpointsPath(std::vector<Point>& path,
                std::vector<VertInf *>& vertices);
        void generateStandardPath(std::vector<Point>& path,
                std::vector<VertInf *>& vertices);
        void unInitialise(void);
        void updateEndPoint(const unsigned int type, const ConnEnd& connEnd);
        void common_updateEndPoint(const unsigned int type, ConnEnd connEnd);
        void freeActivePins(void);

        Router *m_router;
        unsigned int m_id;
        ConnType m_type;
        bool m_orthogonal;
        bool m_needs_reroute_flag;
        bool m_false_path;
        bool m_needs_repaint;
        bool m_active;
        PolyLine m_route;
        Polygon m_display_route;
        double m_route_dist;
        ConnRefList::iterator m_connrefs_pos;
        VertInf *m_src_vert;
        VertInf *m_dst_vert;
        VertInf *m_start_vert;
        bool m_initialised;
        void (*m_callback_func)(void *);
        void *m_connector;
        bool m_hate_crossings;
        ConnEnd *m_src_connend;
        ConnEnd *m_dst_connend;
        std::vector<Point> m_checkpoints;
        std::vector<VertInf *> m_checkpoint_vertices;
};


typedef std::pair<Point *, ConnRef *> PtConnPtrPair;

typedef std::vector< PtConnPtrPair > PointRepVector;
typedef std::list<std::pair<size_t, size_t> > NodeIndexPairLinkList;

class PtOrder
{
    public:
        PtOrder();
        ~PtOrder();
        void addPoints(const size_t dim, const PtConnPtrPair& arg1, 
                const PtConnPtrPair& arg2);
        void addOrderedPoints(const size_t dim, const PtConnPtrPair& innerArg, 
                const PtConnPtrPair& outerArg, bool swapped);
        int positionFor(const size_t dim, const ConnRef *conn);

    private:
        size_t insertPoint(const size_t dim, const PtConnPtrPair& point);
        void sort(const size_t dim);

        // One for each dimension.
        bool sorted[2];
        PointRepVector nodes[2];
        NodeIndexPairLinkList links[2];
        PointRepVector sortedConnVector[2];
};

typedef std::map<Avoid::Point,PtOrder> PtOrderMap;
typedef std::set<Avoid::Point> PointSet;


const unsigned int CROSSING_NONE = 0;
const unsigned int CROSSING_TOUCHES = 1;
const unsigned int CROSSING_SHARES_PATH = 2;
const unsigned int CROSSING_SHARES_PATH_AT_END = 4;
const unsigned int CROSSING_SHARES_FIXED_SEGMENT = 8;


typedef std::pair<int, unsigned int> CrossingsInfoPair;
typedef std::vector<Avoid::Point> PointList;
typedef std::vector<PointList> SharedPathList;

class ConnectorCrossings
{
    public:
        ConnectorCrossings(Avoid::Polygon& poly, bool polyIsConn, 
                Avoid::Polygon& conn, ConnRef *polyConnRef = NULL, 
                ConnRef *connConnRef = NULL);
        void clear(void);
        void countForSegment(size_t cIndex, const bool finalSegment);

        Avoid::Polygon& poly;
        bool polyIsConn; 
        Avoid::Polygon& conn; 
        bool checkForBranchingSegments;
        ConnRef *polyConnRef;
        ConnRef *connConnRef;
        
        unsigned int crossingCount;
        unsigned int crossingFlags;
        PointSet *crossingPoints;
        PtOrderMap *pointOrders;
        SharedPathList *sharedPaths;
};

extern void splitBranchingSegments(Avoid::Polygon& poly, bool polyIsConn,
        Avoid::Polygon& conn, const double tolerance = 0);
extern bool validateBendPoint(VertInf *aInf, VertInf *bInf, VertInf *cInf);

}


#endif


