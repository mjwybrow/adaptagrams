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

#include <set>
#include <vector>
#include <sstream>

#include "libvpsc/assertions.h"

#include "libdialect/ortho.h"
#include "libdialect/graphs.h"
#include "libdialect/faces.h"

using namespace dialect;

const std::map<CompassDir, size_t> Nexus::DIREC_TO_INITIAL_SEARCH_INDEX{
    {CompassDir::NE, 0},
    {CompassDir::EAST, 1},
    {CompassDir::SE, 2},
    {CompassDir::SOUTH, 3},
    {CompassDir::SW, 4},
    {CompassDir::WEST, 5},
    {CompassDir::NW, 6},
    {CompassDir::NORTH, 7}
};

void Nexus::addSide(Side_SP side) {
    // Our tests below will be negative (i.e. that our Node be /not/ the last, or
    // /not/ the first on the Side). So as a check, we first ensure that the Node actually
    // does belong to the Side at all!
    COLA_ASSERT(side->containsNode(m_node->id()));
    // Determine the forward and reverse directions for the Side.
    CardinalDir fwd = side->getForwardDirec(),
                rev = Compass::cardFlip(fwd);
    // If our Node is not the last on the Side, then the Side exits our Node to its forward direction.
    if (m_node != side->lastNode()) writeSlot(fwd, NexusPolarity::EXIT_TO, side);
    // If our Node is not the first on the Side, then the Side enters our Node from its reverse direction.
    if (m_node != side->firstNode()) writeSlot(rev, NexusPolarity::ENTER_FROM, side);
}

std::set<Side_SP> Nexus::getNeighboursOfADirection(CompassDir direc) {
    std::set<Side_SP> nbrs;
    // If the Nexus is empty, so is the set of neighbours.
    if (m_isEmpty) return nbrs;
    // Otherwise, this Nexus holds at least one Side.
    // Get the initial search index.
    size_t i0 = Nexus::DIREC_TO_INITIAL_SEARCH_INDEX.at(direc);
    // To manage search in the two directions, we use a vector of "index differentials".
    // Here, 7 is just -1 mod 8. But it's important that we use a positive number, since
    // we are working with size_t's.
    std::vector<size_t> diffs{1, 7};
    for (size_t di : diffs) {
        // Start at the initial index.
        size_t i = i0;
        while (m_slots[i] == nullptr) {
            i = (i + di) % 8;
            // Since we already checked that this Nexus is not empty, we should not cycle.
            // Just to be sure:
            COLA_ASSERT(i != i0);
        }
        // We have encountered a non-empty slot. We can record its Side as a neighbour.
        nbrs.insert(m_slots[i]);
        // Subtle point: For the anticlockwise search we want to start at the index just
        // prior to the one where we started the clockwise search.
        i0 = (i0 + 7) % 8;
    }
    return nbrs;
}

void Nexus::writeSlot(CardinalDir direc, NexusPolarity polarity, Side_SP side) {
    /*
     * We rely on the actual enum values of the directions and the polarities so that
     * the formula 2*direc + polarity yields:
     *      0       enter from east
     *      1       exit to east
     *      2       enter from south
     *      3       exit to south
     *      4       enter from west
     *      5       exit to west
     *      6       enter from north
     *      7       exit to north
     * This is the order in which the Side SP's will be stored in the eight slots, and this
     * ordering is actually important, when it comes time to compute the neighbours of a given
     * direction.
     */
    size_t d = (size_t) direc,
           p = (size_t) polarity,
           i = 2*d + p;
    m_slots[i] = side;
    // One slot having been written, this Nexus is not empty.
    m_isEmpty = false;
}

std::string Nexus::toString(void) const {
    std::ostringstream ss;
    ss << "Nexus for Node " << m_node->id() << ":" << std::endl;
    if (m_isEmpty) {
        ss << "    empty.";
    } else {
        for (size_t i = 0; i < 8; ++i) {
            Side_SP S = m_slots[i];
            if (S != nullptr) {
                if (i%2 == 0) {
                    ss << "    Entering from the " << Compass::cardToString((CardinalDir)(i/2));
                } else {
                    ss << "    Exiting to the " << Compass::cardToString((CardinalDir)((i-1)/2));
                }
                ss << ": " << S->toString() << std::endl;
            }
        }
    }
    return ss.str();
}
