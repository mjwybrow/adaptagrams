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

#include <vector>
#include <iterator>
#include <stdexcept>
#include <string>
#include <sstream>
#include <memory>
#include <algorithm>
#include <utility>
#include <cmath>

#include "libvpsc/assertions.h"
#include "libvpsc/constraint.h"
#include "libvpsc/rectangle.h"
#include "libvpsc/variable.h"
#include "libcola/unused.h"
#include "libcola/compound_constraints.h"
#include "libavoid/geomtypes.h"

#include "libdialect/graphs.h"
#include "libdialect/constraints.h"
#include "libdialect/util.h"
#include "libdialect/ortho.h"

using namespace dialect;

using std::vector;
using std::string;
using std::swap;
using std::signbit;

using Avoid::Point;

SepDir dialect::negateSepDir(SepDir sd) {
    switch(sd) {
    case SepDir::EAST:
        return SepDir::WEST;
    case SepDir::SOUTH:
        return SepDir::NORTH;
    case SepDir::WEST:
        return SepDir::EAST;
    case SepDir::NORTH:
        return SepDir::SOUTH;
    case SepDir::RIGHT:
        return SepDir::LEFT;
    case SepDir::DOWN:
        return SepDir::UP;
    case SepDir::LEFT:
        return SepDir::RIGHT;
    case SepDir::UP:
        return SepDir::DOWN;
    default :
        COLA_ASSERT(false);
    }
}

bool dialect::sepDirIsCardinal(SepDir sd) {
    switch(sd) {
    case SepDir::EAST:
    case SepDir::SOUTH:
    case SepDir::WEST:
    case SepDir::NORTH:
        return true;
    default:
        return false;
    }
}

CardinalDir dialect::sepDirToCardinalDir(SepDir sd) {
    switch(sd) {
    case SepDir::EAST:
        return CardinalDir::EAST;
    case SepDir::SOUTH:
        return CardinalDir::SOUTH;
    case SepDir::WEST:
        return CardinalDir::WEST;
    case SepDir::NORTH:
        return CardinalDir::NORTH;
    default:
        COLA_ASSERT(false);
    }
}

SepDir dialect::cardinalDirToSepDir(CardinalDir dir) {
    switch(dir) {
    case CardinalDir::EAST:
        return SepDir::EAST;
    case CardinalDir::SOUTH:
        return SepDir::SOUTH;
    case CardinalDir::WEST:
        return SepDir::WEST;
    case CardinalDir::NORTH:
        return SepDir::NORTH;
    default:
        COLA_ASSERT(false);
    }
}

SepDir dialect::lateralWeakening(SepDir sd) {
    switch(sd) {
    case SepDir::EAST:
        return SepDir::RIGHT;
    case SepDir::SOUTH:
        return SepDir::DOWN;
    case SepDir::WEST:
        return SepDir::LEFT;
    case SepDir::NORTH:
        return SepDir::UP;
    default:
        return sd;
    }
}

SepDir dialect::cardinalStrengthening(SepDir sd) {
    switch(sd) {
    case SepDir::RIGHT:
        return SepDir::EAST;
    case SepDir::DOWN:
        return SepDir::SOUTH;
    case SepDir::LEFT:
        return SepDir::WEST;
    case SepDir::UP:
        return SepDir::NORTH;
    default:
        return sd;
    }
}

void SepPair::addSep(GapType gt, SepDir sd, SepType st, double gap) {
    // Cannot use this method to remove a constraint.
    // If SepType is NONE, just do nothing.
    if (st == SepType::NONE) return;
    // Otherwise, act based on the SepDir.
    switch(sd) {
    case SepDir::EAST:
        xgt = gt;
        xst = st;
        xgap = gap;
        ygt = GapType::CENTRE;
        yst = SepType::EQ;
        ygap = 0;
        break;
    case SepDir::SOUTH:
        xgt = GapType::CENTRE;
        xst = SepType::EQ;
        xgap = 0;
        ygt = gt;
        yst = st;
        ygap = gap;
        break;
    case SepDir::WEST:
        xgt = gt;
        xst = st;
        xgap = -gap;
        ygt = GapType::CENTRE;
        yst = SepType::EQ;
        ygap = 0;
        break;
    case SepDir::NORTH:
        xgt = GapType::CENTRE;
        xst = SepType::EQ;
        xgap = 0;
        ygt = gt;
        yst = st;
        ygap = -gap;
        break;
    case SepDir::RIGHT:
        xgt = gt;
        xst = st;
        xgap = gap;
        break;
    case SepDir::DOWN:
        ygt = gt;
        yst = st;
        ygap = gap;
        break;
    case SepDir::LEFT:
        xgt = gt;
        xst = st;
        xgap = -gap;
        break;
    case SepDir::UP:
        ygt = gt;
        yst = st;
        ygap = -gap;
        break;
    }
}

