/*
 * vim: ts=4 sw=4 et tw=0 wm=0
 *
 * libavoid - Fast, Incremental, Object-avoiding Line Router
 *
 * Copyright (C) 2004-2013  Monash University
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
 * Author(s):  Michael Wybrow
*/


#include <algorithm>
#include <cmath>
#include <cfloat>

#include "libavoid/shape.h"
#include "libavoid/router.h"
#include "libavoid/visibility.h"
#include "libavoid/connector.h"
#include "libavoid/junction.h"
#include "libavoid/viscluster.h"
#include "libavoid/connend.h"
#include "libavoid/debug.h"
#include "libavoid/orthogonal.h"
#include "libavoid/assertions.h"
#include "libavoid/connectionpin.h"
#include "libavoid/makepath.h"

namespace Avoid {


Router::Router(const unsigned int flags)
    : visOrthogGraph(),
      PartialTime(false),
      SimpleRouting(false),
      ClusteredRouting(true),
      // Poly-line algorithm options:
      IgnoreRegions(true),
      UseLeesAlgorithm(true),
      InvisibilityGrph(true),
      // General algorithm options:
      SelectiveReroute(true),
      PartialFeedback(false),
      RubberBandRouting(false),
      // Instrumentation:
      st_checked_edges(0),
      m_largest_assigned_id(0),
      m_consolidate_actions(true),
      m_currently_calling_destructors(false),
      m_topology_addon(new TopologyAddonInterface()),
      // Mode options:
      m_allows_polyline_routing(false),
      m_allows_orthogonal_routing(false),
      m_static_orthogonal_graph_invalidated(true),
      m_in_crossing_rerouting_stage(false)
{
    // At least one of the Routing modes must be set.
    COLA_ASSERT(flags & (PolyLineRouting | OrthogonalRouting));

    if (flags & PolyLineRouting)
    {
        m_allows_polyline_routing = true;
    }
    if (flags & OrthogonalRouting)
    {
        m_allows_orthogonal_routing = true;
    }

    for (size_t p = 0; p < lastRoutingParameterMarker; ++p)
    {
        m_routing_parameters[p] = 0.0;
    }
    m_routing_parameters[segmentPenalty] = 10;
    m_routing_parameters[clusterCrossingPenalty] = 4000;
    m_routing_parameters[portDirectionPenalty] = 100;
    m_routing_parameters[idealNudgingDistance] = 4.0;

    m_routing_options[nudgeOrthogonalSegmentsConnectedToShapes] = false;
    m_routing_options[improveHyperedgeRoutesMovingJunctions] = true;
    m_routing_options[penaliseOrthogonalSharedPathsAtConnEnds] = false;
    m_routing_options[nudgeOrthogonalTouchingColinearSegments] = false;

    m_hyperedge_rerouter.setRouter(this);
}


Router::~Router()
{
    m_currently_calling_destructors = true;

    // Delete remaining connectors.
    ConnRefList::iterator conn = connRefs.begin();
    while (conn != connRefs.end())
    {
        db_printf("Deleting connector %u in ~Router()\n", (*conn)->id());
        delete *conn;
        conn = connRefs.begin();
    }

    // Remove remaining obstacles (shapes and junctions).
    ObstacleList::iterator obstacle =  m_obstacles.begin();
    while (obstacle != m_obstacles.end())
    {
        Obstacle *obstaclePtr = *obstacle;
        ShapeRef *shape = dynamic_cast<ShapeRef *> (obstaclePtr);
        db_printf("Deleting %s %u in ~Router()\n", 
                (shape) ? "shape" : "junction", obstaclePtr->id());
        if (obstaclePtr->isActive())
        {
            obstaclePtr->removeFromGraph();
            obstaclePtr->makeInactive();
        }
        delete obstaclePtr;
        obstacle = m_obstacles.begin();
    }
    m_currently_calling_destructors = false;

    // Cleanup orphaned orthogonal graph vertices.
    destroyOrthogonalVisGraph();

    COLA_ASSERT(m_obstacles.size() == 0);
    COLA_ASSERT(connRefs.size() == 0);
    COLA_ASSERT(visGraph.size() == 0);

    delete m_topology_addon;
}

ShapeRef *Router::shapeContainingPoint(const Point& point)
{
    // Count points on the border as being inside.
    bool countBorder = true;

    // Compute enclosing shapes.
    ObstacleList::const_iterator finish = m_obstacles.end();
    for (ObstacleList::const_iterator i = m_obstacles.begin(); i != finish; ++i)
    {
        ShapeRef *shape = dynamic_cast<ShapeRef *>(*i);
        if (shape && inPoly(shape->routingPolygon(), point, countBorder))
        {
            return shape;
        }
    }
    return NULL;
}

void Router::modifyConnector(ConnRef *conn, const unsigned int type,
        const ConnEnd& connEnd, bool connPinMoveUpdate)
{
    ActionInfo modInfo(ConnChange, conn);
    
    ActionInfoList::iterator found = 
            find(actionList.begin(), actionList.end(), modInfo);
    if (found == actionList.end())
    {
        // Matching action not found, so add.
        modInfo.conns.push_back(std::make_pair(type, connEnd));
        actionList.push_back(modInfo);
    }
    else
    {
        // Update the found action as necessary.
        found->addConnEndUpdate(type, connEnd, connPinMoveUpdate);
    }

    if (!m_consolidate_actions)
    {
        processTransaction();
    }
}


void Router::modifyConnector(ConnRef *conn)
{
    ActionInfo modInfo(ConnChange, conn);
    
    ActionInfoList::iterator found = 
            find(actionList.begin(), actionList.end(), modInfo);
    if (found == actionList.end())
    {
        actionList.push_back(modInfo);
    }

    if (!m_consolidate_actions)
    {
        processTransaction();
    }
}


void Router::modifyConnectionPin(ShapeConnectionPin *pin)
{
    ActionInfo modInfo(ConnectionPinChange, pin);
    
    ActionInfoList::iterator found = 
            find(actionList.begin(), actionList.end(), modInfo);
    if (found == actionList.end())
    {
        actionList.push_back(modInfo);
    }

    if (!m_consolidate_actions)
    {
        processTransaction();
    }
}


void Router::removeObjectFromQueuedActions(const void *object)
{
    for (ActionInfoList::iterator curr = actionList.begin();
            curr != actionList.end(); )
    {
        if (curr->objPtr == object)
        {
            curr = actionList.erase(curr);
        }
        else
        {
            ++curr;
        }
    }
}


void Router::addShape(ShapeRef *shape)
{
    // There shouldn't be remove events or move events for the same shape
    // already in the action list.
    // XXX: Possibly we could handle this by ordering them intelligently.
    COLA_ASSERT(find(actionList.begin(), actionList.end(), 
                ActionInfo(ShapeRemove, shape)) == actionList.end());
    COLA_ASSERT(find(actionList.begin(), actionList.end(), 
                ActionInfo(ShapeMove, shape)) == actionList.end());

    ActionInfo addInfo(ShapeAdd, shape);
    
    ActionInfoList::iterator found = 
            find(actionList.begin(), actionList.end(), addInfo);
    if (found == actionList.end())
    {
        actionList.push_back(addInfo);
    }

    if (!m_consolidate_actions)
    {
        processTransaction();
    }
}


void Router::deleteShape(ShapeRef *shape)
{
    // There shouldn't be add events events for the same shape already 
    // in the action list.
    // XXX: Possibly we could handle this by ordering them intelligently.
    COLA_ASSERT(find(actionList.begin(), actionList.end(), 
                ActionInfo(ShapeAdd, shape)) == actionList.end());

    // Delete any ShapeMove entries for this shape in the action list.
    ActionInfoList::iterator found = find(actionList.begin(), 
            actionList.end(), ActionInfo(ShapeMove, shape));
    if (found != actionList.end())
    {
        actionList.erase(found);
    }

    // Add the ShapeRemove entry.
    ActionInfo remInfo(ShapeRemove, shape);
    found = find(actionList.begin(), actionList.end(), remInfo);
    if (found == actionList.end())
    {
        actionList.push_back(remInfo);
    }

    if (!m_consolidate_actions)
    {
        processTransaction();
    }
}


void Router::deleteConnector(ConnRef *connector)
{
    m_currently_calling_destructors = true;
    delete connector;
    m_currently_calling_destructors = false;
}

void Router::moveShape(ShapeRef *shape, const double xDiff, const double yDiff)
{
    ActionInfo moveInfo(ShapeMove, shape, Polygon(), false);
    ActionInfoList::iterator found =
            find(actionList.begin(), actionList.end(), moveInfo);

    Polygon newPoly;
    if (found != actionList.end())
    {
        // The shape already has a queued move, so use that shape position.
        newPoly = found->newPoly;
    }
    else
    {
        // Just use the existing position.
        newPoly = shape->polygon();
    }
    newPoly.translate(xDiff, yDiff);

    moveShape(shape, newPoly);
}


void Router::markAllObstaclesAsMoved(void)
{
    for (ObstacleList::iterator obstacleIt = m_obstacles.begin();
            obstacleIt != m_obstacles.end(); ++obstacleIt)
    {
        ShapeRef *shape = dynamic_cast<ShapeRef *> (*obstacleIt);
        JunctionRef *junction = dynamic_cast<JunctionRef *> (*obstacleIt);
        if (shape)
        {
            moveShape(shape, 0, 0);
        }
        else if (junction)
        {
            moveJunction(junction, 0, 0);
        }
    }
}

void Router::moveShape(ShapeRef *shape, const Polygon& newPoly, 
        const bool first_move)
{
    // There shouldn't be remove events or add events for the same shape
    // already in the action list.
    // XXX: Possibly we could handle this by ordering them intelligently.
    COLA_ASSERT(find(actionList.begin(), actionList.end(), 
                ActionInfo(ShapeRemove, shape)) == actionList.end());
    
    ActionInfoList::iterator found = find(actionList.begin(), 
            actionList.end(), ActionInfo(ShapeAdd, shape));
    if (found != actionList.end())
    {
        // The Add is enough, no need for the Move action too.
        // The shape will be added with it's existing polygon,
        // so set this to be the newPoly passed for the move.
        found->shape()->setNewPoly(newPoly);
        return;
    }

    ActionInfo moveInfo(ShapeMove, shape, newPoly, first_move);
    // Sanely cope with the case where the user requests moving the same
    // shape multiple times before rerouting connectors.
    found = find(actionList.begin(), actionList.end(), moveInfo);

    if (found != actionList.end())
    {
        // Just update the ActionInfo with the second polygon, but
        // leave the firstMove setting alone.
        found->newPoly = newPoly;
    }
    else 
    {
        actionList.push_back(moveInfo);
    }

    if (!m_consolidate_actions)
    {
        processTransaction();
    }
}


void Router::setStaticGraphInvalidated(const bool invalidated)
{
    m_static_orthogonal_graph_invalidated = invalidated;
}


void Router::destroyOrthogonalVisGraph(void)
{
    // Remove orthogonal visibility graph edges.
    visOrthogGraph.clear();

    // Remove the now orphaned vertices.
    VertInf *curr = vertices.shapesBegin();
    while (curr)
    {
        if (curr->orphaned() && (curr->id == dummyOrthogID))
        {
            VertInf *following = vertices.removeVertex(curr);
            delete curr;
            curr = following;
            continue;
        }
        curr = curr->lstNext;
    }
}


void Router::regenerateStaticBuiltGraph(void)
{
    // Here we do talks involved in updating the static-built visibility 
    // graph (if necessary) before we do any routing.
    if (m_static_orthogonal_graph_invalidated)
    {
        if (m_allows_orthogonal_routing)
        {
            destroyOrthogonalVisGraph();

            timers.Register(tmOrthogGraph, timerStart);
            // Regenerate a new visibility graph.
            generateStaticOrthogonalVisGraph(this);
            
            timers.Stop();
        }
        m_static_orthogonal_graph_invalidated = false;
    }
}


bool Router::transactionUse(void) const
{
    return m_consolidate_actions;
}


void Router::setTransactionUse(const bool transactions)
{
    m_consolidate_actions = transactions;
}


// Processes the action list.
void Router::processActions(void)
{
    bool notPartialTime = !(PartialFeedback && PartialTime);
    bool seenShapeMovesOrDeletes = false;

    m_transaction_start_time = clock();
    m_abort_transaction = false;

    actionList.sort();
    ActionInfoList::iterator curr;
    ActionInfoList::iterator finish = actionList.end();
    for (curr = actionList.begin(); curr != finish; ++curr)
    {
        ActionInfo& actInf = *curr;
        if (!((actInf.type == ShapeRemove) || (actInf.type == ShapeMove) ||
              (actInf.type == JunctionRemove) || (actInf.type == JunctionMove)))
        {
            // Not a move or remove action, so don't do anything.
            continue;
        }
        seenShapeMovesOrDeletes = true;

        Obstacle *obstacle = actInf.obstacle();
        ShapeRef *shape = actInf.shape();
        JunctionRef *junction = actInf.junction();
        bool isMove = (actInf.type == ShapeMove) ||
                (actInf.type == JunctionMove);;
        bool first_move = actInf.firstMove;

        unsigned int pid = obstacle->id();

        // o  Remove entries related to this shape's vertices
        obstacle->removeFromGraph();

        if (SelectiveReroute && (!isMove || notPartialTime || first_move))
        {
            markPolylineConnectorsNeedingReroutingForDeletedObstacle(obstacle);
        }

        adjustContainsWithDel(pid);

        if (isMove)
        {
            if (shape)
            {
                shape->moveAttachedConns(actInf.newPoly);
            }
            else if (junction)
            {
                junction->moveAttachedConns(actInf.newPosition);
            }
        }

        // Ignore this shape for visibility.
        // XXX: We don't really need to do this if we're not using Partial
        //      Feedback.  Without this the blocked edges still route
        //      around the shape until it leaves the connector.
        obstacle->makeInactive();

        if (!isMove)
        {
            // Free deleted obstacle.
            m_currently_calling_destructors = true;
            delete obstacle;
            m_currently_calling_destructors = false;
        }
    }

    if (seenShapeMovesOrDeletes && m_allows_polyline_routing)
    {
        if (InvisibilityGrph)
        {
            for (curr = actionList.begin(); curr != finish; ++curr)
            {
                ActionInfo& actInf = *curr;
                if (!((actInf.type == ShapeRemove) ||
                      (actInf.type == ShapeMove) ||
                      (actInf.type == JunctionRemove) ||
                      (actInf.type == JunctionMove)))
                {
                    // Not a move or remove action, so don't do anything.
                    continue;
                }

                // o  Check all edges that were blocked by this shape.
                checkAllBlockedEdges(actInf.obstacle()->id());
            }
        }
        else
        {
            // check all edges not in graph
            checkAllMissingEdges();
        }
    }

    for (curr = actionList.begin(); curr != finish; ++curr)
    {
        ActionInfo& actInf = *curr;
        if (!((actInf.type == ShapeAdd) || (actInf.type == ShapeMove) ||
              (actInf.type == JunctionAdd) || (actInf.type == JunctionMove)))
        {
            // Not a move or add action, so don't do anything.
            continue;
        }

        Obstacle *obstacle = actInf.obstacle();
        ShapeRef *shape = actInf.shape();
        JunctionRef *junction = actInf.junction();
        Polygon& newPoly = actInf.newPoly;
        bool isMove = (actInf.type == ShapeMove) ||
                (actInf.type == JunctionMove);

        unsigned int pid = obstacle->id();

        // Restore this shape for visibility.
        obstacle->makeActive();

        if (isMove)
        {
            if (shape)
            {
                shape->setNewPoly(newPoly);
            }
            else
            {
                junction->setPosition(actInf.newPosition);
            }
        }
        const Polygon& shapePoly = obstacle->routingPolygon();

        adjustContainsWithAdd(shapePoly, pid);

        if (m_allows_polyline_routing)
        {
            // o  Check all visibility edges to see if this one shape
            //    blocks them.
            if (!isMove || notPartialTime)
            {
                newBlockingShape(shapePoly, pid);
            }

            // o  Calculate visibility for the new vertices.
            if (UseLeesAlgorithm)
            {
                obstacle->computeVisibilitySweep();
            }
            else
            {
                obstacle->computeVisibilityNaive();
            }
            obstacle->updatePinPolyLineVisibility();
        }
    }

    // Update connector endpoints.
    for (curr = actionList.begin(); curr != finish; ++curr)
    {
        ActionInfo& actInf = *curr;
        if (actInf.type != ConnChange)
        {
            continue;
        }
        for (ConnUpdateList::iterator conn = actInf.conns.begin();
                conn != actInf.conns.end(); ++conn)
        {
            actInf.conn()->updateEndPoint(conn->first, conn->second);
        }
    }
    // Clear the actionList.
    actionList.clear();
}

bool Router::processTransaction(void)
{
    // If SimpleRouting, then don't update here.
    if ((actionList.empty() && (m_hyperedge_rerouter.count() == 0)) ||
            SimpleRouting)
    {
        return false;
    }

    processActions();

    m_static_orthogonal_graph_invalidated = true;
    rerouteAndCallbackConnectors();

    return true;
}


void Router::addJunction(JunctionRef *junction)
{
    // There shouldn't be remove events or move events for the same junction
    // already in the action list.
    // XXX: Possibly we could handle this by ordering them intelligently.
    COLA_ASSERT(find(actionList.begin(), actionList.end(), 
                ActionInfo(JunctionRemove, junction)) == actionList.end());
    COLA_ASSERT(find(actionList.begin(), actionList.end(), 
                ActionInfo(JunctionMove, junction)) == actionList.end());

    ActionInfo addInfo(JunctionAdd, junction);
    
    ActionInfoList::iterator found = 
            find(actionList.begin(), actionList.end(), addInfo);
    if (found == actionList.end())
    {
        actionList.push_back(addInfo);
    }

    if (!m_consolidate_actions)
    {
        processTransaction();
    }
}


void Router::deleteJunction(JunctionRef *junction)
{
    // There shouldn't be add events events for the same junction already 
    // in the action list.
    // XXX: Possibly we could handle this by ordering them intelligently.
    COLA_ASSERT(find(actionList.begin(), actionList.end(), 
                ActionInfo(JunctionAdd, junction)) == actionList.end());

    // Delete any ShapeMove entries for this shape in the action list.
    ActionInfoList::iterator found = find(actionList.begin(), 
            actionList.end(), ActionInfo(JunctionMove, junction));
    if (found != actionList.end())
    {
        actionList.erase(found);
    }

    // Add the ShapeRemove entry.
    ActionInfo remInfo(JunctionRemove, junction);
    found = find(actionList.begin(), actionList.end(), remInfo);
    if (found == actionList.end())
    {
        actionList.push_back(remInfo);
    }

    if (!m_consolidate_actions)
    {
        processTransaction();
    }
}


void Router::moveJunction(JunctionRef *junction, const double xDiff, 
        const double yDiff)
{
    ActionInfo moveInfo(JunctionMove, junction, Point());
    ActionInfoList::iterator found =
            find(actionList.begin(), actionList.end(), moveInfo);

    Point newPosition;
    if (found != actionList.end())
    {
        // The junction already has a queued move, so use that position.
        newPosition = found->newPosition;
    }
    else
    {
        // Just use the existing position.
        newPosition = junction->position();
    }
    newPosition.x += xDiff;
    newPosition.y += yDiff;

    moveJunction(junction, newPosition);
}


void Router::moveJunction(JunctionRef *junction, const Point& newPosition)
{
    // There shouldn't be remove events or add events for the same junction
    // already in the action list.
    // XXX: Possibly we could handle this by ordering them intelligently.
    COLA_ASSERT(find(actionList.begin(), actionList.end(), 
                ActionInfo(JunctionRemove, junction)) == actionList.end());
    
    ActionInfoList::iterator found = find(actionList.begin(), 
            actionList.end(), ActionInfo(JunctionAdd, junction));
    if (found != actionList.end())
    {
        // The Add is enough, no need for the Move action too.
        // The junction will be added with the new position.
        found->junction()->setPosition(newPosition);
        return;
    }

    ActionInfo moveInfo(JunctionMove, junction, newPosition);
    // Sanely cope with the case where the user requests moving the same
    // shape multiple times before rerouting connectors.
    found = find(actionList.begin(), actionList.end(), moveInfo);

    if (found != actionList.end())
    {
        // Just update the ActionInfo with the second position.
        found->newPosition = newPosition;
    }
    else 
    {
        actionList.push_back(moveInfo);
    }

    if (!m_consolidate_actions)
    {
        processTransaction();
    }
}

void Router::addCluster(ClusterRef *cluster)
{
    cluster->makeActive();
    
    unsigned int pid = cluster->id();
    ReferencingPolygon& poly = cluster->polygon();

    adjustClustersWithAdd(poly, pid);
}


void Router::deleteCluster(ClusterRef *cluster)
{
    cluster->makeInactive();
    
    unsigned int pid = cluster->id();
    
    adjustClustersWithDel(pid);
}


unsigned int Router::newObjectId(void) const
{
    return m_largest_assigned_id + 1;
}


unsigned int Router::assignId(const unsigned int suggestedId)
{
    // If the suggestedId is zero, then we assign the object the next
    // smallest unassigned ID, otherwise we trust the ID given is unique.
    unsigned int assignedId = (suggestedId == 0) ?  newObjectId() : suggestedId;
    
    // If assertions are enabled, then we check that this ID really is unique.
    COLA_ASSERT(objectIdIsUnused(assignedId));
    
    // Have the router record if this ID is larger than the largest assigned ID.
    m_largest_assigned_id = std::max(m_largest_assigned_id, assignedId);

    return assignedId;
}


