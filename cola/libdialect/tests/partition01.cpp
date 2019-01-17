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

#include "libvpsc/assertions.h"

#include "libdialect/commontypes.h"
#include "libdialect/graphs.h"
#include "libdialect/util.h"

using namespace dialect;

using std::vector;
using std::pair;

int main() {
    // Create a vector of Nodes to be partitioned.
    Nodes nodes;
    vector<pair<double, double>> coords{
        // Two nodes are perfectly vertically aligned at x = 0:
        {0, 0},
        {0, 1},
        // Three nodes are imperfectly vertically aligned at x = 1:
        {1, 0},
        {1.00001, 1},
        {0.99999, 2}
    };
    for (auto p : coords) {
        Node_SP u = Node::allocate();
        u->setCentre(p.first, p.second);
        nodes.push_back(u);
    }
    // Strict partition:
    auto parts1 = partition<Node_SP>(nodes,
        [](const Node_SP &u) -> float {
            return (u->getCentre()).x;
        }
    );
    // Tolerant partition:
    auto parts2 = partition<Node_SP>(nodes,
        [](const Node_SP &u) -> float {
            return (u->getCentre()).x;
        },
        0.01
    );
    COLA_ASSERT(parts1.size() == 4);
    COLA_ASSERT(parts2.size() == 2);
	return 0;
}
