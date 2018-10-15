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

#include <math.h>
#include <cmath>
#include <utility>
#include <memory>
#include <vector>
#include <algorithm>
#include <stdexcept>
#include <iterator>
#include <map>
#include <set>

#include "libvpsc/rectangle.h"
#include "libavoid/geomtypes.h"

#include "libdialect/commontypes.h"
#include "libdialect/ortho.h"
#include "libdialect/graphs.h"
#include "libdialect/planarise.h"
#include "libdialect/util.h"
#include "libdialect/constraints.h"
#include "libdialect/nearalign.h"

using namespace dialect;

using std::pair;
using std::vector;
using std::map;
using std::set;
using std::fabs;

using Avoid::Point;


AlignmentTable::AlignmentTable(Graph &graph, const Nodes &ignore) {
    SepMatrix &matrix = graph.getSepMatrix();
    // Get the lookup of Nodes we care about.
    NodesById care = graph.getNodeLookupWithIgnore(ignore);
    addAlignments(care, matrix);
}

AlignmentTable::AlignmentTable(Graph &graph, const NodesById &ignore) {
    SepMatrix &matrix = graph.getSepMatrix();
    // Get the lookup of Nodes we care about.
    NodesById care = graph.getNodeLookupWithIgnore(ignore);
    addAlignments(care, matrix);
}

void AlignmentTable::addAlignments(const NodesById &nodes, const SepMatrix &matrix) {
    // Add alignments with transitive closure for each pair of distinct nodes.
    for (auto it = nodes.begin(); it != std::prev(nodes.end()); ++it) {
        for (auto jt = std::next(it); jt != nodes.end(); ++jt) {
            id_type i = (*it).first,
                    j = (*jt).first;
            if (matrix.areHAligned(i, j)) addAlignment(i, j, AlignmentFlag::HALIGN);
            else if (matrix.areVAligned(i, j)) addAlignment(i, j, AlignmentFlag::VALIGN);
        }
    }
}

bool AlignmentTable::areAligned(id_type uid, id_type vid, AlignmentFlag flag) {
    return (state[uid][vid] & flag) == flag;
}

vector<id_type> AlignmentTable::getAlignedIds(id_type uid, AlignmentFlag flag) {
    vector<id_type> ids{uid};
    for (auto it = state[uid].begin(); it != state[uid].end(); ++it) {
        auto p = *it;
        if ((p.second & flag) == flag) ids.push_back(p.first);
    }
    return ids;
}

void AlignmentTable::addAlignment(id_type uid, id_type vid, AlignmentFlag flag) {
    // Get the alignment vectors.
    vector<id_type> Au = getAlignedIds(uid, flag),
                    Av = getAlignedIds(vid, flag);
    // Record that everything in Au is aligned with everything in Av.
    for (id_type id1 : Au) {
        for (id_type id2 : Av) {
            state[id1][id2] |= flag;
            state[id2][id1] |= flag;
        }
    }
}

void AlignmentTable::noteInfeasibility(id_type uid, id_type vid, AlignmentFlag flag) {
    if (flag == AlignmentFlag::HALIGN) {
        state[uid][vid] |= AlignmentFlag::HINFEAS;
        state[vid][uid] |= AlignmentFlag::HINFEAS;
    } else if (flag == AlignmentFlag::VALIGN) {
        state[uid][vid] |= AlignmentFlag::VINFEAS;
        state[vid][uid] |= AlignmentFlag::VINFEAS;
    }
}

bool AlignmentTable::isMarkedInfeasible(id_type uid, id_type vid, AlignmentFlag flag) {
    AlignmentFlag infeas = flag == AlignmentFlag::HALIGN ? AlignmentFlag::HINFEAS : AlignmentFlag::VINFEAS;
    return (state[uid][vid] & infeas) == infeas;
}


double dialect::manhattan(Node_SP u, Node_SP v) {
    Point uc = u->getCentre(),
          vc = v->getCentre();
    return fabs(uc.x - vc.x) + fabs(uc.y - vc.y);
}