void SepPair::transform(SepTransform tf) {
    double g;
    switch(tf) {
        case SepTransform::ROTATE90CW:
            // Swap x- and y-types.
            swap(xst, yst);
            swap(xgt, ygt);
            // xgap <- -ygap
            // ygap <- +xgap
            g = xgap;
            xgap = -ygap;
            ygap = g;
            break;
        case SepTransform::ROTATE90ACW:
            // Swap x- and y-types.
            swap(xst, yst);
            swap(xgt, ygt);
            // xgap <- +ygap
            // ygap <- -xgap
            g = xgap;
            xgap = ygap;
            ygap = -g;
            break;
        case SepTransform::ROTATE180:
            // Negate both gaps.
            xgap = -xgap;
            ygap = -ygap;
            break;
        case SepTransform::FLIPV:
            // Negate xgap only.
            xgap = -xgap;
            break;
        case SepTransform::FLIPH:
            // Negate ygap only.
            ygap = -ygap;
            break;
        case SepTransform::FLIPMD:
            // Swap x- and y-types.
            swap(xst, yst);
            swap(xgt, ygt);
            // xgap <- +ygap
            // ygap <- +xgap
            g = xgap;
            xgap = ygap;
            ygap = g;
            break;
        case SepTransform::FLIPOD:
            // Swap x- and y-types.
            swap(xst, yst);
            swap(xgt, ygt);
            // xgap <- -ygap
            // ygap <- -xgap
            g = xgap;
            xgap = -ygap;
            ygap = -g;
            break;
    }
}

void SepPair::roundGapsUpAbs(void) {
    xgap = signbit(xgap) ? std::floor(xgap) : std::ceil(xgap);
    ygap = signbit(ygap) ? std::floor(ygap) : std::ceil(ygap);
}

bool SepPair::isVerticalCardinal(void) const {
    return xgt == GapType::CENTRE && xst == SepType::EQ && xgap == 0 && yst != SepType::NONE && (ygt == GapType::BDRY || ygap != 0);
}

bool SepPair::isHorizontalCardinal(void) const {
    return ygt == GapType::CENTRE && yst == SepType::EQ && ygap == 0 && xst != SepType::NONE && (xgt == GapType::BDRY || xgap != 0);
}

bool SepPair::isVAlign(void) const {
    return xgt == GapType::CENTRE && xst == SepType::EQ && xgap == 0;
}

bool SepPair::isHAlign(void) const {
    return ygt == GapType::CENTRE && yst == SepType::EQ && ygap == 0;
}

bool SepPair::isCardinal(void) const {
    return isVerticalCardinal() || isHorizontalCardinal();
}

CardinalDir SepPair::getCardinalDir(void) const {
    if (isVerticalCardinal()) {
        if (signbit(ygap)) return CardinalDir::NORTH;
        return CardinalDir::SOUTH;
    } else if (isHorizontalCardinal()) {
        if (signbit(xgap)) return CardinalDir::WEST;
        return CardinalDir::EAST;
    } else {
        throw std::runtime_error("Nodes do not have cardinal separation!");
    }
}