    // Returns whether the given ID is unique among all objects known by the
    // router.  It is expected this is only going to be called from assertions
    // while debugging, so efficiency is not an issue and we just iterate over
    // all objects.
bool Router::objectIdIsUnused(const unsigned int id) const 
{
    // Examine shapes/junctions.
    for (ObstacleList::const_iterator i = m_obstacles.begin(); 
            i != m_obstacles.end(); ++i) 
    {
        if ((*i)->id() == id)
        {
            return false;
        }
    }

    // Examine connectors.
    for (ConnRefList::const_iterator i = connRefs.begin(); 
            i != connRefs.end(); ++i) 
    {
        if ((*i)->id() == id)
        {
            return false;
        }
    }

    // Examine clusters.
    for (ClusterRefList::const_iterator i = clusterRefs.begin(); 
            i != clusterRefs.end(); ++i) 
    {
        if ((*i)->id() == id)
        {
            return false;
        }
    }

    return true;
}


//----------------------------------------------------------------------------

    // Returns a list of connector Ids of all the connectors of type
    // 'type' attached to the shape with the ID 'shapeId'.
void Router::attachedConns(IntList &conns, const unsigned int shapeId,
        const unsigned int type)
{
    ConnRefList::const_iterator fin = connRefs.end();
    for (ConnRefList::const_iterator i = connRefs.begin(); i != fin; ++i) 
    {
        std::pair<Obstacle *, Obstacle *> anchors = (*i)->endpointAnchors();

        if ((type & runningTo) &&
                (anchors.second && (anchors.second->id() == shapeId)))
        {
            conns.push_back((*i)->id());
        }
        else if ((type & runningFrom) &&
                (anchors.first && (anchors.first->id() == shapeId)))
        {
            conns.push_back((*i)->id());
        }
    }
}


