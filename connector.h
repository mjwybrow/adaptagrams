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

//! @file    shape.h
//! @brief   Contains the interface for the ConnRef class.


#ifndef AVOID_CONNECTOR_H
#define AVOID_CONNECTOR_H

#include <list>
#include <vector>

#include "libavoid/vertices.h"
#include "libavoid/geometry.h"
#include "libavoid/shape.h"


namespace Avoid {

class Router;
class ConnRef;
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


static const double ATTACH_POS_TOP = 0;
static const double ATTACH_POS_CENTRE = 0.5;
static const double ATTACH_POS_BOTTOM = 1;
static const double ATTACH_POS_LEFT = ATTACH_POS_TOP;
static const double ATTACH_POS_RIGHT = ATTACH_POS_BOTTOM;


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
        //! @param[in]  srcPoint  New source endpoint for the connector.
        //! @param[in]  dstPoint  New destination endpoint for the connector.
        void setEndpoints(const ConnEnd& srcPoint, const ConnEnd& dstPoint);
        
        //! @brief  Sets just a new source endpoint for this connector.
        //!
        //! @param[in]  srcPoint  New source endpoint for the connector.
        void setSourceEndpoint(const ConnEnd& srcPoint);
        
        //! @brief  Sets just a new destination endpoint for this connector.
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

       

        // @brief   Returns the source endpoint vertex in the visibility graph.
        // @returns The source endpoint vertex.
        VertInf *src(void);
        // @brief   Returns the destination endpoint vertex in the 
        //          visibility graph.
        // @returns The destination endpoint vertex.
        VertInf *dst(void);
        

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
    
    private:
        friend class Router;
        friend class ConnEnd;

        PolyLine& routeRef(void);
        void freeRoutes(void);
        void performCallback(void);
        bool generatePath(void);
        bool generatePath(Point p0, Point p1);
        void unInitialise(void);
        void updateEndPoint(const unsigned int type, const ConnEnd& connEnd);
        void common_updateEndPoint(const unsigned int type, ConnEnd connEnd);

        Router *_router;
        unsigned int _id;
        ConnType _type;
        unsigned int _srcId, _dstId;
        bool _orthogonal;
        bool _needs_reroute_flag;
        bool _false_path;
        bool _needs_repaint;
        bool _active;
        PolyLine _route;
        Polygon _display_route;
        double _route_dist;
        ConnRefList::iterator _pos;
        VertInf *_srcVert;
        VertInf *_dstVert;
        VertInf *_startVert;
        bool _initialised;
        void (*_callback)(void *);
        void *_connector;
        bool _hateCrossings;
        ConnEnd *_srcConnEnd;
        ConnEnd *_dstConnEnd;
};


class PointRep;
typedef std::set<PointRep *> PointRepSet;
typedef std::list<PointRep *> PointRepList;

class PointRep
{
    public:
        PointRep(Point *p, const ConnRef *c)
            : point(p),
              conn(c)

        {
        }
        bool follow_inner(PointRep *target);

        Point *point;
        const ConnRef *conn;
        // inner_set: Set of pointers to the PointReps 'inner' of 
        // this one, at this corner.
        PointRepSet inner_set;
};


typedef std::pair<Point *, ConnRef *> PtConnPtrPair;

class PtOrder
{
    public:
        PtOrder()
        {
        }
        ~PtOrder();
        bool addPoints(const int dim, PtConnPtrPair innerArg, 
                PtConnPtrPair outerArg, bool swapped);
        void sort(const int dim);
        int positionFor(const ConnRef *conn, const size_t dim) const;

        // One for each dimension.
        PointRepList connList[2];
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