string SepPair::writeTglf(std::map<id_type, unsigned> id2ext, const SepMatrix &m) const {
    // If no constraints, return empty string.
    if (xst == SepType::NONE && yst == SepType::NONE) return "";
    // Prepare strings.
    double xExtraGap = xgt == GapType::BDRY ? m.getExtraBdryGap() : 0.0,
           yExtraGap = ygt == GapType::BDRY ? m.getExtraBdryGap() : 0.0;
    string fmtStr = string_format("%%.%df", tglfPrecision),
           xgapStr = string_format(fmtStr, xgap + xExtraGap),
           ygapStr = string_format(fmtStr, ygap + yExtraGap),
           nxgapStr = string_format(fmtStr, -xgap + xExtraGap),
           nygapStr = string_format(fmtStr, -ygap + yExtraGap),
           xgtStr = xgt == GapType::BDRY ? "B" : "C",
           ygtStr = ygt == GapType::BDRY ? "B" : "C";
    // Prepare indices.
    unsigned src, tgt;
    try { src = id2ext.at(this->src); } catch (std::out_of_range const&) { src = this->src; }
    try { tgt = id2ext.at(this->tgt); } catch (std::out_of_range const&) { tgt = this->tgt; }
    std::ostringstream ss;
    // Vertically aligned
    if (xgt == GapType::CENTRE && xst == SepType::EQ && xgap == 0) {
        switch(yst) {
        case SepType::EQ:
            switch(ygt) {
            case GapType::CENTRE:
                if (ygap < 0) {
                    ss << src << " " << tgt << " C N == " << nygapStr << "\n";
                } else if (ygap > 0) {
                    ss << src << " " << tgt << " C S == " << ygapStr << "\n";
                } else {
                    throw std::runtime_error("Nodes and are constrained to coincide!");
                }
                break;
            case GapType::BDRY:
                if (signbit(ygap)) {
                    ss << src << " " << tgt << " B N == " << nygapStr << "\n";
                } else {
                    ss << src << " " << tgt << " B S == " << ygapStr << "\n";
                }
                break;
            }
            break;
        case SepType::INEQ:
            if (signbit(ygap)) {
                ss << src << " " << tgt << " " << ygtStr << " N >= " << nygapStr << "\n";
            } else {
                ss << src << " " << tgt << " " << ygtStr << " S >= " << ygapStr << "\n";
            }
            break;
        case SepType::NONE:
            ss << src << " " << tgt << " C X == 0\n";
            break;
        }
    }
    // Horizontally aligned
    else if (ygt == GapType::CENTRE && yst == SepType::EQ && ygap == 0) {
        switch(xst) {
        case SepType::EQ:
            switch(xgt) {
            case GapType::CENTRE:
                if (xgap < 0) {
                    ss << src << " " << tgt << " C W == " << nxgapStr << "\n";
                } else if (xgap > 0) {
                    ss << src << " " << tgt << " C E == " << xgapStr << "\n";
                } else {
                    throw std::runtime_error("Nodes and are constrained to coincide!");
                }
                break;
            case GapType::BDRY:
                if (signbit(xgap)) {
                    ss << src << " " << tgt << " B W == " << nxgapStr << "\n";
                } else {
                    ss << src << " " << tgt << " B E == " << xgapStr << "\n";
                }
                break;
            }
            break;
        case SepType::INEQ:
            if (signbit(xgap)) {
                ss << src << " " << tgt << " " << xgtStr << " W >= " << nxgapStr << "\n";
            } else {
                ss << src << " " << tgt << " " << xgtStr << " E >= " << xgapStr << "\n";
            }
            break;
        case SepType::NONE:
            ss << src << " " << tgt << " C Y == 0\n";
            break;
        }
    }
    // Anything else
    else {
        if (xst != SepType::NONE) {
            string reln = xst == SepType::EQ ? "==" : ">=";
            if (signbit(xgap)) {
                ss << src << " " << tgt << " " << xgtStr << " L " << reln << " " << nxgapStr << "\n";
            } else {
                ss << src << " " << tgt << " " << xgtStr << " R " << reln << " " << xgapStr << "\n";
            }
        }
        if (yst != SepType::NONE) {
            string reln = yst == SepType::EQ ? "==" : ">=";
            if (signbit(ygap)) {
                ss << src << " " << tgt << " " << ygtStr << " U " << reln << " " << nygapStr << "\n";
            } else {
                ss << src << " " << tgt << " " << ygtStr << " D " << reln << " " << ygapStr << "\n";
            }
        }
    }
    return ss.str();
}

bool SepPair::hasConstraintInDim(vpsc::Dim dim) const {
    return dim == vpsc::XDIM ? xst != SepType::NONE : yst != SepType::NONE;
}

vpsc::Constraint *SepPair::generateSeparationConstraint(const vpsc::Dim dim, const ColaGraphRep &cgr, SepMatrix *m, vpsc::Variables &vs) {
    size_t left_ix, right_ix;
    double gap;
    bool equality = false;
    if (dim == vpsc::XDIM) {
        // x-dimension
        // If no separation, there is nothing to do.
        if (xst == SepType::NONE) return nullptr;
        // If equality, set boolean accordingly.
        if (xst == SepType::EQ) equality = true;
        // Determine the indices of the left and right variables, and the gap.
        if (signbit(xgap)) {
            gap = -xgap;
            left_ix = cgr.id2ix.at(tgt);
            right_ix = cgr.id2ix.at(src);
        } else {
            gap = xgap;
            left_ix = cgr.id2ix.at(src);
            right_ix = cgr.id2ix.at(tgt);
        }
        // If it's a boundary gap, increase the gap by the average width of the two nodes,
        // plus any global extra boundary gap value.
        if (xgt == GapType::BDRY) gap += (cgr.rs[left_ix]->width()+cgr.rs[right_ix]->width())/2.0 + m->getExtraBdryGap();
    } else {
        // y-dimension
        // If no separation, there is nothing to do.
        if (yst == SepType::NONE) return nullptr;
        // If equality, set boolean accordingly.
        if (yst == SepType::EQ) equality = true;
        // Determine the indices of the left and right variables, and the gap.
        if (signbit(ygap)) {
            gap = -ygap;
            left_ix = cgr.id2ix.at(tgt);
            right_ix = cgr.id2ix.at(src);
        } else {
            gap = ygap;
            left_ix = cgr.id2ix.at(src);
            right_ix = cgr.id2ix.at(tgt);
        }
        // If it's a boundary gap, increase the gap by the average height of the two nodes,
        // plus any global extra boundary gap value.
        if (ygt == GapType::BDRY) gap += (cgr.rs[left_ix]->height()+cgr.rs[right_ix]->height())/2.0 + m->getExtraBdryGap();
    }
    // If we make it this far, there must be a constraint.
    // Allocate the constraint.
    vpsc::Constraint *c = new vpsc::Constraint(vs[left_ix], vs[right_ix], gap, equality);
    c->creator = m;
    return c;
}

