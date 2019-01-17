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
#include <utility>

#include "libavoid/geomtypes.h"
#include "libvpsc/assertions.h"

#include "libdialect/commontypes.h"
#include "libdialect/graphs.h"
#include "libdialect/util.h"

using namespace dialect;

using std::vector;
using std::pair;

using Avoid::Point;

int main(void) {

    // Create some Nodes.
    vector<pair<double, double>> centres {
        {0, 0}, {1.1, 1.1}, {2.5, 2.5}
    };
    Nodes nodes;
    for (auto p : centres) {
        Node_SP u = Node::allocate();
        u->setCentre(p.first, p.second);
        nodes.push_back(u);
    }

    // Try out a NearbyObjectFinder, with threshold 0.5.
    NearbyObjectFinder<Node_SP> nof(0.5);

    // Add the Node at (0, 0).
    nof.addObject(0, 0, nodes[0]);
    // We should not find a Node near (1, 1).
    COLA_ASSERT(nof.findObject(1, 1) == nullptr);

    // Now add the Node at (1.1, 1.1).
    nof.addObject(1.1, 1.1, nodes[1]);
    // This time we should find this Node near (1, 1).
    COLA_ASSERT(nof.findObject(1, 1)->getCentre() == Point(1.1, 1.1));

    // Add the Node at (2.5, 2.5).
    nof.addObject(2.5, 2.5, nodes[2]);
    // Should not find this Node near (2, 2) or (3, 3), since we use open neighbourhoods.
    COLA_ASSERT(nof.findObject(2, 2) == nullptr);
    COLA_ASSERT(nof.findObject(3, 3) == nullptr);
    // But should find it at (2.000001, etc.
    COLA_ASSERT(nof.findObject(2.000001, 2.000001)->getCentre() == Point(2.5, 2.5));
    COLA_ASSERT(nof.findObject(2.999999, 2.999999)->getCentre() == Point(2.5, 2.5));
}