void PotentialAlignment::remove(void) {
    // If not yet removed...
    if (!removed) {
        // ...remove self from linked list by linking prev and next to each other.
        if (prev != nullptr) prev->next = next;
        if (next != nullptr) next->prev = prev;
        // And now mark self as removed.
        removed = true;
    }
}

void PotentialAlignment::addToGraph(Graph &G) {
    SepMatrix &matrix = G.getSepMatrix();
    id_type uid = u->id(),
            vid = v->id();
    if (flag == AlignmentFlag::HALIGN) matrix.hAlign(uid, vid);
    else matrix.vAlign(uid, vid);
}

SepCo_SP PotentialAlignment::writeSepCo(void) {
    vpsc::Dim dim = flag == AlignmentFlag::HALIGN ? vpsc::YDIM : vpsc::XDIM;
    return std::make_shared<SepCo>(dim, u, v, 0, true);
}

size_t dialect::doNearAlignments(Graph &graph, AlignmentTable &atab, NodesById &ignore, const HolaOpts &opts, bool reattempt) {
    size_t successCount = 0;
    using vpsc::XDIM;
    using vpsc::YDIM;
    // Set up maps.
    map<vpsc::Dim, bool> active{
        {XDIM, true}, {YDIM, true}
    };
    map<vpsc::Dim, AlignmentFlag> d2a{
        {XDIM, AlignmentFlag::VALIGN}, {YDIM, AlignmentFlag::HALIGN}
    };
    // Get nodes of interest.
    NodesById nodes = graph.getNodeLookupWithIgnore(ignore);
    // Compute parameters.
    double iel = graph.getIEL(),
           hkw = iel * opts.nearAlignScalar_kinkWidth / 2.0,
           asc = iel * opts.nearAlignScalar_scope;
    // Prepare cola options.
    ColaOptions colaOpts;
    colaOpts.preventOverlaps = true;
    colaOpts.solidifyAlignedEdges = true;
    // As long as either dimension is "active", keep going.
    vpsc::Dim dim = XDIM;
    while (active[XDIM] || active[YDIM]) {
        dim = vpsc::conjugate(dim);
        AlignmentFlag aflag = d2a[dim];
        active[dim] = false;
        // Build map from node IDs to vector of nodes that are candidates for alignment
        // with that node, in the current dimension.
        map<id_type, Nodes> candidates;
        // Build open and close Events for the nodes.
        Events evts;
        evts.reserve(2*nodes.size());
        for (auto p : nodes) {
            Node_SP u = p.second;
            Point c = u->getCentre();
            double z = dim == XDIM ? c.x - hkw : c.y - hkw,
                   Z = dim == XDIM ? c.x + hkw : c.y + hkw;
            evts.push_back(new Event(z, u, EventType::OPEN));
            evts.push_back(new Event(Z, u, EventType::CLOSE));
        }
        // Sort by coordinate.
        std::sort(evts.begin(), evts.end(), [](const Event *a, const Event *b)->bool{return a->varCoord < b->varCoord;});
        // Maintain a set of "open" Node IDs:
        set<id_type> openIds;
        // Scan through the events.
        for (Event *evt : evts) {
            if (evt->type == EventType::OPEN) {
                Node_SP u = evt->getNode();
                id_type uid = u->id();
                Nodes &uCand = candidates[uid];
                Point uc = u->getCentre();
                for (id_type vid : openIds) {
                    Node_SP v = nodes[vid];
                    Point vc = v->getCentre();
                    bool inScope = dim == XDIM ? fabs(uc.y - vc.y) <= asc : fabs(uc.x - vc.x) <= asc;
                    if (inScope && !atab.areAligned(uid, vid, aflag)) {
                        // The nodes are within scope of one another, and not yet aligned.
                        Nodes &vCand = candidates[vid];
                        // We don't want to record the candidate twice, so just store it once in the
                        // vector for the node with smaller ID.
                        if (uid < vid) {
                            uCand.push_back(v);
                        } else {
                            vCand.push_back(u);
                        }
                    }
                }
                // Finally, note that node u is now open.
                openIds.insert(uid);
            } else if (evt->type == EventType::CLOSE) {
                openIds.erase(evt->getNode()->id());
            }
        }
        // Clean up Events.
        for (Event *evt : evts) delete evt;
        // Now build PotentialAlignments, one for each candidate.
        PotentialAlignments pas;
        for (auto p : candidates) {
            Node_SP u = nodes[p.first];
            for (Node_SP v : p.second) {
                if (reattempt || !atab.isMarkedInfeasible(u->id(), v->id(), aflag)) {
                    pas.push_back(new PotentialAlignment(u, v, aflag));
                }
            }
        }
        // If there aren't any, just move on.
        if (pas.empty()) continue;
        // Otherwise sort by cost.
        std::sort(pas.begin(), pas.end(), [](const PotentialAlignment *a, const PotentialAlignment *b)->bool{return a->cost < b->cost;});
        // Build linked list.
        for (auto it = pas.begin(); it != pas.end(); ++it) {
            PotentialAlignment *p = *it;
            if (it != pas.begin()) {
                PotentialAlignment *o = *std::prev(it);
                p->prev = o;
            }
            auto jt = std::next(it);
            if (jt != pas.end()) {
                PotentialAlignment *q = *jt;
                p->next = q;
            }
        }
        // Build a lookup, from pairs of node IDs to PotentialAlignments.
        SparseIdMatrix2d<PotentialAlignment*>::type paIndex;
        for (PotentialAlignment *pa : pas) {
            id_type uid = pa->u->id(),
                    vid = pa->v->id();
            paIndex[uid][vid] = pa;
            paIndex[vid][uid] = pa;
        }
        // Now we choose the "prime" alignments, so called because the process by which we choose
        // them is much like the Sieve of Eratosthenes.
        PotentialAlignments primePAs;
        PotentialAlignment *first = pas.front();
        while (first != nullptr) {
            // Accept the first PA.
            primePAs.push_back(first);
            // Let u, v be the two nodes involved.
            Node_SP u = first->u,
                    v = first->v;
            id_type uid = u->id(),
                    vid = v->id();
            // Cross off any other PA in which u or v is involved.
            auto uLookup = paIndex[uid];
            for (auto it = uLookup.begin(); it != uLookup.end(); ++it) {
                PotentialAlignment *pa = (*it).second;
                if (pa != nullptr) pa->remove();
            }
            auto vLookup = paIndex[vid];
            for (auto it = vLookup.begin(); it != vLookup.end(); ++it) {
                PotentialAlignment *pa = (*it).second;
                if (pa != nullptr) pa->remove();
            }
            // Furthermore, for any node w already aligned with u, remove any potential
            // alignment of w with v. Likewise for nodes already aligned with v.
            vector<id_type> Au = atab.getAlignedIds(uid, aflag);
            for (id_type wid : Au) {
                PotentialAlignment *pa = paIndex[uid][wid];
                if (pa != nullptr) pa->remove();
            }
            vector<id_type> Av = atab.getAlignedIds(vid, aflag);
            for (id_type wid : Av) {
                PotentialAlignment *pa = paIndex[vid][wid];
                if (pa != nullptr) pa->remove();
            }
            // Advance pointer.
            first = first->next;
        }
        // Attempt to apply the prime alignments.
        for (PotentialAlignment *pa : primePAs) {
            SepCo_SP sc = pa->writeSepCo();
            int result = graph.projectOntoSepCo(colaOpts, sc);
            if (result == 0) {
                // Projection was successful.
                pa->addToTable(atab);
                pa->addToGraph(graph);
                ++successCount;
            } else {
                // Projection failed.
                pa->noteInfeasibility(atab);
            }
        }
        // Clean up PotentialAlignments.
        for (PotentialAlignment *pa : pas) delete pa;
    }
    return successCount;
}