void SepMatrix::addSep(id_type id1, id_type id2, GapType gt, SepDir sd, SepType st, double gap) {
    // Access the SepPair for the given Node IDs.
    SepPair_SP &sp = getSepPair(id1, id2);
    // Flip the separation if necessary.
    if (sp->flippedRetrieval) gap = -gap;
    // Add the separation.
    sp->addSep(gt, sd, st, gap);
}

void SepMatrix::addFixedRelativeSep(id_type id1, id_type id2, double dx, double dy) {
    // Access the SepPair for the given Node IDs.
    SepPair_SP &sp = getSepPair(id1, id2);
    // Flip the separations if necessary.
    if (sp->flippedRetrieval) {
        dx = -dx;
        dy = -dy;
    }
    // Add the separations.
    sp->addSep(GapType::CENTRE, SepDir::RIGHT, SepType::EQ, dx);
    sp->addSep(GapType::CENTRE, SepDir::DOWN,  SepType::EQ, dy);
}

void SepMatrix::addFixedRelativeSep(id_type id1, id_type id2) {
    Node_SP u = m_graph->getNode(id1),
            v = m_graph->getNode(id2);
    Point cu = u->getCentre(),
          cv = v->getCentre();
    double dx = cv.x - cu.x,
           dy = cv.y - cu.y;
    addFixedRelativeSep(id1, id2, dx, dy);
}

void SepMatrix::alignByEquatedCoord(id_type id1, id_type id2, vpsc::Dim eqCoord) {
    if (eqCoord == vpsc::XDIM) vAlign(id1, id2);
    else hAlign(id1, id2);
}

void SepMatrix::roundGapsUpward(void) {
    m_extraBdryGap = std::ceil(m_extraBdryGap);
    for (auto p : m_sparseLookup) {
        for (auto q : p.second) {
            q.second->roundGapsUpAbs();
        }
    }
}

CardinalDir SepMatrix::getCardinalDir(id_type id1, id_type id2) const {
    SepPair_SP sp = checkSepPair(id1, id2);
    if (sp == nullptr) throw std::runtime_error("No constraint.");
    CardinalDir d = sp->getCardinalDir();
    // Flip if necessary.
    if (sp->flippedRetrieval) d = Compass::cardFlip(d);
    return d;
}

void SepMatrix::getAlignedSets(std::map<id_type, std::set<id_type> > &hSets, std::map<id_type, std::set<id_type> > &vSets) const {
    // We build two auxiliary Graphs: one where edges represent H-alignments, and one for V-alignments.
    // Then the aligned sets are the connected components of these auxiliary Graphs.
    Graph H, V;
    for (auto p : m_graph->getNodeLookup()) {
        Node_SP u = p.second;
        GhostNode_SP g1 = u->makeGhost(),
                     g2 = u->makeGhost();
        g1->setMasquerade(true);
        g2->setMasquerade(true);
        H.addNode(g1, false);
        V.addNode(g2, false);
    }
    for (auto p : m_sparseLookup) {
        for (auto q : p.second) {
            SepPair_SP sp = q.second;
            if (sp->isHAlign()) H.addEdge(sp->src, sp->tgt);
            if (sp->isVAlign()) V.addEdge(sp->src, sp->tgt);
        }
    }
    Graphs hComps = H.getConnComps(),
           vComps = V.getConnComps();
    for (Graph_SP C : hComps) {
        NodesById nodes = C->getNodeLookup();
        std::set<id_type> ids;
        for (auto p : nodes) ids.insert(p.first);
        for (id_type id : ids) hSets.insert({id, ids});
    }
    for (Graph_SP C : vComps) {
        NodesById nodes = C->getNodeLookup();
        std::set<id_type> ids;
        for (auto p : nodes) ids.insert(p.first);
        for (id_type id : ids) vSets.insert({id, ids});
    }
}

bool SepMatrix::areHAligned(id_type id1, id_type id2) const {
    SepPair_SP sp = checkSepPair(id1, id2);
    if (sp == nullptr) return false;
    // No need to flip.
    return sp->isHAlign();
}

bool SepMatrix::areVAligned(id_type id1, id_type id2) const {
    SepPair_SP sp = checkSepPair(id1, id2);
    if (sp == nullptr) return false;
    // No need to flip.
    return sp->isVAlign();
}

void SepMatrix::transform(SepTransform tf) {
    // Apply the transformation to every SepPair.
    for (auto p : m_sparseLookup) {
        for (auto q : p.second) {
            q.second->transform(tf);
        }
    }
}

