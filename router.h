/*
 * vim: ts=4 sw=4 et tw=0 wm=0
 *
 * libavoid - Fast, Incremental, Object-avoiding Line Router
 *
 * Copyright (C) 2004-2007  Michael Wybrow <mjwybrow@users.sourceforge.net>
 * Copyright (C) 2008-2009  Monash University
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


static const unsigned int runningTo = 1;
static const unsigned int runningFrom = 2;
static const unsigned int runningToAndFrom = runningTo | runningFrom;


class Router {
    public:
        Router();

        ShapeRefList shapeRefs;
        ConnRefList connRefs;
        ClusterRefList clusterRefs;
        EdgeList visGraph;
        EdgeList invisGraph;
        ContainsMap contains;
        VertInfList vertices;
        ContainsMap enclosingClusters;
        
        bool PartialTime;
        bool SimpleRouting;
        bool ClusteredRouting;
        double segmt_penalty;
        double angle_penalty;
        double crossing_penalty;
        double cluster_crossing_penalty;


        bool UseAStarSearch;
        bool IgnoreRegions;
        bool SelectiveReroute;
        bool IncludeEndpoints;
        bool UseLeesAlgorithm;
        bool InvisibilityGrph;
        bool ConsolidateMoves;
        bool PartialFeedback;
        bool RubberBandRouting;

        // Instrumentation:
        Timer timers;
        int st_checked_edges;
#ifdef LINEDEBUG
        SDL_Surface *avoid_screen;
#endif

        void addShape(ShapeRef *shape);
        void delShape(ShapeRef *shape);
        void moveShape(ShapeRef *shape, const Polygon& newPoly,
                const bool first_move = false);
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

        MoveInfoList moveList;
};

}



#endif
