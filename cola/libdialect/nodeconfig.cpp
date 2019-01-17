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

#include <algorithm>
#include <deque>
#include <memory>
#include <functional>
#include <string>

#include "libavoid/geomtypes.h"

#include "libdialect/commontypes.h"
#include "libdialect/quadaction.h"
#include "libdialect/graphs.h"
#include "libdialect/util.h"
#include "libdialect/logging.h"
#include "libdialect/nodeconfig.h"

using namespace dialect;

using Avoid::Point;

using std::string;

OrthoHubLayout::OrthoHubLayout(Graph_SP G, OrthoHubLayoutOptions opts)
    : m_graph(G),
      m_opts(opts),
      m_cgr(G->updateColaGraphRep()),
      m_aca(G),
      m_edgeLookup(m_cgr.rs.size(), m_cgr.rs.size())
{
    // Make vector of "hubs", i.e. nodes of degree 3 and up, sorted by descending degree.
    // First grab all Nodes, filtering out those of degree less than 3.
    const NodesById &node_lookup = m_graph->getNodeLookup();
    m_hubs.reserve(node_lookup.size());
    size_t h = 0;
    for (auto p : node_lookup) {
        if (opts.includeLinks || p.second->getDegree() > 2) {
            m_hubs.push_back(p.second);
            ++h;
        }
    }
    m_hubs.resize(h);
    // And sort by descending degree.
    std::stable_sort(m_hubs.begin(), m_hubs.end(),
        [](const Node_SP &a, const Node_SP &b) -> bool {return a->getDegree() > b->getDegree();}
    );
    // Make lookup for edge indices by their endpt indices.
    for (size_t j = 0; j < m_cgr.es.size(); ++j) {
        auto e = m_cgr.es[j];
        m_edgeLookup(e.first, e.second) = j;
        m_edgeLookup(e.second, e.first) = j;
    }
    // If we're avoiding flat triangles, then we'll need to build the adjacency matrix for
    // all Nodes in the Graph, by ID.
    if (opts.avoidFlatTriangles) {
        for (auto p : m_graph->getEdgeLookup()) {
            id_type id1 = p.second->getSourceEnd()->id(),
                    id2 = p.second->getTargetEnd()->id();
            m_adjMat[id1][id2] = 1;
            m_adjMat[id2][id1] = 1;
        }
    }
    // When we destress, we want overlap prevention.
    m_colaOpts.preventOverlaps = true;
}

bool OrthoHubLayout::makesFlatTriangle(const Assignment_SP &asgn) {
    // An Assignment makes a flat triangle iff it assigned a pair of connected
    // neighbours to North & South, and/or to East & West.
    for (size_t i = 0; i < 2; ++i) {
        // When i == 0 we get the Nbrs assigned to the East and West semiaxes;
        // when i == 1 we get the Nbrs assigned to the South and North semiaxes.
        Nbr_SP u1 = asgn->semis[i],
               u2 = asgn->semis[i+2];
        if (u1 != nullptr && u2 != nullptr) {
            // Both semiaxes have a Nbr assigned to them.
            // Are the corresponding Nodes connected?
            // We check the adjacency matrix.
            if (m_adjMat[u1->id][u2->id] > 0) return true;
        }
    }
    // We did not find that there would be a flat triangle.
    return false;
}

Assignments OrthoHubLayout::getAssignmentsForNode(const Node_SP &node) const {
    // First construct a Nbr to represent each neighbouring Node of the given one.
    Nodes nbrNodes = node->getNeighbours();
    Nbrs nbrs;
    Point p0 = node->getCentre();
    for (Node_SP u : nbrNodes) {
        Point p1 = u->getCentre();
        double dx = p1.x - p0.x,
               dy = p1.y - p0.y;
        Nbr_SP nbr = std::make_shared<Nbr>(u->id(), dx, dy);
        nbrs.push_back(nbr);
    }
    // Now form the Arrangement based on these Nbrs, and ask it to compute
    // all possible Assignments.
    Arrangement arr(nbrs);
    return arr.computeAllAssignments();
}