void SepMatrix::transformClosedSubset(SepTransform tf, const std::set<id_type> &ids) {
    // We take advantage of the fact that both the std::set giving the desired IDs,
    // and the std::map with which our constraint lookup is implemented, are ordered.
    // This makes our algorithm more efficient.
    auto map_ptr1 = m_sparseLookup.begin();
    auto map_end1 = m_sparseLookup.end();
    auto set_ptr1 = ids.cbegin();
    auto set_end = ids.cend();
    while (map_ptr1 != map_end1 && set_ptr1 != set_end) {
        id_type i = (*map_ptr1).first;
        id_type a = *set_ptr1;
        if (i > a) {
            // The element in the set is less than that in the map.
            // Advance the set pointer, but do not advance the map pointer.
            ++set_ptr1;
        } else {
            if (i == a) {
                // ID i belongs to the set.
                auto i_lookup = (*map_ptr1).second;
                auto map_ptr2 = i_lookup.begin();
                auto map_end2 = i_lookup.end();
                // Since second ID must be greater than the first, we can start looking
                // through the set at set_ptr1 + 1.
                // set iterators do not support "+ 1", but you can instead use
                // std::next (thanks to https://stackoverflow.com/a/19946497)
                auto set_ptr2 = std::next(set_ptr1);
                while (map_ptr2 != map_end2 && set_ptr2 != set_end) {
                    id_type j = (*map_ptr2).first;
                    id_type b = *set_ptr2;
                    if (j > b) {
                        // The element in the set is less than that in the map.
                        // Advance the set pointer, but do not advance the map pointer.
                        ++set_ptr2;
                    } else {
                        if (j == b) {
                            // ID j belongs to the set.
                            // Both IDs are in the set, so we apply the transformation.
                            SepPair_SP &sp = (*map_ptr2).second;
                            sp->transform(tf);
                        }
                        // Whether or not we did anything, advance the map pointer.
                        ++map_ptr2;
                    }
                }
            }
            // Whether or not we did anything, advance the map pointer.
            ++map_ptr1;
        }
    }
}

void SepMatrix::transformOpenSubset(SepTransform tf, const std::set<id_type> &ids) {
    // First pass: We partition the domain of the outer map into elements in the given
    // set, and elements not in the given set.
    // Elements in the set can actually be processed inline; elements outside the set
    // must wait until later.
    vector<id_type> out_of_set;
    auto map_ptr1 = m_sparseLookup.begin();
    auto map_end1 = m_sparseLookup.end();
    auto set_ptr1 = ids.cbegin();
    auto set_end = ids.cend();
    // First pass, Part (a): before we run off the end of either the map or the set:
    while (map_ptr1 != map_end1 && set_ptr1 != set_end) {
        id_type i = (*map_ptr1).first;
        id_type a = *set_ptr1;
        if (i > a) {
            // The element in the set is less than that in the map.
            // We cannot decide set membership.
            // Advance the set pointer, but do not advance the map pointer.
            ++set_ptr1;
        } else {
            // The element in the set is >= that in the map.
            // We can decide set membership.
            if (i == a) {
                // ID i belongs to the set. Transform every pair (i, j).
                auto i_lookup = (*map_ptr1).second;
                for (auto p : i_lookup) {
                    SepPair_SP &sp = p.second;
                    sp->transform(tf);
                }
            } else {
                // ID i does not belong to the set.
                out_of_set.push_back(i);
            }
            // In both cases, advance the map pointer.
            ++map_ptr1;
        }
    }
    // First pass, Part (b): if we exited part (a) because the set ended first, then all remaining
    // elements in the domain of the map are not in the set.
    while (map_ptr1 != map_end1) {
        id_type i = (*map_ptr1).first;
        out_of_set.push_back(i);
        ++map_ptr1;
    }
    // Second pass: For elements i not in the set, transform only those pairs (i, j) for
    // which j is in the set.
    for (id_type i : out_of_set) {
        auto i_lookup = m_sparseLookup[i];
        auto map_ptr2 = i_lookup.begin();
        auto map_end2 = i_lookup.end();
        auto set_ptr2 = ids.cbegin();
        while (map_ptr2 != map_end2 && set_ptr2 != set_end) {
            id_type j = (*map_ptr2).first;
            id_type b = *set_ptr2;
            if (j > b) {
                // The element in the set is less than that in the map.
                // Advance the set pointer, but do not advance the map pointer.
                ++set_ptr2;
            } else {
                if (j == b) {
                    // ID j belongs to the set, so we apply the transformation.
                    SepPair_SP &sp = (*map_ptr2).second;
                    sp->transform(tf);
                }
                // Whether or not we did anything, advance the map pointer.
                ++map_ptr2;
            }
        }
    }
}

void SepMatrix::removeNode(id_type id) {
    // We replace our sparse lookup map with a new one.
    SparseIdMatrix2d<SepPair_SP>::type m;
    // Iterate over the old lookup.
    for (auto p : m_sparseLookup) {
        // If first ID matches the given one, skip it entirely.
        if (p.first == id) continue;
        // Else initialise a map for this ID.
        std::map<id_type, SepPair_SP> &mi = m[p.first];
        // Iterate over the nested map.
        for (auto q : p.second) {
            // If second ID matches the given one, skip it.
            if (q.first == id) continue;
            // Else insert a copy of the pair in the new lookup.
            mi.insert(q);
        }
    }
    // Replace the old lookup.
    // Since we use shared ptrs to our SepPairs, those among the old ones that
    // lack shared ptrs in the new lookup will be destroyed automatically.
    m_sparseLookup = m;
}

