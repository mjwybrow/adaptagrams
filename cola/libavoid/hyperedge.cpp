/*
 * vim: ts=4 sw=4 et tw=0 wm=0
 *
 * libavoid - Fast, Incremental, Object-avoiding Line Router
 *
 * Copyright (C) 2011-2014  Monash University
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

#include "libavoid/hyperedge.h"
#include "libavoid/hyperedgetree.h"
#include "libavoid/mtst.h"
#include "libavoid/junction.h"
#include "libavoid/connector.h"
#include "libavoid/vertices.h"
#include "libavoid/connend.h"
#include "libavoid/shape.h"
#include "libavoid/router.h"
#include "libavoid/assertions.h"



namespace Avoid {

HyperedgeRerouter::HyperedgeRerouter()
    : m_router(NULL)
{
}

void HyperedgeRerouter::setRouter(Router *router)
{
    m_router = router;
}

size_t HyperedgeRerouter::registerHyperedgeForRerouting(
        ConnEndList terminals)
{
    m_terminals_vector.push_back(terminals);
    m_root_junction_vector.push_back(NULL);

    return m_terminals_vector.size() - 1;
}

size_t HyperedgeRerouter::registerHyperedgeForRerouting(
        JunctionRef *junction)
{
    m_terminals_vector.push_back(ConnEndList());
    m_root_junction_vector.push_back(junction);

    return m_terminals_vector.size() - 1;
}

size_t HyperedgeRerouter::count(void) const
{
    return m_terminals_vector.size();
}

HyperedgeNewAndDeletedObjectLists HyperedgeRerouter::newAndDeletedObjectLists(
        size_t index) const
{
    COLA_ASSERT(index <= count());

    HyperedgeNewAndDeletedObjectLists result;

    result.newJunctionList = m_new_junctions_vector[index];
    result.deletedJunctionList = m_deleted_junctions_vector[index];
    result.newConnectorList = m_new_connectors_vector[index];
    result.deletedConnectorList = m_deleted_connectors_vector[index];

    return result;
}


void HyperedgeRerouter::outputInstanceToSVG(FILE *fp)
{
    if (count() == 0)
    {
        return;
    }

    fprintf(fp, "    HyperedgeRerouter *hyperedgeRerouter = router->hyperedgeRerouter();\n");
    const size_t num_hyperedges = count();
    unsigned int heEndpointCount = 0;
    for (size_t i = 0; i < num_hyperedges; ++i)
    {
        if (m_root_junction_vector[i])
        {
            fprintf(fp, "    hyperedgeRerouter->registerHyperedgeForRerouting(junctionRef%u);\n",
                    m_root_junction_vector[i]->id());
        }
        else
        {
            fprintf(fp, "    ConnEndList heConnList%u;\n", (unsigned int) i);
            for (ConnEndList::const_iterator it = m_terminals_vector[i].begin();
                    it != m_terminals_vector[i].end(); ++it)
            {
                ++heEndpointCount;
                (*it).outputCode(fp, "heEnd", heEndpointCount);
                fprintf(fp, "    heConnList%u.push_back(heEndPt%u);\n",
                        (unsigned int) i, heEndpointCount);
            }
            fprintf(fp, "    hyperedgeRerouter->registerHyperedgeForRerouting(heConnList%u);\n",
                    (unsigned int) i);

        }
    }
    fprintf(fp, "\n");
}


// Follow connected junctions and connectors from the given connector to
// determine the hyperedge topology, saving objects to the deleted-objects
// vectors as we go.
void HyperedgeRerouter::findAttachedObjects(size_t index,
        ConnRef *connector, JunctionRef *ignore, ConnRefSet& hyperedgeConns)
{
    connector->assignConnectionPinVisibility(true);

    m_deleted_connectors_vector[index].push_back(connector);
    hyperedgeConns.insert(connector);

    std::pair<Obstacle *, Obstacle *> anchors = connector->endpointAnchors();
    JunctionRef *jFirst = dynamic_cast<JunctionRef *> (anchors.first);
    JunctionRef *jSecond = dynamic_cast<JunctionRef *> (anchors.second);

    if (jFirst)
    {
        // If attached to a junction and not one we've explored, then continue.
        if (jFirst != ignore)
        {
            findAttachedObjects(index, jFirst, connector, hyperedgeConns);
        }
    }
    else
    {
        // If its an endpoint, then record the vertex for this endpoint.
        COLA_ASSERT(connector->m_src_vert);
        m_terminal_vertices_vector[index].insert(connector->m_src_vert);
    }

    if (jSecond)
    {
        // If attached to a junction and not one we've explored, then continue.
        if (jSecond != ignore)
        {
            findAttachedObjects(index, jSecond, connector, hyperedgeConns);
        }
    }
    else
    {
        // If its an endpoint, then record the vertex for this endpoint.
        COLA_ASSERT(connector->m_dst_vert);
        m_terminal_vertices_vector[index].insert(connector->m_dst_vert);
    }
}


// Follow connected junctions and connectors from the given junction to
// determine the hyperedge topology, saving objects to the deleted-objects
// vectors as we go.
void HyperedgeRerouter::findAttachedObjects(size_t index,
        JunctionRef *junction, ConnRef *ignore, ConnRefSet& hyperedgeConns)
{
    m_deleted_junctions_vector[index].push_back(junction);

    ConnRefList connectors = junction->attachedConnectors();

    for (ConnRefList::iterator curr  = connectors.begin();
            curr != connectors.end(); ++curr)
    {
        if (*curr == ignore)
        {
            continue;
        }

        COLA_ASSERT(*curr != NULL);
        findAttachedObjects(index, (*curr), junction, hyperedgeConns);
    }
}


// Populate the deleted-object vectors with all the connectors and junctions
// that form the registered hyperedges.  Then return the set of all these
// connectors so they can be ignored for individual rerouting.
ConnRefSet HyperedgeRerouter::calcHyperedgeConnectors(void)
{
    COLA_ASSERT(m_router != NULL);

    ConnRefSet allRegisteredHyperedgeConns;

    // Clear the deleted-object vectors.  We populate them here if necessary.
    m_deleted_junctions_vector.clear();
    m_deleted_junctions_vector.resize(count());
    m_deleted_connectors_vector.clear();
    m_deleted_connectors_vector.resize(count());

    m_terminal_vertices_vector.clear();
    m_terminal_vertices_vector.resize(count());
    m_added_vertices.clear();

    // Populate the deleted-object vectors.
    const size_t num_hyperedges = count();
    for (size_t i = 0; i < num_hyperedges; ++i)
    {
        if (m_root_junction_vector[i])
        {
            // Follow objects attached to junction to find the hyperedge.
            findAttachedObjects(i, m_root_junction_vector[i], NULL,
                    allRegisteredHyperedgeConns);
            continue;
        }

        // Alternatively, we have a set of ConnEnds, so store the
        // corresponding terminals
        std::pair<bool, VertInf *> maybeNewVertex;
        for (ConnEndList::const_iterator it = m_terminals_vector[i].begin();
                it != m_terminals_vector[i].end(); ++it)
        {
            maybeNewVertex = it->getHyperedgeVertex(m_router);
            COLA_ASSERT(maybeNewVertex.second != NULL);
            m_terminal_vertices_vector[i].insert(maybeNewVertex.second);

            if (maybeNewVertex.first)
            {
                // This is a newly created vertex.  Remember it so we can
                // free it and it's visibility edges later.
                m_added_vertices.push_back(maybeNewVertex.second);
            }
        }
    }

    // Return these connectors that don't require rerouting.
    return allRegisteredHyperedgeConns;
}

#ifdef HYPEREDGE_DEBUG
static const double LIMIT = 100000000;

static void reduceRange(double& val)
{
    val = std::min(val, LIMIT);
    val = std::max(val, -LIMIT);
}

#if 0
static bool shapeContainsEndpointVertex(Obstacle *obstacle)
{
    Router *router = obstacle->router();
    ShapeRef *shape = dynamic_cast<ShapeRef *> (obstacle);

    if (!router || !shape)
    {
        return false;
    }

    for (VertInf *k = router->vertices.connsBegin(); 
            k != router->vertices.shapesBegin(); k = k->lstNext)
    {
        if (inPolyGen(shape->polygon(), k->point))
        {
            return true;
        }
    }
    return false;
}
#endif
#endif

void HyperedgeRerouter::performRerouting(void)
{
    COLA_ASSERT(m_router != NULL);

    m_new_junctions_vector.clear();
    m_new_junctions_vector.resize(count());
    m_new_connectors_vector.clear();
    m_new_connectors_vector.resize(count());

#ifdef HYPEREDGE_DEBUG
    double minX = LIMIT;
    double minY = LIMIT;
    double maxX = -LIMIT;
    double maxY = -LIMIT;

    VertInf *curr = m_router->vertices.connsBegin();
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

    FILE *fp = fopen("hyperedge-debug.svg", "w");
    fprintf(fp, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
    // width=\"100%%\" height=\"100%%\" 
    fprintf(fp, "<svg xmlns:inkscape=\"http://www.inkscape.org/namespaces/inkscape\" xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"%g %g %g %g\">\n", minX, minY, maxX - minX, maxY - minY);
    fprintf(fp, "<defs>\n");
    fprintf(fp, "<style type=\"text/css\" ><![CDATA[\n");
    fprintf(fp, ".shape { stroke-width: 1px; stroke: black; fill: blue; stroke-opacity: 0.50; fill-opacity: 0.50; }\n");
    fprintf(fp, ".graph { opacity: 0; fill: none; stroke: red; stroke-width: 1px; }\n");
    fprintf(fp, ".forest { fill: none; stroke: purple; stroke-width: 1px; }\n");
    fprintf(fp, ".hyperedge { fill: none; stroke-width: 1px; }\n");
    fprintf(fp, "]]></style>\n");
    fprintf(fp, "</defs>\n");

    fprintf(fp, "<g inkscape:groupmode=\"layer\" "
            "inkscape:label=\"ShapesRect\">\n");
    ObstacleList::iterator obstacleIt = m_router->m_obstacles.begin();
    while (obstacleIt != m_router->m_obstacles.end())
    {
        Obstacle *obstacle = *obstacleIt;
        bool isShape = (NULL != dynamic_cast<ShapeRef *> (obstacle));

        if ( ! isShape )
        {
            // Don't output obstacles here, for now.
            ++obstacleIt;
            continue;
        }

        Box bb = obstacle->routingBox();

        fprintf(fp, "<rect id=\"rect-%u\" x=\"%g\" y=\"%g\" width=\"%g\" "
                "height=\"%g\" class=\"shape\" />\n",
                obstacle->id(), bb.min.x, bb.min.y, 
                bb.max.x - bb.min.x, bb.max.y - bb.min.y);
        // shapeContainsEndpointVertex(obstacle) ? "style=\"fill: green;\"" : "");
        ++obstacleIt;
    }
    fprintf(fp, "</g>\n");

    fprintf(fp, "<g inkscape:groupmode=\"layer\" "
            "id=\"graph\" style=\"display: none;\" "
            "inkscape:label=\"OrthogVisGraph\">\n");
    EdgeInf *finish = m_router->visOrthogGraph.end();
    for (EdgeInf *t = m_router->visOrthogGraph.begin(); t != finish; t = t->lstNext)
    {
        std::pair<Point, Point> ptpair = t->points();
        Point p1 = ptpair.first;
        Point p2 = ptpair.second;
        
        reduceRange(p1.x);
        reduceRange(p1.y);
        reduceRange(p2.x);
        reduceRange(p2.y);
        
        // std::pair<VertID, VertID> ids = t->ids();
        // (ids.first.isConnPt() || ids.second.isConnPt()) ? "style=\"stroke: green\" " : ""
        fprintf(fp, "<path class=\"graph\" d=\"M %g %g L %g %g\" "
                "%s />\n", p1.x, p1.y, p2.x, p2.y,
                ""
                );
    }
    fprintf(fp, "</g>\n");
#endif
    
    // For each hyperedge...
    const size_t num_hyperedges = count();
    for (size_t i = 0; i < num_hyperedges; ++i)
    {
        // Execute the MTST method to find good junction positions and an
        // initial path.  A hyperedge tree will be built for the new route.
        JunctionHyperedgeTreeNodeMap hyperedgeTreeJunctions;
        MinimumTerminalSpanningTree mtst(m_router, 
                m_terminal_vertices_vector[i], &hyperedgeTreeJunctions);
#ifdef HYPEREDGE_DEBUG
        mtst.setDebuggingOutput(fp, i);
#endif
        // The older MTST construction method (faster, worse results).
        //mtst.constructSequential();
        
        // The preferred MTST construction method.
        // Slightly slower, better quality results.
        mtst.constructInterleaved();

        HyperedgeTreeNode *treeRoot = mtst.rootJunction();
        COLA_ASSERT(treeRoot);
        
        // Fill in connector information and join them to junctions of endpoints
        // of original connectors.
        treeRoot->addConns(NULL, m_router, 
                m_deleted_connectors_vector[i], NULL);

        // Output the list of new junctions and connectors from hyperedge tree.
        treeRoot->listJunctionsAndConnectors(NULL, m_new_junctions_vector[i],
                m_new_connectors_vector[i]);

        // Write paths from the hyperedge tree back into individual
        // connector routes.
        for (size_t pass = 0; pass < 2; ++pass)
        {
            treeRoot->writeEdgesToConns(NULL, pass);
        }

        // Tell the router that we are deleting the objects used for the
        // previous path for the hyperedge.
        for (ConnRefList::iterator curr = 
                m_deleted_connectors_vector[i].begin();
                curr != m_deleted_connectors_vector[i].end(); ++curr)
        {
            // Clear visibility assigned for connection pins.
            (*curr)->assignConnectionPinVisibility(false);

            m_router->deleteConnector(*curr);
        }
        for (JunctionRefList::iterator curr = 
                m_deleted_junctions_vector[i].begin();
                curr != m_deleted_junctions_vector[i].end(); ++curr)
        {
            m_router->deleteJunction(*curr);
        }
    }

    // Clear the input to this class, so that new objects can be registered
    // for rerouting for the next time that transaction that is processed.
    m_terminals_vector.clear();
    m_root_junction_vector.clear();

    // Free temporarily added vertices.
    for (VertexList::iterator curr = m_added_vertices.begin();
            curr != m_added_vertices.end(); ++curr)
    {
        (*curr)->removeFromGraph();
        m_router->vertices.removeVertex(*curr);
        delete *curr;
    }
    m_added_vertices.clear();
#ifdef HYPEREDGE_DEBUG
    fprintf(fp, "</svg>\n");
    fclose(fp);
#endif
}


}
    
