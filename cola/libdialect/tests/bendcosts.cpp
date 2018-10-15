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
#include <deque>
#include <sstream>

#include "libvpsc/assertions.h"

#include "libdialect/commontypes.h"
#include "libdialect/io.h"
#include "libdialect/util.h"
#include "libdialect/graphs.h"
#include "libdialect/chains.h"

using namespace dialect;

using std::vector;
using std::deque;
using std::cout;
using std::endl;

int main(void) {
    Graph_SP graph = buildGraphFromTglfFile("graphs/" "special/bend_costs.tglf");
    writeStringToFile(graph->writeTglf(), "output/" "bendcosts_" "layout.tglf");

    vector<deque<Node_SP>> chains, cycles;
    graph->getChainsAndCycles(chains, cycles);
    COLA_ASSERT(chains.size() == 1);
    COLA_ASSERT(cycles.size() == 1);

    cout << "Cycle bend costs for TLC:" << endl;
    Chain theCycle(graph, cycles[0], true);
    double c[16] = {1, 2, 3, 4, 5, 6, 7, 8, 7, 6, 5, 4, 3, 2, 1, 0};
    for (size_t i = 0; i < 16; ++i) {
        double cost = theCycle.bendCost(LinkShape::TLC, i);
        cout << i << ": " << cost << endl;
        COLA_ASSERT(approx_equal(cost, c[i]/8.0, 0.001));
    }

    cout << "Chain bend costs for TLC:" << endl;
    Chain theChain(graph, chains[0]);
    double c2[7] = {0, 0, 1, 2, 3, 4, 4};
    for (size_t i = 0; i < 7; ++i) {
        double cost = theChain.bendCost(LinkShape::TLC, i);
        cout << i << ": " << cost << endl;
        COLA_ASSERT(approx_equal(cost, c2[i]/4.0, 0.001));
    }

    return 0;
}