void SepMatrix::removeNodes(const NodesById &nodes) {
    // We replace our sparse lookup map with a new one.
    SparseIdMatrix2d<SepPair_SP>::type m;
    // Iterate over the old lookup and the given "set" of nodes.
    // (We treat it like a set; really we are using the first component of
    //  the pairs contained in this map.)
    auto map_ptr1 = m_sparseLookup.begin();
    auto map_end1 = m_sparseLookup.end();
    auto set_ptr1 = nodes.cbegin();
    auto set_end = nodes.cend();
    // Special case: If `nodes` is empty, then don't waste time copying the map.
    if (set_ptr1 == set_end) return;
    while (map_ptr1 != map_end1 && set_ptr1 != set_end) {
        id_type i = (*map_ptr1).first;
        id_type a = (*set_ptr1).first;
        if (i > a) {
            // The element in the set is less than that in the map.
            // Advance the set pointer, but do not advance the map pointer.
            ++set_ptr1;
        } else {
            // In all other cases we will advance the map pointer.
            // However first we must consider the case in which i < a.
            if (i < a) {
                // ID i does not belong to the set.
                // Initialise a map for this ID.
                std::map<id_type, SepPair_SP> &mi = m[i];
                // Iterate over the nested map.
                auto i_lookup = (*map_ptr1).second;
                auto map_ptr2 = i_lookup.begin();
                auto map_end2 = i_lookup.end();
                auto set_ptr2 = set_ptr1;
                while (map_ptr2 != map_end2 && set_ptr2 != set_end) {
                    id_type j = (*map_ptr2).first;
                    id_type b = (*set_ptr2).first;
                    if (j > b) {
                        // The element in the set is less than that in the map.
                        // Advance the set pointer, but do not advance the map pointer.
                        ++set_ptr2;
                    } else {
                        if (j < b) {
                            // ID j does not belong to the set.
                            // Both IDs are outside the set, so we can keep this datum.
                            mi.insert(*map_ptr2);
                        }
                        // Whether or not we did anything, advance the map pointer.
                        ++map_ptr2;
                    }
                }
                // If the set ended first, anything else in the map stays.
                while (map_ptr2 != map_end2) {
                    mi.insert(*map_ptr2);
                    ++map_ptr2;
                }
            }
            // Whether or not we did anything, advance the map pointer.
            ++map_ptr1;
        }
    }
    // If the set ended first, anything else in the map stays.
    while (map_ptr1 != map_end1) {
        m.insert(*map_ptr1);
        ++map_ptr1;
    }
    // Replace the old lookup.
    m_sparseLookup = m;
}

void SepMatrix::setSepPair(id_type id1, id_type id2, SepPair_SP sp) {
    if (id1 >= id2) throw std::runtime_error("Bad ids for SepPair.");
    getSepPair(id1, id2) = sp;
}

void SepMatrix::setCorrespondingConstraints(SepMatrix &matrix) const {
    // Get the lookup of nodes belonging to the other matrix's graph.
    NodesById otherNodes = matrix.m_graph->getNodeLookup();
    // Although this is a map, we will use it as a set, i.e. to test presence of
    // Node IDs in the other graph.
    // Accordingly we refer to "set" pointers below.
    auto map_ptr1 = m_sparseLookup.cbegin();
    auto map_end1 = m_sparseLookup.cend();
    auto set_ptr1 = otherNodes.cbegin();
    auto set_end = otherNodes.cend();
    while (map_ptr1 != map_end1 && set_ptr1 != set_end) {
        id_type i = (*map_ptr1).first;
        id_type a = (*set_ptr1).first;
        if (i > a) {
            // The element in the set is less than that in the map.
            // Advance the set pointer, but do not advance the map pointer.
            ++set_ptr1;
        } else {
            if (i == a) {
                // ID i belongs to the other graph.
                auto i_lookup = (*map_ptr1).second;
                auto map_ptr2 = i_lookup.begin();
                auto map_end2 = i_lookup.end();
                // Since second ID must be greater than the first, we can start looking
                // through the set at set_ptr1 + 1.
                auto set_ptr2 = std::next(set_ptr1);
                while (map_ptr2 != map_end2 && set_ptr2 != set_end) {
                    id_type j = (*map_ptr2).first;
                    id_type b = (*set_ptr2).first;
                    if (j > b) {
                        // The element in the set is less than that in the map.
                        // Advance the set pointer, but do not advance the map pointer.
                        ++set_ptr2;
                    } else {
                        if (j == b) {
                            // ID j belongs to the other graph.
                            // Since both IDs belong to the other graph, we can set this constraint.
                            SepPair_SP sp = (*map_ptr2).second;
                            matrix.setSepPair(i, j, sp);
                        }
                        // Whether or not we did anything, advance the map pointer.
                        ++map_ptr2;
                    }
                }
            }
            // Whether or not we did anything, advance the map pointer.
            ++map_ptr1;
        }
    }
}

