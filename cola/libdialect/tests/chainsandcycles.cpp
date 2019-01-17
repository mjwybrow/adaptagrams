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
#include <iostream>

#include "libvpsc/assertions.h"

#include "libdialect/commontypes.h"
#include "libdialect/io.h"
#include "libdialect/util.h"
#include "libdialect/graphs.h"

using namespace dialect;

using std::vector;
using std::deque;
using std::cout;
using std::endl;

int main(void) {
    Graph_SP graph = buildGraphFromTglfFile("graphs/" "special/chains_and_cycles.tglf");
    vector<deque<Node_SP>> chains, cycles;
    graph->getChainsAndCycles(chains, cycles);

    // Chains
    cout << "Chains:" << endl;
    for (auto d : chains) {
        for (auto u : d) {
            cout << u->id() << " ";
        }
        cout << endl;
    }

    // Cycles
    cout << "Cycles:" << endl;
    for (auto d : cycles) {
        for (auto u : d) {
            cout << u->id() << " ";
        }
        cout << endl;
    }

    COLA_ASSERT(chains.size() == 8);
    COLA_ASSERT(cycles.size() == 1);

    return 0;
}
