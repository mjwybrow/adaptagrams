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


#include <algorithm>
#include <cstdlib>
#include <math.h>
#include "libavoid/shape.h"
#include "libavoid/router.h"
#include "libavoid/visibility.h"
#include "libavoid/connector.h"
#include "libavoid/debug.h"
#include "libavoid/orthogonal.h"

namespace Avoid {


enum ActionType {
    ShapeMove,
    ShapeAdd,
    ShapeRemove,
    ConnChange
};

class ActionInfo {
    public:
        ActionInfo(ActionType t, ShapeRef *s, const Polygon& p, bool fM)
            : type(t),
              shape(s),
              conn(NULL),
              newPoly(p),
              firstMove(fM)
        {
            assert(type == ShapeMove);
        }
        ActionInfo(ActionType t, ShapeRef *s)
            : type(t),
              shape(s),
              conn(NULL)
        {
            assert(type != ConnChange);
        }
        ActionInfo(ActionType t, ConnRef *c)
            : type(t),
              shape(NULL),
              conn(c)
        {
            assert(type == ConnChange);
        }
        ~ActionInfo()
        {
        }
        bool operator==(const ActionInfo& rhs) const
        {
            return (type == rhs.type) && (shape == rhs.shape);
        }
        ActionType type;
        ShapeRef *shape;
        ConnRef *conn;
        Polygon newPoly;
        bool firstMove;
};


Router::Router(const unsigned int flags)
    : visOrthogGraph(true),
      PartialTime(false),
      SimpleRouting(false),
      ClusteredRouting(true),
      segmt_penalty(0),
      angle_penalty(0),
      crossing_penalty(200),
      cluster_crossing_penalty(4000),
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
#ifdef LIBAVOID_SDL
      avoid_screen(NULL),
#endif
      _largestAssignedId(0),
      _consolidateActions(true),
      // Mode options:
      _polyLineRouting(false),
      _orthogonalRouting(false),
      _staticGraphInvalidated(true)
{
    // At least one of the Routing modes must be set.
    assert(flags & (PolyLineRouting | OrthogonalRouting));

    if (flags & PolyLineRouting)
    {
        _polyLineRouting = true;
    }
    if (flags & OrthogonalRouting)
    {
        _orthogonalRouting = true;
    }
}


Router::~Router()
{
    // Delete remaining connectors.
    ConnRefList::iterator conn = connRefs.begin();
    while (conn != connRefs.end())
    {
        db_printf("Deleting connector %u in ~Router()\n", (*conn)->id());
        delete *conn;
        conn = connRefs.begin();
    }

    // Remove remaining shapes.
    ShapeRefList::iterator shape = shapeRefs.begin();
    while (shape != shapeRefs.end())
    {
        ShapeRef *shapePtr = *shape;
        db_printf("Deleting shape %u in ~Router()\n", shapePtr->id());
        shapePtr->removeFromGraph();
        shapePtr->makeInactive();
        delete shapePtr;
        shape = shapeRefs.begin();
    }

    // Cleanup orphaned orthogonal graph vertices.
    destroyOrthogonalVisGraph();

    assert(connRefs.size() == 0);
    assert(shapeRefs.size() == 0);
    assert(visGraph.size() == 0);
    assert(invisGraph.size() == 0);
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

    if (!_consolidateActions)
    {
        processTransaction();
    }
}


void Router::addShape(ShapeRef *shape)
{
    // There shouldn't be remove events or move events for the same shape
    // already in the action list.
    // XXX: Possibly we could handle this by ordering them intelligently.
    assert(find(actionList.begin(), actionList.end(), 
                ActionInfo(ShapeRemove, shape)) == actionList.end());
    assert(find(actionList.begin(), actionList.end(), 
                ActionInfo(ShapeMove, shape)) == actionList.end());

    ActionInfo addInfo(ShapeAdd, shape);
    
    ActionInfoList::iterator found = 
            find(actionList.begin(), actionList.end(), addInfo);
    if (found == actionList.end())
    {
        actionList.push_back(addInfo);
    }

    if (!_consolidateActions)
    {
        processTransaction();
    }
}


void Router::removeShape(ShapeRef *shape)
{
    // There shouldn't be add events events for the same shape already 
    // in the action list.
    // XXX: Possibly we could handle this by ordering them intelligently.
    assert(find(actionList.begin(), actionList.end(), 
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

    if (!_consolidateActions)
    {
        processTransaction();
    }
}


void Router::moveShape(ShapeRef *shape, const double xDiff, const double yDiff)
{
    Polygon newPoly = shape->polygon();
    newPoly.translate(xDiff, yDiff);

    moveShape(shape, newPoly);
}


void Router::moveShape(ShapeRef *shape, const Polygon& newPoly, 
        const bool first_move)
{
    // There shouldn't be remove events or add events for the same shape
    // already in the action list.
    // XXX: Possibly we could handle this by ordering them intelligently.
    assert(find(actionList.begin(), actionList.end(), 
                ActionInfo(ShapeRemove, shape)) == actionList.end());
    
    if (find(actionList.begin(), actionList.end(), 
                ActionInfo(ShapeAdd, shape)) != actionList.end())
    {
        // The Add is enough, no need for the Move action too.
        return;
    }

    ActionInfo moveInfo(ShapeMove, shape, newPoly, first_move);
    // Sanely cope with the case where the user requests moving the same
    // shape multiple times before rerouting connectors.
    ActionInfoList::iterator found = 
            find(actionList.begin(), actionList.end(), moveInfo);

    if (found != actionList.end())
    {
        if (!SimpleRouting)
        {
            db_printf("warning: multiple moves requested for shape %d "
                    "within a single transaction.\n", (int) shape->id());
        }
        // Just update the ActionInfo with the second polygon, but
        // leave the firstMove setting alone.
        found->newPoly = newPoly;
    }
    else 
    {
        actionList.push_back(moveInfo);
    }

    if (!_consolidateActions)
    {
        processTransaction();
    }
}


void Router::setStaticGraphInvalidated(const bool invalidated)
{
    _staticGraphInvalidated = invalidated;
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
    if (_staticGraphInvalidated)
    {
        if (_orthogonalRouting)
        {
            destroyOrthogonalVisGraph();

            // Regenerate a new visibility graph.
            generateStaticOrthogonalVisGraph(this);
        }
        _staticGraphInvalidated = false;
    }
}


bool Router::shapeInQueuedActionList(ShapeRef *shape) const
{
    bool foundAdd = find(actionList.begin(), actionList.end(), 
                ActionInfo(ShapeAdd, shape)) != actionList.end();
    bool foundRem = find(actionList.begin(), actionList.end(), 
                ActionInfo(ShapeRemove, shape)) != actionList.end();
    bool foundMove = find(actionList.begin(), actionList.end(), 
                ActionInfo(ShapeMove, shape)) != actionList.end();

    return (foundAdd || foundRem || foundMove);
}


bool Router::transactionUse(void) const
{
    return _consolidateActions;
}


void Router::setTransactionUse(const bool transactions)
{
    _consolidateActions = transactions;
}


void Router::processTransaction(void)
{
    bool notPartialTime = !(PartialFeedback && PartialTime);
    bool seenShapeMovesOrDeletes = false;

    // If SimpleRouting, then don't update here.
    if (actionList.empty() || SimpleRouting)
    {
        return;
    }

    ActionInfoList::iterator curr;
    ActionInfoList::iterator finish = actionList.end();
    for (curr = actionList.begin(); curr != finish; ++curr)
    {
        ActionInfo& actInf = *curr;
        if (!((actInf.type == ShapeRemove) || (actInf.type == ShapeMove)))
        {
            // Not a move or remove action, so don't do anything.
            continue;
        }
        seenShapeMovesOrDeletes = true;

        ShapeRef *shape = actInf.shape;
        bool isMove = (actInf.type == ShapeMove);
        bool first_move = actInf.firstMove;

        unsigned int pid = shape->id();

        // o  Remove entries related to this shape's vertices
        shape->removeFromGraph();
        
        if (SelectiveReroute && (!isMove || notPartialTime || first_move))
        {
            markConnectors(shape);
        }

        adjustContainsWithDel(pid);
        
        // Ignore this shape for visibility.
        // XXX: We don't really need to do this if we're not using Partial
        //      Feedback.  Without this the blocked edges still route
        //      around the shape until it leaves the connector.
        shape->makeInactive();
    }
    
    if (seenShapeMovesOrDeletes && _polyLineRouting)
    {
        if (InvisibilityGrph)
        {
            for (curr = actionList.begin(); curr != finish; ++curr)
            {
                ActionInfo& actInf = *curr;
                if (!((actInf.type == ShapeRemove) || 
                            (actInf.type == ShapeMove)))
                {
                    // Not a move or remove action, so don't do anything.
                    continue;
                }

                // o  Check all edges that were blocked by this shape.
                checkAllBlockedEdges(actInf.shape->id());
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
        if (!((actInf.type == ShapeAdd) || (actInf.type == ShapeMove)))
        {
            // Not a move or add action, so don't do anything.
            continue;
        }

        ShapeRef *shape = actInf.shape;
        Polygon& newPoly = actInf.newPoly;
        bool isMove = (actInf.type == ShapeMove);

        unsigned int pid = shape->id();

        // Restore this shape for visibility.
        shape->makeActive();
        
        if (isMove)
        {
            shape->setNewPoly(newPoly);
        }

        adjustContainsWithAdd(newPoly, pid);

        if (_polyLineRouting)
        {
            // o  Check all visibility edges to see if this one shape
            //    blocks them.
            if (!isMove || notPartialTime)
            {
                newBlockingShape(newPoly, pid);
            }

            // o  Calculate visibility for the new vertices.
            if (UseLeesAlgorithm)
            {
                shapeVisSweep(shape);
            }
            else
            {
                shapeVis(shape);
            }
        }
    }
    // Clear the actionList.
    actionList.clear();
    
    _staticGraphInvalidated = true;
    callbackAllInvalidConnectors();
}


void Router::addCluster(ClusterRef *cluster)
{
    cluster->makeActive();
    
    unsigned int pid = cluster->id();
    ReferencingPolygon& poly = cluster->polygon();

    adjustClustersWithAdd(poly, pid);
}


void Router::delCluster(ClusterRef *cluster)
{
    cluster->makeInactive();
    
    unsigned int pid = cluster->id();
    
    adjustClustersWithDel(pid);
}


unsigned int Router::assignId(const unsigned int suggestedId)
{
    // If the suggestedId is zero, then we assign the object the next
    // smallest unassigned ID, otherwise we trust the ID given is unique.
    unsigned int assignedId = (suggestedId == 0) ? 
            (_largestAssignedId + 1) : suggestedId;
    
    // Have the router record if this ID is larger than the _largestAssignedId.
    _largestAssignedId = std::max(_largestAssignedId, assignedId);

    // If assertions are enabled, then we check that this ID really is unique.
    assert(idIsUnique(assignedId));

    return assignedId;
}


    // Returns whether the given ID is unique among all objects known by the
    // router.  Outputs a warning if the ID is found ore than once. 
    // It is expected this is only going to be called from assertions while
    // debugging, so efficiency is not an issue and we just iterate over all
    // objects.
bool Router::idIsUnique(const unsigned int id) const 
{
    unsigned int count = 0;

    // Examine shapes.
    for (ShapeRefList::const_iterator i = shapeRefs.begin(); 
            i != shapeRefs.end(); ++i) 
    {
        if ((*i)->id() == id)
        {
            count++;
        }
    }

    // Examine connectors.
    for (ConnRefList::const_iterator i = connRefs.begin(); 
            i != connRefs.end(); ++i) 
    {
        if ((*i)->id() == id)
        {
            count++;
        }
    }

    // Examine clusters.
    for (ClusterRefList::const_iterator i = clusterRefs.begin(); 
            i != clusterRefs.end(); ++i) 
    {
        if ((*i)->id() == id)
        {
            count++;
        }
    }

    if (count > 1)
    {
        db_printf("Warning:\tlibavoid object ID %d not unique.\n", id);
        return false;
    }
    return true;
}


//----------------------------------------------------------------------------

// XXX: attachedShapes and attachedConns both need to be rewritten
//      for constant time lookup of attached objects once this info
//      is stored better within libavoid.  Also they shouldn't need to
//      be friends of ConnRef.

    // Returns a list of connector Ids of all the connectors of type
    // 'type' attached to the shape with the ID 'shapeId'.
void Router::attachedConns(IntList &conns, const unsigned int shapeId,
        const unsigned int type)
{
    ConnRefList::const_iterator fin = connRefs.end();
    for (ConnRefList::const_iterator i = connRefs.begin(); i != fin; ++i) 
    {
        if ((type & runningTo) && ((*i)->_dstId == shapeId)) 
        {
            conns.push_back((*i)->_id);
        }
        else if ((type & runningFrom) && ((*i)->_srcId == shapeId)) 
        {
            conns.push_back((*i)->_id);
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
        if ((type & runningTo) && ((*i)->_dstId == shapeId)) 
        {
            if ((*i)->_srcId != 0)
            {
                // Only if there is a shape attached to the other end.
                shapes.push_back((*i)->_srcId);
            }
        }
        else if ((type & runningFrom) && ((*i)->_srcId == shapeId)) 
        {
            if ((*i)->_dstId != 0)
            {
                // Only if there is a shape attached to the other end.
                shapes.push_back((*i)->_dstId);
            }
        }
    }
}


    // It's intended this function is called after visibility changes 
    // resulting from shape movement have happened.  It will alert 
    // rerouted connectors (via a callback) that they need to be redrawn.
void Router::callbackAllInvalidConnectors(void)
{
    std::set<ConnRef *> reroutedConns;
    ConnRefList::const_iterator fin = connRefs.end();
    for (ConnRefList::const_iterator i = connRefs.begin(); i != fin; ++i) 
    {
        (*i)->_needs_repaint = false;
        bool rerouted = (*i)->generatePath();
        if (rerouted)
        {
            reroutedConns.insert(*i);
        }
    }

    centreOrthogonalRoutes(this);

    // Alert connectors that they need redrawing.
    for (std::set<ConnRef *>::const_iterator i = reroutedConns.begin(); 
            i != reroutedConns.end(); ++i) 
    {
        (*i)->_needs_repaint = true;
        (*i)->performCallback();
    }
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
            bool ep_in_poly1 = !(eID1.isShape) ? 
                    inPoly(poly, e1, countBorder) : false;
            bool ep_in_poly2 = !(eID2.isShape) ? 
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
    assert(InvisibilityGrph);

    for (EdgeInf *iter = invisGraph.begin(); iter != invisGraph.end() ; )
    {
        EdgeInf *tmp = iter;
        iter = iter->lstNext;

        if (tmp->_blocker == -1)
        {
            tmp->alertConns();
            tmp->checkVis();
        }
        else if (tmp->_blocker == pid)
        {
            tmp->checkVis();
        }
    }
}


void Router::checkAllMissingEdges(void)
{
    assert(!InvisibilityGrph);

    VertInf *first = vertices.connsBegin();

    VertInf *pend = vertices.end();
    for (VertInf *i = first; i != pend; i = i->lstNext)
    {
        VertID iID = i->id;

        // Check remaining, earlier vertices
        for (VertInf *j = first ; j != i; j = j->lstNext)
        {
            VertID jID = j->id;
            if (!(iID.isShape) && (iID.objID != jID.objID))
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
    ShapeRefList::const_iterator finish = shapeRefs.end();
    for (ShapeRefList::const_iterator i = shapeRefs.begin(); i != finish; ++i)
    {
        if (inPoly((*i)->polygon(), pt->point, countBorder))
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

void Router::markConnectors(ShapeRef *shape)
{
    if (RubberBandRouting)
    {
        // When rubber-band routing, we do no reroute connectors that
        // may have a better route, only invalid connectors.
        return;
    }

    assert(SelectiveReroute);

    ConnRefList::const_iterator end = connRefs.end();
    for (ConnRefList::const_iterator it = connRefs.begin(); it != end; ++it)
    {
        ConnRef *conn = (*it);

        if (conn->_route.empty())
        {
            // Ignore uninitialised connectors.
            continue;
        }
        else if (conn->_needs_reroute_flag)
        {
            // Already marked, so skip.
            continue;
        }

        Point start = conn->_route.ps[0];
        Point end = conn->_route.ps[conn->_route.size() - 1];

        double conndist = conn->_route_dist;

        double estdist;
        double e1, e2;

        VertInf *beginV = shape->firstVert();
        VertInf *endV = shape->lastVert()->lstNext;
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

                if (((int) r_p2.y) != 0)
                {
                    db_printf("r_p2.y: %f != 0\n", r_p2.y);
                    abort();
                }
                // This might be slightly off.
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
                conn->_needs_reroute_flag = true;
                break;
            }

        }
    }
}


ConnType Router::defaultConnType(void) const
{
    if (_orthogonalRouting && !_polyLineRouting)
    {
        return ConnType_Orthogonal;
    }
    return ConnType_PolyLine;
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
    int st_invalid_visedges = 0;
    VertInf *finish = vertices.end();
    for (VertInf *t = vertices.connsBegin(); t != finish; t = t->lstNext)
    {
        VertID pID = t->id;

        if ((pID.isShape) && (pID.objID != currshape))
        {
            currshape = pID.objID;
            st_shapes++;
        }
        if (pID.isShape)
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

        if (!(idpair.first.isShape) || !(idpair.second.isShape))
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
    fprintf(fp, "Number of shapes: %d\n", st_shapes);
    fprintf(fp, "Number of vertices: %d (%d real, %d endpoints)\n",
            st_vertices + st_endpoints, st_vertices, st_endpoints);
    fprintf(fp, "Number of vis_edges: %d (%d valid [%d normal, %d endpt], "
            "%d invalid)\n", st_valid_shape_visedges + st_invalid_visedges +
            st_valid_endpt_visedges, st_valid_shape_visedges +
            st_valid_endpt_visedges, st_valid_shape_visedges,
            st_valid_endpt_visedges, st_invalid_visedges);
    fprintf(fp, "----------------------\n");
    fprintf(fp, "checkVisEdge tally: %d\n", st_checked_edges);
    fprintf(fp, "----------------------\n");

    fprintf(fp, "ADDS:  "); timers.Print(tmAdd);
    fprintf(fp, "DELS:  "); timers.Print(tmDel);
    fprintf(fp, "MOVS:  "); timers.Print(tmMov);
    fprintf(fp, "***S:  "); timers.Print(tmSev);
    fprintf(fp, "PTHS:  "); timers.Print(tmPth);
    fprintf(fp, "\n");
}


}