string SepMatrix::writeTglf(std::map<id_type, unsigned> id2ext) const {
    std::ostringstream ss;
    for (auto p : m_sparseLookup) {
        for (auto q : p.second) {
            SepPair_SP &sp = q.second;
            string s = sp->writeTglf(id2ext, *this);
            if (!s.empty()) ss << s;
        }
    }
    return ss.str();
}

SepPair_SP &SepMatrix::getSepPair(id_type id1, id_type id2) {
    if (id1 == id2) {
        throw std::runtime_error("Cannot set a constraint between a node and itself.");
    } else if (id1 < id2) {
        SepPair_SP &sp = m_sparseLookup[id1][id2];
        if (sp == nullptr) {
            sp = std::make_shared<SepPair>();
            sp->src = id1;
            sp->tgt = id2;
            sp->flippedRetrieval = false;
        }
        return sp;
    } else { // id2 < id1
        SepPair_SP &sp = m_sparseLookup[id2][id1];
        if (sp == nullptr) {
            sp = std::make_shared<SepPair>();
            sp->src = id2;
            sp->tgt = id1;
            sp->flippedRetrieval = true;
        }
        return sp;
    }
}

SepPair_SP SepMatrix::checkSepPair(id_type id1, id_type id2) const {
    if (id1 == id2) {
        return nullptr;
    } else {
        bool flipped = id2 < id1;
        id_type idSmall = flipped ? id2 : id1,
                idLarge = flipped ? id1 : id2;
        auto it = m_sparseLookup.find(idSmall);
        if (it == m_sparseLookup.end()) {
            return nullptr;
        } else {
            auto m = (*it).second;
            auto jt = m.find(idLarge);
            if (jt == m.end()) {
                return nullptr;
            } else {
                SepPair_SP sp = (*jt).second;
                sp->flippedRetrieval = flipped;
                return sp;
            }
        }
    }
}

void SepMatrix::free(id_type id1, id_type id2) {
    if (id1 == id2) {
        // There is nothing to do.
        return;
    } else if (id2 < id1) {
        // Swap so id1 is smaller.
        std::swap(id1, id2);
    }
    // Under id1, erase the entry for id2 (if any).
    // Since SepPairs are stored via shared pointers, we need not worry
    // about whether to delete the SepPair.
    // This is cruicially important in cases where a copy of a Graph has
    // been made, and one copy wants to free a constraint /without/ depriving
    // the other Graph of that constraint. This is a good example of a true shared resource.
    m_sparseLookup[id1].erase(id2);
}

// ------------------------------------------------------------------
// cola::CompoundConstraint interface

void SepMatrix::generateVariables(const vpsc::Dim dim, vpsc::Variables &vars) {
    COLA_UNUSED(dim);
    COLA_UNUSED(vars);
}

void SepMatrix::generateSeparationConstraints(const vpsc::Dim dim,
        vpsc::Variables &vs, vpsc::Constraints &cs, vpsc::Rectangles &bbs) {
    COLA_UNUSED(bbs);
    ColaGraphRep &cgr = m_graph->getColaGraphRep();
    for (auto p : m_sparseLookup) {
        for (auto q : p.second) {
            SepPair_SP &sp = q.second;
            vpsc::Constraint *c = sp->generateSeparationConstraint(dim, cgr, this, vs);
            if (c != nullptr) cs.push_back(c);
        }
    }
}

string SepMatrix::toString(void) const {
    std::ostringstream ss;
    ss << "Separation Matrix\n";
    return ss.str();
}

void SepMatrix::markAllSubConstraintsAsInactive(void) {
    // We take advantage of this opportunity to refresh our list of subconstraint infos, since
    // new SepPairs may have been added to the SepMatrix.
    // First clean up.
    for (cola::SubConstraintInfo *info : _subConstraintInfo) delete info;
    _subConstraintInfo.clear();
    // Now populate with up to two infos for each of our SepPairs, one in each dimension.
    for (auto p : m_sparseLookup) {
        for (auto q : p.second) {
            SepPair_SP &sp = q.second;
            for (unsigned d = 0; d < 2; ++d) {
                vpsc::Dim dim = (vpsc::Dim)d;
                if (sp->hasConstraintInDim(dim)) {
                    _subConstraintInfo.push_back(new SepPairSubConstraintInfo(sp, dim));
                }
            }
        }
    }
    // And set the index to zero.
    _currSubConstraintIndex = 0;
}

cola::SubConstraintAlternatives SepMatrix::getCurrSubConstraintAlternatives(vpsc::Variables vs[]) {
    cola::SubConstraintAlternatives alternatives;
    SepPairSubConstraintInfo *info = dynamic_cast<SepPairSubConstraintInfo*>(_subConstraintInfo[_currSubConstraintIndex]);
    SepPair_SP sp = info->sp;
    vpsc::Dim dim = info->dim;
    ColaGraphRep &cgr = m_graph->getColaGraphRep();
    vpsc::Constraint *c = sp->generateSeparationConstraint(dim, cgr, this, vs[dim]);
    alternatives.push_back(cola::SubConstraint(dim, *c));
    // The subconstraint stores its own copy of the vpsc::Constraint, so we can and must delete the original now.
    delete c;
    return alternatives;
}