    // Returns a list of shape Ids of all the shapes attached to the
    // shape with the ID 'shapeId' with connection type 'type'.
void Router::attachedShapes(IntList &shapes, const unsigned int shapeId,
        const unsigned int type)
{
    ConnRefList::const_iterator fin = connRefs.end();
    for (ConnRefList::const_iterator i = connRefs.begin(); i != fin; ++i) 
    {
        std::pair<Obstacle *, Obstacle *> anchors = (*i)->endpointAnchors();

        if ((type & runningTo) &&
                (anchors.second && (anchors.second->id() == shapeId)))
        {
            if (anchors.first)
            {
                // Only if there is a shape attached to the other end.
                shapes.push_back(anchors.first->id());
            }
        }
        else if ((type & runningFrom) &&
            (anchors.first && (anchors.first->id() == shapeId)))
        {
            if (anchors.second)
            {
                // Only if there is a shape attached to the other end.
                shapes.push_back(anchors.second->id());
            }
        }
    }
}


    // It's intended this function is called after visibility changes 
    // resulting from shape movement have happened.  It will alert 
    // rerouted connectors (via a callback) that they need to be redrawn.
void Router::rerouteAndCallbackConnectors(void)
{
    ConnRefList reroutedConns;
    ConnRefList::const_iterator fin = connRefs.end();
    
    this->m_conn_reroute_flags.alertConns();

    // Updating the orthogonal visibility graph if necessary. 
    regenerateStaticBuiltGraph();

    for (ConnRefList::const_iterator i = connRefs.begin(); i != fin; ++i) 
    {
        (*i)->freeActivePins();
    }

    // Calculate and return connectors that are part of hyperedges and will
    // be completely rerouted by that code and thus don't need to have routes
    // generated here.
    ConnRefSet hyperedgeConns =
            m_hyperedge_rerouter.calcHyperedgeConnectors();

    timers.Register(tmOrthogRoute, timerStart);
    unsigned int totalConns = connRefs.size();
    unsigned int numOfReroutedConns = 0;
    for (ConnRefList::const_iterator i = connRefs.begin(); i != fin; ++i) 
    {
        // Progress reporting and continuation check.
        performContinuationCheck(TransactionPhaseRouteSearch, 
                numOfReroutedConns, totalConns);
        ++numOfReroutedConns;

        ConnRef *connector = *i;
        if (hyperedgeConns.find(connector) != hyperedgeConns.end())
        {
            // This will be rerouted by the hyperedge code, so do nothing.
            continue;
        }

        if (connector->hasFixedRoute())
        {
            // We don't reroute connectors with fixed routes.
            continue;
        }

        connector->m_needs_repaint = false;
        bool rerouted = connector->generatePath();
        if (rerouted)
        {
            reroutedConns.push_back(connector);
        }
    }
    timers.Stop();


    // Perform any complete hyperedge rerouting that has been requested.
    m_hyperedge_rerouter.performRerouting();

    // Find and reroute crossing connectors if crossing penalties are set.
    improveCrossings();

    if (routingOption(improveHyperedgeRoutesMovingJunctions))
    {
        improveHyperedgeRoutes(this);
    }

    // Perform centring and nudging for orthogonal routes.
    improveOrthogonalRoutes(this);

    // Alert connectors that they need redrawing.
    fin = reroutedConns.end();
    for (ConnRefList::const_iterator i = reroutedConns.begin(); i != fin; ++i) 
    {
        (*i)->m_needs_repaint = true;
        (*i)->performCallback();
    }

    // Progress reporting.
    performContinuationCheck(TransactionPhaseCompleted, 1, 1);
}

// Type holding a cost estimate and ConnRef.
typedef std::pair<double, ConnRef *> ConnCostRef;

// A comparison class used to order a set of ConnCostRefs.
class CmpConnCostRef
{
    public:
        CmpConnCostRef()
        {
        }
        bool operator() (const ConnCostRef& u, const ConnCostRef& v) const
        {
            return (u.second->id() < v.second->id());
        }
};

typedef std::set<ConnCostRef, CmpConnCostRef> ConnCostRefSet;
typedef std::list<ConnCostRefSet> ConnCostRefSetList;

static ConnCostRefSetList::iterator setForCrossingConn(
        ConnCostRefSetList &setList, ConnCostRef conn)
{
    for (ConnCostRefSetList::iterator it = setList.begin(); 
            it != setList.end(); ++it)
    {
        if (it->find(conn) != it->end())
        {
            return it;
        }
    }
    return setList.end();
}


static void addCrossingConnsToSetList(ConnCostRefSetList &setList, ConnCostRef conn1,
        ConnCostRef conn2)
{
    ConnCostRefSetList::iterator set1 = setForCrossingConn(setList, conn1);
    ConnCostRefSetList::iterator set2 = setForCrossingConn(setList, conn2);
    
    if ((set1 == setList.end()) && (set2 == setList.end()))
    {
        ConnCostRefSet newSet;
        newSet.insert(conn1);
        newSet.insert(conn2);
        setList.push_back(newSet);
    }
    else if ((set1 != setList.end()) && (set2 == setList.end()))
    {
        set1->insert(conn2);
    }
    else if ((set1 == setList.end()) && (set2 != setList.end()))
    {
        set2->insert(conn1);
    }
    else
    {
        COLA_ASSERT((set1 != setList.end()) && (set2 != setList.end()));
        set1->insert(set2->begin(), set2->end());
        setList.erase(set2);
    }
}

static bool connsKnownToCross(ConnCostRefSetList &setList, ConnCostRef conn1,
        ConnCostRef conn2)
{
    ConnCostRefSetList::iterator set1 = setForCrossingConn(setList, conn1);
    ConnCostRefSetList::iterator set2 = setForCrossingConn(setList, conn2);

    if ((set1 == set2) && (set1 != setList.end()))
    {
        return true;
    }
    return false;
}


static double cheapEstimatedCost(ConnRef *lineRef)
{
    // We use an estimate of overall connector length, reduced by the length 
    // of the shortest segment.  This causes straight line connectors to be 
    // left alone.  It also causes connectors running close to things to get
    // rerouted rather than ones in wide channels, which hopefully has the 
    // effect of us not leaving a heavily restricted connector in place and 
    // trying to route around it with others.
    bool isPolyLine = (lineRef->routingType() == ConnType_PolyLine);
    const PolyLine& route = lineRef->displayRoute();
    double smallestSegment = DBL_MAX;
    double length = 0;

    for (size_t i = 1; i < route.size(); ++i)
    {
        const Point& a = route.ps[i - 1];
        const Point& b = route.ps[i];

        double segmentLength = (isPolyLine) ? 
                euclideanDist(a, b) : manhattanDist(a, b);
        smallestSegment = std::min(smallestSegment, segmentLength);
        length += segmentLength;
    }
    return length - smallestSegment;
}


void Router::performContinuationCheck(unsigned int phaseNumber, 
        unsigned int stepNumber, unsigned int totalSteps)
{
    // Compute the elapsed time in msec since the beginning of the transaction.
    unsigned int elapsedMsec = (unsigned int) 
            ((clock() - m_transaction_start_time) / 
             (CLOCKS_PER_SEC / (double) 1000));

    bool shouldContinue = shouldContinueTransactionWithProgress(elapsedMsec, 
            phaseNumber, TransactionPhaseCompleted, 
            stepNumber / (double)totalSteps);
    if (shouldContinue == false)
    {
        // Host program has asked us not to continue the transaction.
        m_abort_transaction = true;
    }
}


bool Router::shouldContinueTransactionWithProgress(unsigned int elapsedTime, 
        unsigned int phaseNumber, unsigned int totalPhases, 
        double proportion)
{
    COLA_UNUSED(elapsedTime);
    COLA_UNUSED(phaseNumber);
    COLA_UNUSED(totalPhases);
    COLA_UNUSED(proportion);

#if 0
    printf("Progress: %8u, phase %u of %u... %.2f%%\n", elapsedTime,
            phaseNumber, totalPhases, proportion * 100);
#endif

    // We always continue.  Subclasses can override this behaviour.
    return true;
}


class CmpOrderedConnCostRef
{
    public:
        CmpOrderedConnCostRef()
        {
        }
        bool operator() (const ConnCostRef& u, const ConnCostRef& v) const
        {
            return (u.first < v.first);
        }
};
typedef std::list<ConnCostRef> ConnCostRefList;


void Router::improveCrossings(void)
{
    const double crossing_penalty = routingParameter(crossingPenalty);
    const double shared_path_penalty = routingParameter(fixedSharedPathPenalty);
    if ((crossing_penalty == 0) && (shared_path_penalty == 0))
    {
        // No penalties, return.
        return;
    }
    
    unsigned int numOfConns = connRefs.size();
    unsigned int numOfConnsChecked = 0;

    // Find crossings and reroute connectors.
    m_in_crossing_rerouting_stage = true;
    ConnCostRefSet crossingConns;
    ConnCostRefSetList fixedSharedPathConns;
    ConnRefList::iterator fin = connRefs.end();
    for (ConnRefList::iterator i = connRefs.begin(); i != fin; ++i) 
    {
        // Progress reporting and continuation check.
        ++numOfConnsChecked;
        performContinuationCheck(TransactionPhaseCrossingDetection,
                numOfConnsChecked, numOfConns);
        if (m_abort_transaction)
        {
            m_in_crossing_rerouting_stage = false;
            return;
        }
    
        Avoid::Polygon& iRoute = (*i)->routeRef();
        if (iRoute.size() == 0)
        {
            // Rerouted hyperedges will have an empty route.
            // We can't reroute these.
            continue;
        }
        ConnCostRef iCostRef = std::make_pair(cheapEstimatedCost(*i), *i);
        ConnRefList::iterator j = i;
        for (++j; j != fin; ++j) 
        {
            ConnCostRef jCostRef = std::make_pair(cheapEstimatedCost(*j), *j);
            if (connsKnownToCross(fixedSharedPathConns, iCostRef, jCostRef) ||
                    (crossingConns.count(iCostRef) && 
                     crossingConns.count(jCostRef)))
            {
                // We already know both these have crossings.
                continue;
            }
            // Determine if this pair cross.
            Avoid::Polygon& jRoute = (*j)->routeRef();
            ConnectorCrossings cross(iRoute, true, jRoute, *i, *j);
            for (size_t jInd = 1; jInd < jRoute.size(); ++jInd)
            {
                const bool finalSegment = ((jInd + 1) == jRoute.size());
                cross.countForSegment(jInd, finalSegment);
                
                if ((shared_path_penalty > 0) && 
                    (cross.crossingFlags & CROSSING_SHARES_PATH) && 
                    (cross.crossingFlags & CROSSING_SHARES_FIXED_SEGMENT) &&
                    (m_routing_options[penaliseOrthogonalSharedPathsAtConnEnds] || 
                     !(cross.crossingFlags & CROSSING_SHARES_PATH_AT_END))) 
                {
                    // We are penalising fixedSharedPaths and there is a
                    // fixedSharedPath.

                    if ((cross.crossingFlags & CROSSING_SHARES_PATH_AT_END) &&
                            (cross.firstSharedPathAtEndLength !=
                                cross.secondSharedPathAtEndLength))
                    {
                        // Get costs of each path from the crossings object.
                        // For shared paths that cross at the end, these will 
                        // be the shared path length minus some amount if the
                        // diverging segment is not a bend.  For every other 
                        // path it will be cheapEstimatedCost().  We want 
                        // low costs for straight segments so these are not 
                        // rerouted.
                        iCostRef.first = cross.firstSharedPathAtEndLength;
                        jCostRef.first = cross.secondSharedPathAtEndLength;
                    }
                    addCrossingConnsToSetList(fixedSharedPathConns, 
                            iCostRef, jCostRef);
                    break;
                }
                else if ((crossing_penalty > 0) && (cross.crossingCount > 0))
                {
                    // We are penalising crossings and this is a crossing.
                    crossingConns.insert(iCostRef);
                    crossingConns.insert(jCostRef);
                    break;
                }
            }
        }
    }

    unsigned int numOfConnsToReroute = 1;
    unsigned int numOfConnsRerouted = 1;
    // At this point we have a list containing sets of interacting (crossing) 
    // connectors.  The first element in each set is the ideal candidate to 
    // keep the route for.  The others should be rerouted.  We do this via
    // two passes: 1) clear existing routes and free pin assignments, and
    // 2) compute new routes.
    for (int pass = 0; pass < 2; ++pass)
    {
        // First deal with the fixed shared paths, if there are any.
        for (ConnCostRefSetList::iterator setIt = fixedSharedPathConns.begin(); 
                setIt != fixedSharedPathConns.end(); ++setIt)
        {
            ConnCostRefList orderedConnList(setIt->begin(), setIt->end());
            orderedConnList.sort(CmpOrderedConnCostRef());
            for (ConnCostRefList::iterator connIt = orderedConnList.begin(); 
                    connIt != orderedConnList.end(); ++connIt)
            {
                if (pass == 0)
                {
                    // Remove these connectors from the crossingConns set. 
                    // We reroute them here, so don't need to do it again later.
                    crossingConns.erase(*connIt);
                }

                if (connIt == orderedConnList.begin())
                {
                    // Don't reroute the first connector in each set of 
                    // fixed shared paths.
                    continue;
                }

                ConnRef *conn = connIt->second;
                if (pass == 0)
                {
                    ++numOfConnsToReroute;

                    // Mark the fixed shared path as being invalid.
                    conn->makePathInvalid();
                    
                    // Free the previous path, so it is not noticed by other 
                    // connectors during rerouting.
                    conn->freeRoutes();
                    
                    // Free pin assignments.
                    conn->freeActivePins();
                }
                else if (pass == 1)
                {
                    // Progress reporting and continuation check.
                    performContinuationCheck(TransactionPhaseRerouteSearch, 
                            numOfConnsRerouted, numOfConnsToReroute);
                    if (m_abort_transaction)
                    {
                        m_in_crossing_rerouting_stage = false;
                        return;
                    }
                    ++numOfConnsRerouted;
                    
                    // Recompute this path.
                    conn->generatePath();
                }
            }
        }

        // Deal with crossing connector rerouting after the 
        // fixedSharedPath routing has been completed.
        ConnCostRefList orderedConnList(crossingConns.begin(), 
                crossingConns.end());
        orderedConnList.sort(CmpOrderedConnCostRef());
        for (ConnCostRefList::iterator connIt = orderedConnList.begin(); 
                    connIt != orderedConnList.end(); ++connIt)
        {
            ConnRef *conn = connIt->second;
            if (pass == 0)
            {
                ++numOfConnsToReroute;

                // Mark the crossing connector path as being invalid.
                conn->makePathInvalid();
                // Freeing the routes here means that, if possible, we 
                // reroute all the crossing routes one by one, threading 
                // them through the non-crossing routes to avoid as many 
                // crossings as possible.
                conn->freeRoutes();
                
                // Free pin assignments.
                conn->freeActivePins();
            }
            else if (pass == 1)
            {
                // Progress reporting and continuation check.
                performContinuationCheck(TransactionPhaseRerouteSearch,
                        numOfConnsRerouted, numOfConnsToReroute);
                ++numOfConnsRerouted;
                
                // Recompute this path.
                conn->generatePath();
            }
        }
    }
    m_in_crossing_rerouting_stage = false;
}


void Router::newBlockingShape(const Polygon& poly, int pid)
{
    // o  Check all visibility edges to see if this one shape
    //    blocks them.
    EdgeInf *finish = visGraph.end();
    for (EdgeInf *iter = visGraph.begin(); iter != finish ; )
    {
        EdgeInf *tmp = iter;
        iter = iter->lstNext;

        if (tmp->getDist() != 0)
        {
            std::pair<VertID, VertID> ids(tmp->ids());
            VertID eID1 = ids.first;
            VertID eID2 = ids.second;
            std::pair<Point, Point> points(tmp->points());
            Point e1 = points.first;
            Point e2 = points.second;
            bool blocked = false;

            bool countBorder = false;
            bool ep_in_poly1 = (eID1.isConnPt()) ? 
                    inPoly(poly, e1, countBorder) : false;
            bool ep_in_poly2 = (eID2.isConnPt()) ? 
                    inPoly(poly, e2, countBorder) : false;
            if (ep_in_poly1 || ep_in_poly2)
            {
                // Don't check edges that have a connector endpoint
                // and are inside the shape being added.
                continue;
            }

            bool seenIntersectionAtEndpoint = false;
            for (size_t pt_i = 0; pt_i < poly.size(); ++pt_i)
            {
                size_t pt_n = (pt_i == (poly.size() - 1)) ? 0 : pt_i + 1;
                const Point& pi = poly.ps[pt_i];
                const Point& pn = poly.ps[pt_n];
                if (segmentShapeIntersect(e1, e2, pi, pn, 
                        seenIntersectionAtEndpoint))
                {
                    blocked = true;
                    break;
                }
            }
            if (blocked)
            {
                db_printf("\tRemoving newly blocked edge (by shape %3d)"
                        "... \n\t\t", pid);
                tmp->alertConns();
                tmp->db_print();
                if (InvisibilityGrph)
                {
                    tmp->addBlocker(pid);
                }
                else
                {
                    delete tmp;
                }
            }
        }
    }
}


void Router::checkAllBlockedEdges(int pid)
{
    COLA_ASSERT(InvisibilityGrph);

    for (EdgeInf *iter = invisGraph.begin(); iter != invisGraph.end() ; )
    {
        EdgeInf *tmp = iter;
        iter = iter->lstNext;

        if (tmp->blocker() == -1)
        {
            tmp->alertConns();
            tmp->checkVis();
        }
        else if (tmp->blocker() == pid)
        {
            tmp->checkVis();
        }
    }
}


void Router::checkAllMissingEdges(void)
{
    COLA_ASSERT(!InvisibilityGrph);

    VertInf *first = vertices.connsBegin();

    VertInf *pend = vertices.end();
    for (VertInf *i = first; i != pend; i = i->lstNext)
    {
        VertID iID = i->id;

        // Check remaining, earlier vertices
        for (VertInf *j = first ; j != i; j = j->lstNext)
        {
            VertID jID = j->id;
            if (iID.isConnPt() && !iID.isConnectionPin() && 
                    (iID.objID != jID.objID))
            {
                // Don't keep visibility between edges of different conns
                continue;
            }

            // See if the edge is already there?
            bool found = (EdgeInf::existingEdge(i, j) != NULL);

            if (!found)
            {
                // Didn't already exist, check.
                bool knownNew = true;
                EdgeInf::checkEdgeVisibility(i, j, knownNew);
            }
        }
    }
}


void Router::generateContains(VertInf *pt)
{
    contains[pt->id].clear();
    enclosingClusters[pt->id].clear();

    // Don't count points on the border as being inside.
    bool countBorder = false;

    // Compute enclosing shapes.
    ObstacleList::const_iterator finish = m_obstacles.end();
    for (ObstacleList::const_iterator i = m_obstacles.begin(); i != finish; ++i)
    {
        if (inPoly((*i)->routingPolygon(), pt->point, countBorder))
        {
            contains[pt->id].insert((*i)->id());
        }
    }

    // Computer enclosing Clusters
    ClusterRefList::const_iterator clFinish = clusterRefs.end();
    for (ClusterRefList::const_iterator i = clusterRefs.begin(); 
            i != clFinish; ++i)
    {
        if (inPolyGen((*i)->polygon(), pt->point))
        {
            enclosingClusters[pt->id].insert((*i)->id());
        }
    }
}


void Router::adjustClustersWithAdd(const PolygonInterface& poly, 
        const int p_cluster)
{
    for (VertInf *k = vertices.connsBegin(); k != vertices.shapesBegin();
            k = k->lstNext)
    {
        if (inPolyGen(poly, k->point))
        {
            enclosingClusters[k->id].insert(p_cluster);
        }
    }
}


void Router::adjustClustersWithDel(const int p_cluster)
{
    for (ContainsMap::iterator k = enclosingClusters.begin();
            k != enclosingClusters.end(); ++k)
    {
        (*k).second.erase(p_cluster);
    }
}


void Router::adjustContainsWithAdd(const Polygon& poly, const int p_shape)
{
    // Don't count points on the border as being inside.
    bool countBorder = false;

    for (VertInf *k = vertices.connsBegin(); k != vertices.shapesBegin();
            k = k->lstNext)
    {
        if (inPoly(poly, k->point, countBorder))
        {
            contains[k->id].insert(p_shape);
        }
    }
}


void Router::adjustContainsWithDel(const int p_shape)
{
    for (ContainsMap::iterator k = contains.begin(); k != contains.end(); ++k)
    {
        (*k).second.erase(p_shape);
    }
}


#ifdef SELECTIVE_DEBUG
static double AngleAFromThreeSides(const double a, const double b,
        const double c)
{
    // returns angle A, the angle opposite from side a, in radians
    return acos((pow(b, 2) + pow(c, 2) - pow(a, 2)) / (2 * b * c));
}
#endif

// Given an deleted obstacle, uses a simple heauristic to determine polyline
// connectors that may now have a better path through the region occupied by
// the shape and mark them as needing to be rerouted.
// See the "Incremental Connector Routing" paper which explains this code.
//
void Router::markPolylineConnectorsNeedingReroutingForDeletedObstacle(
        Obstacle *obstacle)
{
    if (RubberBandRouting)
    {
        // When rubber-band routing, we do not reroute connectors that
        // may have a better route, only invalid connectors.
        return;
    }

    COLA_ASSERT(SelectiveReroute);

    // For each connector...
    ConnRefList::const_iterator end = connRefs.end();
    for (ConnRefList::const_iterator it = connRefs.begin(); it != end; ++it)
    {
        ConnRef *conn = (*it);

        if (conn->m_route.empty())
        {
            // Ignore uninitialised connectors.
            continue;
        }
        else if (conn->m_needs_reroute_flag)
        {
            // Already marked, so skip.
            continue;
        }
        else if (conn->routingType() != ConnType_PolyLine)
        {
            // This test only works for polyline connectors, so skip others.
            continue;
        }

        Point start = conn->m_route.ps[0];
        Point end = conn->m_route.ps[conn->m_route.size() - 1];

        double conndist = conn->m_route_dist;

        double estdist;
        double e1, e2;

        // For each vertex of the obstacle...
        VertInf *beginV = obstacle->firstVert();
        VertInf *endV = obstacle->lastVert()->lstNext;
        for (VertInf *i = beginV; i != endV; i = i->lstNext)
        {
            const Point& p1 = i->point;
            const Point& p2 = i->shNext->point;

            double offy;
            double a;
            double b;
            double c;
            double d;

            double min;
            double max;

            if (p1.y == p2.y)
            {
                // Standard case
                offy = p1.y;
                a = start.x;
                b = start.y - offy;
                c = end.x;
                d = end.y - offy;

                min = std::min(p1.x, p2.x);
                max = std::max(p1.x, p2.x);
            }
            else if (p1.x == p2.x)
            {
                // Other Standard case
                offy = p1.x;
                a = start.y;
                b = start.x - offy;
                c = end.y;
                d = end.x - offy;

                min = std::min(p1.y, p2.y);
                max = std::max(p1.y, p2.y);
            }
            else
            {
                // Need to do rotation
                Point n_p2(p2.x - p1.x, p2.y - p1.y);
                Point n_start(start.x - p1.x, start.y - p1.y);
                Point n_end(end.x - p1.x, end.y - p1.y);
                //db_printf("n_p2:    (%.1f, %.1f)\n", n_p2.x, n_p2.y);
                //db_printf("n_start: (%.1f, %.1f)\n", n_start.x, n_start.y);
                //db_printf("n_end:   (%.1f, %.1f)\n", n_end.x, n_end.y);

                double theta = 0 - atan2(n_p2.y, n_p2.x);
                //db_printf("theta = %.2f\n", theta * (180 / PI));

                Point r_p1(0, 0);
                Point r_p2 = n_p2;
                start = n_start;
                end = n_end;

                double cosv = cos(theta);
                double sinv = sin(theta);

                r_p2.x = cosv * n_p2.x - sinv * n_p2.y;
                r_p2.y = cosv * n_p2.y + sinv * n_p2.x;
                start.x = cosv * n_start.x - sinv * n_start.y;
                start.y = cosv * n_start.y + sinv * n_start.x;
                end.x = cosv * n_end.x - sinv * n_end.y;
                end.y = cosv * n_end.y + sinv * n_end.x;
                //db_printf("r_p2:    (%.1f, %.1f)\n", r_p2.x, r_p2.y);
                //db_printf("r_start: (%.1f, %.1f)\n", start.x, start.y);
                //db_printf("r_end:   (%.1f, %.1f)\n", end.x, end.y);

                // This might be slightly off.
                if (fabs(r_p2.y) > 0.0001)
                {
                    db_printf("r_p2.y: %f != 0\n", r_p2.y);
                }
                r_p2.y = 0;

                offy = r_p1.y;
                a = start.x;
                b = start.y - offy;
                c = end.x;
                d = end.y - offy;

                min = std::min(r_p1.x, r_p2.x);
                max = std::max(r_p1.x, r_p2.x);

            }

            double x;
            if ((b + d) == 0)
            {
                db_printf("WARNING: (b + d) == 0\n");
                d = d * -1;
            }

            if ((b == 0) && (d == 0))
            {
                db_printf("WARNING: b == d == 0\n");
                if (((a < min) && (c < min)) ||
                        ((a > max) && (c > max)))
                {
                    // It's going to get adjusted.
                    x = a;
                }
                else
                {
                    continue;
                }
            }
            else
            {
                x = ((b*c) + (a*d)) / (b + d);
            }

            //db_printf("%.1f, %.1f, %.1f, %.1f\n", a, b, c, d);
            //db_printf("x = %.1f\n", x);

            x = std::max(min, x);
            x = std::min(max, x);

            //db_printf("x = %.1f\n", x);

            Point xp;
            if (p1.x == p2.x)
            {
                xp.x = offy;
                xp.y = x;
            }
            else
            {
                xp.x = x;
                xp.y = offy;
            }
            //db_printf("(%.1f, %.1f)\n", xp.x, xp.y);

            e1 = euclideanDist(start, xp);
            e2 = euclideanDist(xp, end);
            estdist = e1 + e2;


            //db_printf("is %.1f < %.1f\n", estdist, conndist);
            if (estdist < conndist)
            {
#ifdef SELECTIVE_DEBUG
                //double angle = AngleAFromThreeSides(dist(start, end),
                //        e1, e2);
                db_printf("[%3d] - Possible better path found (%.1f < %.1f)\n",
                        conn->_id, estdist, conndist);
#endif
                conn->m_needs_reroute_flag = true;
                break;
            }

        }
    }
}


ConnType Router::validConnType(const ConnType select) const
{
    if (select != ConnType_None)
    {
        if ((select == ConnType_Orthogonal) && m_allows_orthogonal_routing)
        {
            return ConnType_Orthogonal;
        }
        else if ((select == ConnType_PolyLine) && m_allows_polyline_routing)
        {
            return ConnType_PolyLine;
        }
    }

    if (m_allows_polyline_routing)
    {
        return ConnType_PolyLine;
    }
    else if (m_allows_orthogonal_routing)
    {
        return ConnType_Orthogonal;
    }
    return ConnType_None;
}


void Router::setRoutingParameter(const RoutingParameter parameter,
        const double value)
{
    COLA_ASSERT(parameter < lastRoutingParameterMarker);
    if (value < 0)
    {
        // Set some sensible parameter value for the parameter being 'active'.
        switch (parameter)
        {
            case segmentPenalty:
                m_routing_parameters[parameter] = 50;
                break;
            case fixedSharedPathPenalty:
                m_routing_parameters[parameter] = 110;
                break;
            case anglePenalty:
                m_routing_parameters[parameter] = 50;
                break;
            case crossingPenalty:
                m_routing_parameters[parameter] = 200;
                break;
            case clusterCrossingPenalty:
                m_routing_parameters[parameter] = 4000;
                break;
            case idealNudgingDistance:
                m_routing_parameters[parameter] = 4.0;
                break;
            default:
                m_routing_parameters[parameter] = 50;
                break;
        }
    }
    else
    {
        m_routing_parameters[parameter] = value;
    }
}


double Router::routingParameter(const RoutingParameter parameter) const
{
    COLA_ASSERT(parameter < lastRoutingParameterMarker);
    return m_routing_parameters[parameter];
}


void Router::setRoutingOption(const RoutingOption option, const bool value)
{
    COLA_ASSERT(option < lastRoutingOptionMarker);
    m_routing_options[option] = value;
}


bool Router::routingOption(const RoutingOption option) const
{
    COLA_ASSERT(option < lastRoutingOptionMarker);
    return m_routing_options[option];
}


void Router::setRoutingPenalty(const RoutingParameter penType,
        const double penValue)
{
    setRoutingParameter(penType, penValue);
}


HyperedgeRerouter *Router::hyperedgeRerouter(void)
{
    return &m_hyperedge_rerouter;
}


bool Router::isInCrossingPenaltyReroutingStage(void) const
{
    return m_in_crossing_rerouting_stage;
}


void Router::printInfo(void)
{
    FILE *fp = stdout;
    fprintf(fp, "\nVisibility Graph info:\n");
    fprintf(fp, "----------------------\n");

    unsigned int currshape = 0;
    int st_shapes = 0;
    int st_vertices = 0;
    int st_endpoints = 0;
    int st_valid_shape_visedges = 0;
    int st_valid_endpt_visedges = 0;
    int st_orthogonal_visedges = 0;
    int st_invalid_visedges = 0;
    VertInf *finish = vertices.end();
    for (VertInf *t = vertices.connsBegin(); t != finish; t = t->lstNext)
    {
        VertID pID = t->id;

        if (!(pID.isConnPt()) && (pID.objID != currshape))
        {
            currshape = pID.objID;
            st_shapes++;
        }
        if (!(pID.isConnPt()))
        {
            st_vertices++;
        }
        else
        {
            // The shape 0 ones are temporary and not considered.
            st_endpoints++;
        }
    }
    for (EdgeInf *t = visGraph.begin(); t != visGraph.end();
            t = t->lstNext)
    {
        std::pair<VertID, VertID> idpair = t->ids();

        if (idpair.first.isConnPt() || idpair.second.isConnPt())
        {
            st_valid_endpt_visedges++;
        }
        else
        {
            st_valid_shape_visedges++;
        }
    }
    for (EdgeInf *t = invisGraph.begin(); t != invisGraph.end();
            t = t->lstNext)
    {
        st_invalid_visedges++;
    }
    for (EdgeInf *t = visOrthogGraph.begin(); t != visOrthogGraph.end();
            t = t->lstNext)
    {
        st_orthogonal_visedges++;
    }
    fprintf(fp, "Number of shapes: %d\n", st_shapes);
    fprintf(fp, "Number of vertices: %d (%d real, %d endpoints)\n",
            st_vertices + st_endpoints, st_vertices, st_endpoints);
    fprintf(fp, "Number of orhtog_vis_edges: %d\n", st_orthogonal_visedges);
    fprintf(fp, "Number of vis_edges: %d (%d valid [%d normal, %d endpt], "
            "%d invalid)\n", st_valid_shape_visedges + st_invalid_visedges +
            st_valid_endpt_visedges, st_valid_shape_visedges +
            st_valid_endpt_visedges, st_valid_shape_visedges,
            st_valid_endpt_visedges, st_invalid_visedges);
    fprintf(fp, "----------------------\n");
    fprintf(fp, "checkVisEdge tally: %d\n", st_checked_edges);
    fprintf(fp, "----------------------\n");

    fprintf(fp, "ADDS:  "); timers.Print(tmAdd, fp);
    fprintf(fp, "DELS:  "); timers.Print(tmDel, fp);
    fprintf(fp, "MOVS:  "); timers.Print(tmMov, fp);
    fprintf(fp, "***S:  "); timers.Print(tmSev, fp);
    fprintf(fp, "PTHS:  "); timers.Print(tmPth, fp);
    fprintf(fp, "OrthogGraph:  "); timers.Print(tmOrthogGraph, fp);
    fprintf(fp, "OrthogRoute:  "); timers.Print(tmOrthogRoute, fp);
    fprintf(fp, "OrthogCentre:  "); timers.Print(tmOrthogCentre, fp);
    fprintf(fp, "OrthogNudge:  "); timers.Print(tmOrthogNudge, fp);
    fprintf(fp, "HyperedgeForest:  "); timers.Print(tmHyperedgeForest, fp);
    fprintf(fp, "HyperedgeMTST:  "); timers.Print(tmHyperedgeMTST, fp);
    fprintf(fp, "HyperedgeImprove:  "); timers.Print(tmHyperedgeImprove, fp);
    fprintf(fp, "\n");
    timers.Reset();
}


static const double LIMIT = 100000000;

static void reduceRange(double& val)
{
    val = std::min(val, LIMIT);
    val = std::max(val, -LIMIT);
}


//=============================================================================
// The following methods are for testing and debugging.


bool Router::existsOrthogonalSegmentOverlap(const bool atEnds)
{
    ConnRefList::iterator fin = connRefs.end();
    for (ConnRefList::iterator i = connRefs.begin(); i != fin; ++i) 
    {
        Avoid::Polygon iRoute = (*i)->displayRoute();
        ConnRefList::iterator j = i;
        for (++j; j != fin; ++j) 
        {
            // Determine if this pair overlap
            Avoid::Polygon jRoute = (*j)->displayRoute();
            ConnectorCrossings cross(iRoute, true, jRoute, *i, *j);
            cross.checkForBranchingSegments = true;
            for (size_t jInd = 1; jInd < jRoute.size(); ++jInd)
            {
                const bool finalSegment = ((jInd + 1) == jRoute.size());
                cross.countForSegment(jInd, finalSegment);
                
                if ((cross.crossingFlags & CROSSING_SHARES_PATH) && 
                    (atEnds || 
                     !(cross.crossingFlags & CROSSING_SHARES_PATH_AT_END))) 
                {
                    // We are looking for fixedSharedPaths and there is a
                    // fixedSharedPath.
                    return true;
                }
            }
        }
    }
    return false;
}


bool Router::existsOrthogonalFixedSegmentOverlap(const bool atEnds)
{
    ConnRefList::iterator fin = connRefs.end();
    for (ConnRefList::iterator i = connRefs.begin(); i != fin; ++i) 
    {
        Avoid::Polygon iRoute = (*i)->displayRoute();
        ConnRefList::iterator j = i;
        for (++j; j != fin; ++j) 
        {
            // Determine if this pair overlap
            Avoid::Polygon jRoute = (*j)->displayRoute();
            ConnectorCrossings cross(iRoute, true, jRoute, *i, *j);
            cross.checkForBranchingSegments = true;
            for (size_t jInd = 1; jInd < jRoute.size(); ++jInd)
            {
                const bool finalSegment = ((jInd + 1) == jRoute.size());
                cross.countForSegment(jInd, finalSegment);
                
                if ((cross.crossingFlags & CROSSING_SHARES_PATH) && 
                    (cross.crossingFlags & CROSSING_SHARES_FIXED_SEGMENT) &&
                    (atEnds || 
                     !(cross.crossingFlags & CROSSING_SHARES_PATH_AT_END))) 
                {
                    // We are looking for fixedSharedPaths and there is a
                    // fixedSharedPath.
                    return true;
                }
            }
        }
    }
    return false;
}


bool Router::existsOrthogonalTouchingPaths(void)
{
    ConnRefList::iterator fin = connRefs.end();
    for (ConnRefList::iterator i = connRefs.begin(); i != fin; ++i) 
    {
        Avoid::Polygon iRoute = (*i)->displayRoute();
        ConnRefList::iterator j = i;
        for (++j; j != fin; ++j) 
        {
            // Determine if this pair overlap
            Avoid::Polygon jRoute = (*j)->displayRoute();
            ConnectorCrossings cross(iRoute, true, jRoute, *i, *j);
            cross.checkForBranchingSegments = true;
            for (size_t jInd = 1; jInd < jRoute.size(); ++jInd)
            {
                const bool finalSegment = ((jInd + 1) == jRoute.size());
                cross.countForSegment(jInd, finalSegment);
                
                if (cross.crossingFlags & CROSSING_TOUCHES) 
                {
                    return true;
                }
            }
        }
    }
    return false;
}


// Counts the number of crossings between all connectors.
//
// If optimisedForConnectorType is set, This will only count crossings 
// between orthogonal connectors if they appear at segment endpoints. 
// Thus, it can be used on raw connectors but not on simplified orthogonal
// connectors.
//
int Router::existsCrossings(const bool optimisedForConnectorType)
{
    int count = 0;
    ConnRefList::iterator fin = connRefs.end();
    for (ConnRefList::iterator i = connRefs.begin(); i != fin; ++i) 
    {
        Avoid::Polygon iRoute = (*i)->displayRoute();
        ConnRefList::iterator j = i;
        for (++j; j != fin; ++j) 
        {
            // Determine if this pair overlap
            Avoid::Polygon jRoute = (*j)->displayRoute();
            ConnRef *iConn = (optimisedForConnectorType) ? *i : NULL;
            ConnRef *jConn = (optimisedForConnectorType) ? *j : NULL;
            ConnectorCrossings cross(iRoute, true, jRoute, iConn, jConn);
            cross.checkForBranchingSegments = true;
            for (size_t jInd = 1; jInd < jRoute.size(); ++jInd)
            {
                const bool finalSegment = ((jInd + 1) == jRoute.size());
                
                // Normal crossings aren't counted if we pass the pointers
                // for the connectors, so don't pass them.
                cross.countForSegment(jInd, finalSegment);
                
                count += cross.crossingCount;
            }
        }
    }
    return count;
}

// Looks for non-orthogonal segments in orthogonal connectors and 
// returns true if it finds any.
bool Router::existsInvalidOrthogonalPaths(void)
{
    // For each connector...
    ConnRefList::iterator fin = connRefs.end();
    for (ConnRefList::iterator i = connRefs.begin(); i != fin; ++i) 
    {
        // If it is an orthogonal connector...
        if ((*i)->routingType() == Avoid::ConnType_Orthogonal)
        {
            // Check each segment of the path...
            Avoid::Polygon iRoute = (*i)->displayRoute();
            for (size_t iInd = 1; iInd < iRoute.size(); ++iInd)
            {
                // And if it isn't either vertical or horizontal...
                if ( (iRoute.at(iInd - 1).x != iRoute.at(iInd).x) &&
                     (iRoute.at(iInd - 1).y != iRoute.at(iInd).y) )
                {
                    // Then we've found an invalid path.
                    return true;
                }
            }
        }
    }
    return false;
}


void Router::setTopologyAddon(TopologyAddonInterface *topologyAddon)
{
    COLA_ASSERT(m_topology_addon);
    delete m_topology_addon;
    if (topologyAddon)
    {
        m_topology_addon = topologyAddon->clone();
    }
    else
    {
        m_topology_addon = new TopologyAddonInterface();
    }
}

void Router::improveOrthogonalTopology(void)
{
    COLA_ASSERT(m_topology_addon);
    m_topology_addon->improveOrthogonalTopology(this);
}

void Router::outputInstanceToSVG(std::string instanceName)
{
    std::string filename;
    if (!instanceName.empty())
    {
        filename = instanceName;
    }
    else
    {
        filename = "libavoid-debug";
    }
    filename += ".svg";
    FILE *fp = fopen(filename.c_str(), "w");

    if (fp == NULL)
    {
        return;
    }

    double minX = LIMIT;
    double minY = LIMIT;
    double maxX = -LIMIT;
    double maxY = -LIMIT;

    VertInf *curr = vertices.connsBegin();
    while (curr)
    {
        Point p = curr->point;

        reduceRange(p.x);
        reduceRange(p.y);
        
        if (p.x > -LIMIT)
        {
            minX = std::min(minX, p.x);
        }
        if (p.x < LIMIT)
        {
            maxX = std::max(maxX, p.x);
        }
        if (p.y > -LIMIT)
        {
            minY = std::min(minY, p.y);
        }
        if (p.y < LIMIT)
        {
            maxY = std::max(maxY, p.y);
        }
        curr = curr->lstNext;
    }
    minX -= 8;
    minY -= 8;
    maxX += 8;
    maxY += 8;

    fprintf(fp, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
    fprintf(fp, "<svg xmlns:inkscape=\"http://www.inkscape.org/namespaces/inkscape\" xmlns=\"http://www.w3.org/2000/svg\" width=\"100%%\" height=\"100%%\" viewBox=\"%g %g %g %g\">\n", minX, minY, maxX - minX, maxY - minY);

    // Output source code to generate this instance of the router.
    fprintf(fp, "<!-- Source code to generate this instance:\n");
    fprintf(fp, "#include \"libavoid/libavoid.h\"\n");
    if (m_topology_addon->outputCode(NULL))
    {
        fprintf(fp, "#include \"libcola/cola.h\"\n");
        fprintf(fp, "#include \"libtopology/orthogonal_topology.h\"\n");
        fprintf(fp, "using namespace cola;\n");
    }
    fprintf(fp, "using namespace Avoid;\n");
    fprintf(fp, "int main(void) {\n");
    fprintf(fp, "    Router *router = new Router(\n");
    fprintf(fp, "            PolyLineRouting | OrthogonalRouting);\n");
    for (size_t p = 0; p < lastRoutingParameterMarker; ++p)
    {
        fprintf(fp, "    router->setRoutingParameter((RoutingParameter)%lu, %g);\n", 
                (unsigned long)p, m_routing_parameters[p]);
    }
    for (size_t p = 0; p < lastRoutingOptionMarker; ++p)
    {
        fprintf(fp, "    router->setRoutingOption((RoutingOption)%lu, %s);\n", 
                (unsigned long)p, (m_routing_options[p]) ? "true" : "false");
    }
    fprintf(fp, "    Polygon polygon;\n");
    fprintf(fp, "    ConnRef *connRef = NULL;\n");
    fprintf(fp, "    ConnEnd srcPt;\n");
    fprintf(fp, "    ConnEnd dstPt;\n");
    ClusterRefList::reverse_iterator revClusterRefIt = clusterRefs.rbegin();
    while (revClusterRefIt != clusterRefs.rend())
    {
        ClusterRef *cRef = *revClusterRefIt;
        fprintf(fp, "    Polygon polygon.resize(%lu);\n", 
                (unsigned long)cRef->polygon().size());
        for (size_t i = 0; i <cRef->polygon().size(); ++i)
        {
            fprintf(fp, "    polygon.ps[%lu] = Point(%g, %g);\n", 
                    (unsigned long)i, cRef->polygon().at(i).x,
                    cRef->polygon().at(i).y);
        }
        fprintf(fp, "    new ClusterRef(router, polygon, %u);\n", cRef->id());
        ++revClusterRefIt;
    }
    ObstacleList::reverse_iterator revObstacleIt = m_obstacles.rbegin();
    while (revObstacleIt != m_obstacles.rend())
    {
        Obstacle *obstacle = *revObstacleIt;
        obstacle->outputCode(fp);
        ++revObstacleIt;
    }
    ConnRefList::reverse_iterator revConnRefIt = connRefs.rbegin();
    while (revConnRefIt != connRefs.rend())
    {
        ConnRef *connRef = *revConnRefIt;
        connRef->outputCode(fp);
        ++revConnRefIt;
    }

    m_topology_addon->outputCode(fp);

    fprintf(fp, "    router->processTransaction();\n");
    fprintf(fp, "    router->outputInstanceToSVG();\n");

    m_topology_addon->outputDeletionCode(fp);
    fprintf(fp, "    delete router;\n");
    fprintf(fp, "    return 0;\n");
    fprintf(fp, "};\n");
    fprintf(fp, "-->\n");

    fprintf(fp, "<g inkscape:groupmode=\"layer\" "
            "inkscape:label=\"Clusters\">\n");
    revClusterRefIt = clusterRefs.rbegin();
    while (revClusterRefIt != clusterRefs.rend())
    {
        ClusterRef *cRef = *revClusterRefIt;
        fprintf(fp, "<path id=\"cluster-%u\" style=\"stroke-width: 1px; "
                "stroke: black; fill: green; fill-opacity: 0.1;\" d=\"", 
                cRef->id());
        for (size_t i = 0; i < cRef->polygon().size(); ++i)
        {
            fprintf(fp, "%c %g %g ", ((i == 0) ? 'M' : 'L'), 
                    cRef->polygon().at(i).x, cRef->polygon().at(i).y);
        }
        fprintf(fp, "Z\" />\n");

        fprintf(fp, "<path id=\"cluster-%u\" style=\"stroke-width: 1px; "
                "stroke: black; fill: green; fill-opacity: 0.1;\" d=\"", 
                cRef->id());
        for (size_t i = 0; i < cRef->rectangularPolygon().size(); ++i)
        {
            fprintf(fp, "%c %g %g ", ((i == 0) ? 'M' : 'L'), 
                    cRef->rectangularPolygon().at(i).x, 
                    cRef->rectangularPolygon().at(i).y);
        }
        fprintf(fp, "Z\" />\n");
        ++revClusterRefIt;
    }
    fprintf(fp, "</g>\n");

    fprintf(fp, "<g inkscape:groupmode=\"layer\" "
    "style=\"display: none;\" "
            "inkscape:label=\"ShapePolygons\">\n");
    ObstacleList::iterator obstacleIt = m_obstacles.begin();
    while (obstacleIt != m_obstacles.end())
    {
        Obstacle *obstacle = *obstacleIt;
        bool isShape = (NULL != dynamic_cast<ShapeRef *> (obstacle));

        if ( ! isShape )
        {
            // Don't output obstacles here, for now.
            ++obstacleIt;
            continue;
        }

        fprintf(fp, "<path id=\"poly-%u\" style=\"stroke-width: 1px; "
                "stroke: black; fill: %s; fill-opacity: 0.3;\" d=\"", 
                obstacle->id(), (isShape) ? "grey" : "red");
        for (size_t i = 0; i < obstacle->polygon().size(); ++i)
        {
            fprintf(fp, "%c %g %g ", ((i == 0) ? 'M' : 'L'), 
                    obstacle->polygon().at(i).x, obstacle->polygon().at(i).y);
        }
        fprintf(fp, "Z\" />\n");
        ++obstacleIt;
    }
    fprintf(fp, "</g>\n");

    fprintf(fp, "<g inkscape:groupmode=\"layer\" "
    "style=\"display: none;\" "
            "inkscape:label=\"ObstaclePolygons\">\n");
    obstacleIt = m_obstacles.begin();
    while (obstacleIt != m_obstacles.end())
    {
        Obstacle *obstacle = *obstacleIt;
        bool isShape = (NULL != dynamic_cast<ShapeRef *> (obstacle));

        if ( ! isShape )
        {
            // Don't output obstacles here, for now.
            ++obstacleIt;
            continue;
        }

        Polygon polygon = obstacle->routingPolygon();
        fprintf(fp, "<path id=\"poly-%u\" style=\"stroke-width: 1px; "
                "stroke: black; fill: %s; fill-opacity: 0.3;\" d=\"", 
                obstacle->id(), (isShape) ? "grey" : "red");
        for (size_t i = 0; i < polygon.size(); ++i)
        {
            fprintf(fp, "%c %g %g ", ((i == 0) ? 'M' : 'L'), 
                    polygon.at(i).x, polygon.at(i).y);
        }
        fprintf(fp, "Z\" />\n");
        ++obstacleIt;
    }
    fprintf(fp, "</g>\n");

    fprintf(fp, "<g inkscape:groupmode=\"layer\" "
            "style=\"display: none;\" "
            "inkscape:label=\"IdealJunctions\">\n");
    for (ObstacleList::iterator obstacleIt = m_obstacles.begin();
            obstacleIt != m_obstacles.end(); ++obstacleIt)
    {
        JunctionRef *junction = dynamic_cast<JunctionRef *> (*obstacleIt);
        if (junction)
        {
            fprintf(fp, "<circle id=\"idealJunction-%u\" cx=\"%g\" cy=\"%g\" "
                    "r=\"8\" style=\"stroke: none; fill: %s; "
                    "fill-opacity: 0.5;\"  />\n", junction->id(), 
                    junction->recommendedPosition().x, 
                    junction->recommendedPosition().y, "green");
        }

    }
    fprintf(fp, "</g>\n");

    fprintf(fp, "<g inkscape:groupmode=\"layer\" "
            "inkscape:label=\"ObstacleRects\">\n");
    obstacleIt = m_obstacles.begin();
    while (obstacleIt != m_obstacles.end())
    {
        Obstacle *obstacle = *obstacleIt;
        bool isShape = (NULL != dynamic_cast<ShapeRef *> (obstacle));

        if ( ! isShape )
        {
            // Don't output obstacles here, for now.
            ++obstacleIt;
            continue;
        }

        Box bBox = obstacle->routingBox();

        fprintf(fp, "<rect id=\"rect-%u\" x=\"%g\" y=\"%g\" width=\"%g\" "
                "height=\"%g\" style=\"stroke-width: 1px; stroke: black; "
                "fill: grey; stroke-opacity: 0.1; fill-opacity: 0.1;\" />\n",
                obstacle->id(), bBox.min.x, bBox.min.y, 
                bBox.max.x - bBox.min.x, bBox.max.y - bBox.min.y);
        ++obstacleIt;
    }
    fprintf(fp, "</g>\n");

    fprintf(fp, "<g inkscape:groupmode=\"layer\" "
            "inkscape:label=\"VisGraph\""
            ">\n");
    EdgeInf *finish = NULL;
    fprintf(fp, "<g inkscape:groupmode=\"layer\" "
            "style=\"display: none;\" "
            "inkscape:label=\"VisGraph-shape\""
            ">\n");
    finish = visGraph.end();
    for (EdgeInf *t = visGraph.begin(); t != finish; t = t->lstNext)
    {
        std::pair<VertID, VertID> ids = t->ids();
        bool isConn = ids.first.isConnPt() || ids.second.isConnPt();
        if (isConn)
        {
            continue;
        }
        std::pair<Point, Point> ptpair = t->points();
        Point p1 = ptpair.first;
        Point p2 = ptpair.second;
        
        reduceRange(p1.x);
        reduceRange(p1.y);
        reduceRange(p2.x);
        reduceRange(p2.y);
        
        fprintf(fp, "<path d=\"M %g %g L %g %g\" "
                "style=\"fill: none; stroke: %s; stroke-width: 1px;\" />\n", 
                p1.x, p1.y, p2.x, p2.y,
                (ids.first.isConnPt() || ids.second.isConnPt()) ? "blue" :
                "red");
    }
    fprintf(fp, "</g>\n");
    fprintf(fp, "<g inkscape:groupmode=\"layer\" "
            "style=\"display: none;\" "
            "inkscape:label=\"VisGraph-conn\""
            ">\n");
    finish = visGraph.end();
    for (EdgeInf *t = visGraph.begin(); t != finish; t = t->lstNext)
    {
        std::pair<VertID, VertID> ids = t->ids();
        bool isConn = ids.first.isConnPt() || ids.second.isConnPt();
        if (!isConn)
        {
            continue;
        }
        std::pair<Point, Point> ptpair = t->points();
        Point p1 = ptpair.first;
        Point p2 = ptpair.second;
        
        reduceRange(p1.x);
        reduceRange(p1.y);
        reduceRange(p2.x);
        reduceRange(p2.y);
        
        fprintf(fp, "<path d=\"M %g %g L %g %g\" "
                "style=\"fill: none; stroke: %s; stroke-width: 1px;\" />\n", 
                p1.x, p1.y, p2.x, p2.y,
                (ids.first.isConnPt() || ids.second.isConnPt()) ? "blue" :
                "red");
    }
    fprintf(fp, "</g>\n");
    fprintf(fp, "</g>\n");

    fprintf(fp, "<g inkscape:groupmode=\"layer\" "
            "style=\"display: none;\" "
            "inkscape:label=\"OrthogVisGraph\">\n");
    finish = visOrthogGraph.end();
    for (EdgeInf *t = visOrthogGraph.begin(); t != finish; t = t->lstNext)
    {
        std::pair<Point, Point> ptpair = t->points();
        Point p1 = ptpair.first;
        Point p2 = ptpair.second;
        
        reduceRange(p1.x);
        reduceRange(p1.y);
        reduceRange(p2.x);
        reduceRange(p2.y);
        
        std::pair<VertID, VertID> ids = t->ids();

        fprintf(fp, "<path d=\"M %g %g L %g %g\" "
                "style=\"fill: none; stroke: %s; stroke-width: 1px;\" />\n", 
                p1.x, p1.y, p2.x, p2.y,
                (ids.first.isConnPt() || ids.second.isConnPt()) ? "green" : 
                "red");
    }
    fprintf(fp, "</g>\n");

    fprintf(fp, "<g inkscape:groupmode=\"layer\" "
            "style=\"display: none;\" "
            "inkscape:label=\"RawConnectors\""
            ">\n");
    ConnRefList::iterator connRefIt = connRefs.begin();
    while (connRefIt != connRefs.end())
    {
        ConnRef *connRef = *connRefIt;
    
        PolyLine route = connRef->route();
        if (!route.empty())
        {
            fprintf(fp, "<path id=\"raw-%u\" d=\"M %g %g ", connRef->id(),
                    route.ps[0].x, route.ps[0].y);
            for (size_t i = 1; i < route.size(); ++i)
            {
                fprintf(fp, "L %g %g ", route.ps[i].x, route.ps[i].y);
            }
            fprintf(fp, "\" ");
            if (connRef->src() && connRef->dst())
            {
                fprintf(fp, "debug=\"src: %d dst: %d\" ",
                        connRef->src()->visDirections,
                        connRef->dst()->visDirections);
            }
            fprintf(fp, "style=\"fill: none; stroke: black; "
                    "stroke-width: 1px;\" />\n");
        }

        ++connRefIt;
    }
    fprintf(fp, "</g>\n");

    fprintf(fp, "<g inkscape:groupmode=\"layer\" "
            "style=\"display: none;\" "
            "inkscape:label=\"CurvedDisplayConnectors\""
            ">\n");
    connRefIt = connRefs.begin();
    while (connRefIt != connRefs.end())
    {
        ConnRef *connRef = *connRefIt;
    
        PolyLine route = connRef->displayRoute().curvedPolyline(8);
        if (!route.empty())
        {
            fprintf(fp, "<path id=\"curved-%u\" d=\"M %g %g ", connRef->id(),
                    route.ps[0].x, route.ps[0].y);
            for (size_t i = 1; i < route.size(); ++i)
            {
                if (route.ts[i] == 'C')
                {
                    fprintf(fp, "%c %g %g %g %g %g %g", route.ts[i], 
                            route.ps[i].x, route.ps[i].y,
                            route.ps[i+1].x, route.ps[i+1].y,
                            route.ps[i+2].x, route.ps[i+2].y);
                    i += 2;
                }
                else
                {
                    fprintf(fp, "%c %g %g ", route.ts[i], 
                            route.ps[i].x, route.ps[i].y);
                }
            }
            fprintf(fp, "\" ");
            if (connRef->src() && connRef->dst())
            {
                fprintf(fp, "debug=\"src: %d dst: %d\" ",
                        connRef->src()->visDirections,
                        connRef->dst()->visDirections);
            }
            fprintf(fp, "style=\"fill: none; stroke: black; "
                    "stroke-width: 1px;\" />\n");
        }
        
        ++connRefIt;
    }
    fprintf(fp, "</g>\n");


    fprintf(fp, "<g inkscape:groupmode=\"layer\" "
            "inkscape:label=\"DisplayConnectors\""
            ">\n");
    connRefIt = connRefs.begin();
    while (connRefIt != connRefs.end())
    {
        ConnRef *connRef = *connRefIt;
    
        PolyLine route = connRef->displayRoute();
        if (!route.empty())
        {
            fprintf(fp, "<path id=\"disp-%u\" d=\"M %g %g ", connRef->id(),
                    route.ps[0].x, route.ps[0].y);
            for (size_t i = 1; i < route.size(); ++i)
            {
                fprintf(fp, "L %g %g ", route.ps[i].x, route.ps[i].y);
            }
            fprintf(fp, "\" ");
            if (connRef->src() && connRef->dst())
            {
                fprintf(fp, "debug=\"src: %d dst: %d\" ",
                        connRef->src()->visDirections,
                        connRef->dst()->visDirections);
            }
            fprintf(fp, "style=\"fill: none; stroke: black; "
                    "stroke-width: 1px;\" />\n");
        }
        
        ++connRefIt;
    }
    fprintf(fp, "</g>\n");

    fprintf(fp, "<g inkscape:groupmode=\"layer\" "
            "inkscape:label=\"ConnectorCheckpoints\""
            ">\n");
    connRefIt = connRefs.begin();
    while (connRefIt != connRefs.end())
    {
        ConnRef *connRef = *connRefIt;
    
        for (size_t i = 0; i < connRef->m_checkpoints.size(); ++i)
        {
            fprintf(fp, "<circle id=\"checkpoint-%u-%d\" cx=\"%g\" cy=\"%g\" "
                    "r=\"8\" style=\"stroke: none; fill: red; "
                    "fill-opacity: 0.25;\"  />\n", connRef->id(), (int) i,
                    connRef->m_checkpoints[i].point.x, 
                    connRef->m_checkpoints[i].point.y);
        }
        
        ++connRefIt;
    }
    fprintf(fp, "</g>\n");


    fprintf(fp, "</svg>\n");
    fclose(fp);
    //printInfo();
}

void Router::outputDiagramSVG(std::string instanceName, LineReps *lineReps)
{
    std::string filename;
    if (!instanceName.empty())
    {
        filename = instanceName;
    }
    else
    {
        filename = "libavoid-diagram";
    }
    filename += ".svg";
    FILE *fp = fopen(filename.c_str(), "w");

    if (fp == NULL)
    {
        return;
    }

    double minX = LIMIT;
    double minY = LIMIT;
    double maxX = -LIMIT;
    double maxY = -LIMIT;

    VertInf *curr = vertices.connsBegin();
    while (curr)
    {
        Point p = curr->point;

        reduceRange(p.x);
        reduceRange(p.y);
        
        if (p.x > -LIMIT)
        {
            minX = std::min(minX, p.x);
        }
        if (p.x < LIMIT)
        {
            maxX = std::max(maxX, p.x);
        }
        if (p.y > -LIMIT)
        {
            minY = std::min(minY, p.y);
        }
        if (p.y < LIMIT)
        {
            maxY = std::max(maxY, p.y);
        }
        curr = curr->lstNext;
    }
    minX -= 8;
    minY -= 8;
    maxX += 8;
    maxY += 8;

    fprintf(fp, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
    fprintf(fp, "<svg xmlns:inkscape=\"http://www.inkscape.org/namespaces/inkscape\" xmlns=\"http://www.w3.org/2000/svg\" width=\"100%%\" height=\"100%%\" viewBox=\"%g %g %g %g\">\n", minX, minY, maxX - minX, maxY - minY);

    fprintf(fp, "<g inkscape:groupmode=\"layer\" "
            "inkscape:label=\"ShapesRect\">\n");
    ObstacleList::iterator obstacleIt = m_obstacles.begin();
    while (obstacleIt != m_obstacles.end())
    {
        Obstacle *obstacle = *obstacleIt;
        bool isShape = (NULL != dynamic_cast<ShapeRef *> (obstacle));

        if ( ! isShape )
        {
            // Don't output non-shape obstacles here, for now.
            ++obstacleIt;
            continue;
        }

        Box bBox = obstacle->polygon().offsetBoundingBox(0.0);

        fprintf(fp, "<rect id=\"rect-%u\" x=\"%g\" y=\"%g\" width=\"%g\" "
                "height=\"%g\" style=\"stroke-width: 1px; stroke: black; "
                "fill: grey; stroke-opacity: 0.5; fill-opacity: 0.4;\" />\n",
                obstacle->id(), bBox.min.x, bBox.min.y,
                bBox.max.x - bBox.min.x, bBox.max.y - bBox.min.y);
        ++obstacleIt;
    }
    fprintf(fp, "</g>\n");

    fprintf(fp, "<g inkscape:groupmode=\"layer\" "
            "inkscape:label=\"DisplayConnectors\""
            ">\n");
    ConnRefList::iterator connRefIt = connRefs.begin();
    while (connRefIt != connRefs.end())
    {
        ConnRef *connRef = *connRefIt;
    
        PolyLine route = connRef->displayRoute();
        if (!route.empty())
        {
            fprintf(fp, "<path id=\"disp-%u\" d=\"M %g %g ", connRef->id(),
                    route.ps[0].x, route.ps[0].y);
            for (size_t i = 1; i < route.size(); ++i)
            {
                fprintf(fp, "L %g %g ", route.ps[i].x, route.ps[i].y);
            }
            fprintf(fp, "\" ");
            fprintf(fp, "style=\"fill: none; stroke: black; "
                    "stroke-width: 1px;\" />\n");

            /*
            for (size_t i = 1; i < route.size(); ++i)
            {
                if (route.segmentHasCheckpoint[i - 1])
                {
                    fprintf(fp, "<path d=\"M %g %g ", 
                            route.ps[i - 1].x, route.ps[i - 1].y);
                    fprintf(fp, "L %g %g\" ", route.ps[i].x, route.ps[i].y);
                    fprintf(fp, "style=\"fill: none; stroke: red; "
                            "stroke-width: 1px; stroke-opacity: 1;\" />\n");
                }
            }
            */
        }
        
        ++connRefIt;
    }
    fprintf(fp, "</g>\n");

    if (lineReps)
    {

        for (LineReps::iterator it = lineReps->begin(); it != lineReps->end();
                ++it)
        {
            fprintf(fp, "<path d=\"M %g %g ", 
                    it->begin.x, it->begin.y);
            fprintf(fp, "L %g %g\" ", it->end.x, it->end.y);
            fprintf(fp, "style=\"fill: none; stroke: red; "
                    "stroke-width: 1px; stroke-opacity: 0.7;\" />\n");
        }
    }

    fprintf(fp, "</svg>\n");
    fclose(fp);
}

ConnRerouteFlagDelegate::ConnRerouteFlagDelegate()
{
}

ConnRerouteFlagDelegate::~ConnRerouteFlagDelegate()
{
}

bool *ConnRerouteFlagDelegate::addConn(ConnRef *conn)
{
    m_mapping.push_back(std::make_pair(conn, false));
    return &(m_mapping.back().second);
}

void ConnRerouteFlagDelegate::removeConn(ConnRef *conn)
{
    std::list<std::pair<ConnRef *, bool> >::iterator it;
    for (it = m_mapping.begin(); it != m_mapping.end(); ++it)
    {
        if (it->first == conn)
        {
            it->first = NULL;
        }
    }
}


void ConnRerouteFlagDelegate::alertConns(void)
{
    std::list<std::pair<ConnRef *, bool> >::iterator it;
    for (it = m_mapping.begin(); it != m_mapping.end(); ++it)
    {
        if ((it->first != NULL) && (it->second == true))
        {
            it->second = false;
            it->first->m_needs_reroute_flag = true;
        }
    }
}


}

