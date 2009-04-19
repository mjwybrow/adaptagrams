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

//! @file    router.h
//! @brief   Contains the interface for the Router class.


#ifndef AVOID_ROUTER_H
#define AVOID_ROUTER_H

//#define LINEDEBUG

#include "libavoid/shape.h"
#include "libavoid/viscluster.h"
#include "libavoid/graph.h"
#include "libavoid/timer.h"
#include <list>
#include <utility>
#ifdef LINEDEBUG       
    #include <SDL.h>
#endif


namespace Avoid {

class ConnRef;
typedef std::list<ConnRef *> ConnRefList;
typedef std::list<unsigned int> IntList;
class MoveInfo;
typedef std::list<MoveInfo *> MoveInfoList;

//! @brief  Flags that can be passed to the router during initialisation 
//!         to specify options.
enum RouterFlag
{
        //! @brief  This option specifies that the router should maintain the
        //!         structures necessary to allow poly-line connector routing.
        PolyLineRouting = 1,
        //! @brief  This option specifies that the router should maintain the
        //!         structures necessary to allow orthogonal connector routing.
        OrthogonalRouting = 2
};


static const unsigned int runningTo = 1;
static const unsigned int runningFrom = 2;
static const unsigned int runningToAndFrom = runningTo | runningFrom;


//! @brief   The Router class represents a libavoid router instance.
//!
//! Usually you would keep a separate Router instance for each diagram
//! or layout you have open in your application.
//
class Router {
    public:
        //! @brief  Constructor for router instance.
        //!
        //! @param[in]  flags  One or more Avoid::RouterFlag options to 
        //!                    control the behaviour of the router.
        Router(const unsigned int flags);

        //! @brief  Destructor for router instance.
        //!
        //! @note   Destroying a router instance will delete all remaining
        //!         shapes and connectors, thereby invalidating any existing
        //!         pointers to them.
        ~Router();

        ShapeRefList shapeRefs;
        ConnRefList connRefs;
        ClusterRefList clusterRefs;
        EdgeList visGraph;
        EdgeList invisGraph;
        EdgeList visOrthogGraph;
        ContainsMap contains;
        VertInfList vertices;
        VertInfList orthogVertices;
        ContainsMap enclosingClusters;
        
        bool PartialTime;
        bool SimpleRouting;
        bool ClusteredRouting;
        double segmt_penalty;
        double angle_penalty;
        double crossing_penalty;
        double cluster_crossing_penalty;

        // Poly-line routing options:
        bool IgnoreRegions;
        bool IncludeEndpoints;
        bool UseLeesAlgorithm;
        bool InvisibilityGrph;
       
        // General routing options:
        bool SelectiveReroute;
        
        //! @brief Controls whether shape movement actions are performed 
        //!        immediately or are consolidated.
        //!
        //! If true (the default), then moveShape() operations are queued and
        //! performed together efficiently when processMoves() is called.  If
        //! set to false, then movement actions are performed immediately by 
        //! the moveShape() function.
        //
        bool ConsolidateMoves;
        bool PartialFeedback;
        bool RubberBandRouting;
        

        // Instrumentation:
        Timer timers;
        int st_checked_edges;
#ifdef LINEDEBUG
        SDL_Surface *avoid_screen;
#endif

        //! @brief Add a shape to the router scene.
        //!
        //! This shape will be considered to be an obstacle. Calling this 
        //! function will cause connectors intersecting the added shape to
        //! be marked as needing to be rerouted.
        //!
        //! @param[in]  shape  Pointer reference to the shape being added.
        //
        void addShape(ShapeRef *shape);

        //! @brief Remove a shape from the router scene.
        //!
        //! Connectors that could have a better (usually shorter) path after
        //! the removal of this shape will be marked as needing to be rerouted.
        //!
        //! @param[in]  shape  Pointer reference to the shape being removed.
        //
        void removeShape(ShapeRef *shape);

        //! @brief Move or resize an existing shape within the router scene.
        //!
        //! A new polygon for the shape can be given to effectively move or 
        //! resize the shape with the scene.  Connectors that intersect the 
        //! new shape polygon, or that could have a better (usually shorter)
        //! path after the change, will be marked as needing to be rerouted.
        //!
        //! @param[in]  shape       Pointer reference to the shape being 
        //!                         moved/resized.
        //! @param[in]  newPoly     The new polygon boundry for the shape.
        //! @param[in]  first_move  This option is used for some advanced 
        //!                         (currently undocumented) behaviour and 
        //!                         it should be ignored for the moment.
        //
        void moveShape(ShapeRef *shape, const Polygon& newPoly,
                const bool first_move = false);

        //! @brief Move an existing shape within the router scene by a relative
        //!        distance.
        //!         
        //! Connectors that intersect the shape's new position, or that could 
        //! have a better (usually shorter) path after the change, will be 
        //! marked as needing to be rerouted.
        //!
        //! @param[in]  shape       Pointer reference to the shape being moved.
        //! @param[in]  xDiff       The distance to move the shape in the 
        //!                         x dimension.
        //! @param[in]  yDiff       The distance to move the shape in the 
        //!                         y dimension.
        //
        void moveShape(ShapeRef *shape, const double xDiff, const double yDiff);

        //! @brief Process all queued shape move/resize actions efficiently.
        //!
        //! In the default mode of the Router (where ConsolidateMoves = true),
        //! this function will efficiently process all move actions registered
        //! by calls to moveShape() since the last call to processMoves().
        //!
        //! If ConsolidateMoves = false, then this function does nothing.
        //
        void processMoves(void);
        
        void addCluster(ClusterRef *cluster);
        void delCluster(ClusterRef *cluster);

        void attachedConns(IntList &conns, const unsigned int shapeId,
                const unsigned int type);
        void attachedShapes(IntList &shapes, const unsigned int shapeId,
                const unsigned int type);
        
        void markConnectors(ShapeRef *shape);
        void generateContains(VertInf *pt);
        void printInfo(void);
        unsigned int assignId(const unsigned int suggestedId);
        void regenerateStaticBuiltGraph(void);
        void setStaticGraphInvalidated(const bool invalidated);
        unsigned int defaultConnType(void) const;

    private:
        void newBlockingShape(Polygon *poly, int pid);
        void checkAllBlockedEdges(int pid);
        void checkAllMissingEdges(void);
        void adjustContainsWithAdd(const Polygon& poly, const int p_shape);
        void adjustContainsWithDel(const int p_shape);
        void adjustClustersWithAdd(const PolygonInterface& poly, 
                const int p_cluster);
        void adjustClustersWithDel(const int p_cluster);
        void callbackAllInvalidConnectors(void);
        bool idIsUnique(const unsigned int id) const;

        MoveInfoList moveList;
        unsigned int _largestAssignedId;

    public:
        // Overall modes:
        bool _polyLineRouting;
        bool _orthogonalRouting;

        bool _staticGraphInvalidated;
};

}



#endif