// ------------------------------------------------------------------

void SepCo::generateColaConstraints(const ColaGraphRep &cgr, cola::CompoundConstraints &ccs) {
    unsigned l = cgr.id2ix.at(left->id()),
             r = cgr.id2ix.at(right->id());
    if (gap >= 0) {
        ccs.push_back(new cola::SeparationConstraint(dim, l, r, gap, exact));
    } else {
        // A separation constraint with negative gap is realised using a combination of
        // alignments and separations.
        cola::AlignmentConstraint *alnL = new cola::AlignmentConstraint(dim, cgr.rs[l]->getCentreD(dim));
        alnL->addShape(l, 0);
        cola::AlignmentConstraint *alnR = new cola::AlignmentConstraint(dim, cgr.rs[r]->getCentreD(dim) - gap);
        alnR->addShape(r, gap);
        cola::SeparationConstraint *sep = new cola::SeparationConstraint(dim, alnL, alnR, 0, exact);
        ccs.push_back(alnL);
        ccs.push_back(alnR);
        ccs.push_back(sep);
    }
}

CardinalDir SepCo::getDirecRelativeToNode(Node_SP baseNode) const {
    if (baseNode == left) {
        return dim == vpsc::HORIZONTAL ? CardinalDir::EAST : CardinalDir::SOUTH;
    } else {
        return dim == vpsc::HORIZONTAL ? CardinalDir::WEST : CardinalDir::NORTH;
    }
}

void SepCo::addToMatrix(SepMatrix &matrix) const {
    SepDir sd = dim == vpsc::HORIZONTAL ? SepDir::RIGHT : SepDir::DOWN;
    SepType st = exact ? SepType::EQ : SepType::INEQ;
    matrix.addSep(left->id(), right->id(), GapType::CENTRE, sd, st, gap);
}

double SepCo::violation(void) const {
    Point lc = left->getCentre(),
          rc = right->getCentre();
    double vio = gap - (dim==vpsc::HORIZONTAL ? rc.x - lc.x : rc.y - lc.y);
    if (vio < 0) vio = exact ? -vio : 0;
    return vio;
}

cola::CompoundConstraints Projection::generateColaConstraints(const ColaGraphRep &cgr) {
    cola::CompoundConstraints ccs;
    for (SepCo_SP s : sepCoSet) {
        s->generateColaConstraints(cgr, ccs);
    }
    return ccs;
}

void ProjSeq::addProjection(SepCoSet sepCos, vpsc::Dim dim) {
    // Ensure monotonicity:
    SepCoSet &finalSet = m_finalSets[dim];
    finalSet.insert(sepCos.begin(), sepCos.end());
    // Record new Projection.
    Projection_SP proj = std::make_shared<Projection>(finalSet, dim);
    m_projections.push_back(proj);
}

Projection_SP ProjSeq::nextProjection(void) {
    Projection_SP next = nullptr;
    if (m_ptr < m_projections.size()) {
        next = m_projections[m_ptr++];
    }
    return next;
}

ProjSeq &ProjSeq::operator+=(const ProjSeq &rhs) {
    for (Projection_SP proj : rhs.m_projections) {
        addProjection(proj->sepCoSet, proj->dim);
    }
    return *this;
}

ProjSeq operator+(const ProjSeq &lhs, const ProjSeq &rhs) {
    ProjSeq sum = lhs;
    sum += rhs;
    return sum;
}

SepCoSet ProjSeq::getAllConstraints(void) const {
    SepCoSet all = m_finalSets.at(vpsc::XDIM);
    const SepCoSet &ySet = m_finalSets.at(vpsc::YDIM);
    all.insert(ySet.cbegin(), ySet.cend());
    return all;
}

double ProjSeq::violation(void) const {
    SepCoSet all = getAllConstraints();
    double v = 0;
    for (SepCo_SP sc : all) v += sc->violation();
    return v;
}

string SepCo::toString(void) const {
    std::ostringstream ss;
    ss << "SepCo: ";
    ss << (dim == vpsc::XDIM ? "x" : "y");
    ss << ", u" << left->id();
    if (gap != 0) {
        ss << " + " << gap;
    }
    ss << (exact ? " == " : " <= ");
    ss << "u" << right->id();
    return ss.str();
}

string Projection::toString(void) const {
    std::ostringstream ss;
    ss << "Projection: ";
    ss << (dim == vpsc::XDIM ? "x" : "y") << std::endl;
    for (SepCo_SP sc : sepCoSet) {
        ss << "  " << sc->toString() << std::endl;
    }
    return ss.str();
}

string ProjSeq::toString(void) const {
    std::ostringstream ss;
    ss << "ProjSeq:" << std::endl;
    for (Projection_SP p : m_projections) {
        ss << p->toString() << std::endl;
    }
    return ss.str();
}