void OrthoHubLayout::layout(Logger *logger) {

    // Set up for logging.
    unsigned ln = logger != nullptr ? logger->nextLoggingIndex : 0;
    unsigned lns = 0;
    std::function<void(string)> log = [this, logger](string name)->void{
        if (logger!=nullptr) logger->log(*(this->m_graph), name);
    };

    // We iterate over all hubs.
    auto hub_ptr = m_hubs.begin(),
         hub_end = m_hubs.end();
    // Flag saying whether we can reduce stress and try a node again:
    bool mightNeedToDestress = true;
    // ACASepFlags corresponding to semiaxes:
    ACASepFlag sepFlags[4] = {ACAEAST, ACASOUTH, ACAWEST, ACANORTH};
    while (hub_ptr != hub_end) {
        // Consider next hub.
        Node_SP hub = *hub_ptr;
        // Compute the possible assignments of neighbours to compass directions,
        // in order of descending desirability.
        // First make sure the Node objects have the most up-to-date positions.
        // This is so that the costs of the assignments are correctly evaluated.
        m_graph->updateNodesFromRects();
        Assignments v = getAssignmentsForNode(hub);
        // We need to turn the vector into a deque.
        std::deque<Assignment_SP> asgns;
        asgns.resize(v.size());
        // We may also filter it if we're avoiding flat triangles.
        if (m_opts.avoidFlatTriangles) {
            auto it = std::copy_if(v.cbegin(), v.cend(), asgns.begin(),
                [this](const Assignment_SP &a) -> bool { return !makesFlatTriangle(a); }
            );
            asgns.resize(it - asgns.begin());
        } else {
            std::copy(v.cbegin(), v.cend(), asgns.begin());
        }
        // If there are no viable Assignments, move on to the next hub.
        if (asgns.size() == 0) {
            ++hub_ptr;
            continue;
        }
        // Otherwise, begin attempting the Assignments.
        // Quit either when one works, or when we run out.
        bool success = false;
        OrderedAlignments oas;
        while (!asgns.empty()) {
            // Pop next Assignment.
            Assignment_SP asgn = asgns.front();
            asgns.pop_front();
            // Free allocated objects from previous attempt.
            for (OrderedAlignment *oa : oas) {
                delete oa->separation;
                delete oa->alignment;
                delete oa;
            }
            oas.clear();
            // Consider each semiaxis.
            for (size_t i = 0; i < 4; ++i) {
                // Consider the Nbr assigned to semiaxis i.
                Nbr_SP nbr = asgn->semis[i];
                // If none, continue.
                if (nbr == nullptr) continue;
                // If the corresponding Node is already logically aligned with the hub Node, then we do nothing.
                /* NB: This is not just to save time. In fact if you give VPSC redundant
                 * equality constraints it will mark the second one as unsatisfiable.
                 * This is because once one constraint is active, it will think the
                 * other is still inactive and violated. To satisfy that one it
                 * will try to split the block to which the two variables already
                 * belong, and fail because there are only equality constraints
                 * along the path between them. Since we are calling ACA's
                 * 'allOrNothing' method, this one "unsatisfiable" constraint will
                 * cause the entire node arrangement to fail.)
                 */
                size_t hub_ix = m_cgr.id2ix[hub->id()],
                       nbr_ix = m_cgr.id2ix[nbr->id];
                if (m_aca.nodesAreAligned(hub_ix, nbr_ix)) continue;
                // Otherwise create an OrderedAlignment.
                size_t edge_ix = m_edgeLookup(hub_ix, nbr_ix);
                OrderedAlignment *oa = m_aca.initOrdAlign(hub_ix, nbr_ix, sepFlags[i], edge_ix);
                oas.push_back(oa);
            }
            // Attempt to apply the constraints.
            success = m_aca.applyOAsAllOrNothing(oas);
            if (success) break;
        }
        // Now we have either found an assignment that works, or tried them all and none of them worked.
        if (success) {
            // An assignment was successful. Move on to next hub.
            ++hub_ptr;
            mightNeedToDestress = true;
            if (logger != nullptr) m_graph->updateNodesFromRects();
            log(string_format("%02d_%02d_config_node_%d", ln, lns++, hub->id()));
        } else if (mightNeedToDestress) {
            // If we were not able to configure this hub, it may be that relieving
            // stress in the graph will permit us to configure it. So destress and try this one again.
            // However, set the flag to indicate there will not be another attempt at this.
            m_graph->destress(m_colaOpts);
            mightNeedToDestress = false;
            log(string_format("%02d_%02d_destress", ln, lns++));
        } else {
            // If we have already tried relieving stress once, and we /still/ couldn't
            // apply any assignment, then we give up on this node and move on to the next.
            // Note that we must leave mightNeedToDestress equal to False, because since we have
            // tried destressing once already, there is no reason to destress again until at least
            // one more node has been configured.
            ++hub_ptr;
        }
    }
    // Accept final positions and constraints.
    m_graph->updateNodesFromRects();
    m_aca.updateSepMatrix();
}
