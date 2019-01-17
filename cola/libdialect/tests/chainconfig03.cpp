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
#include <memory>

#include "libvpsc/assertions.h"
#include "libvpsc/rectangle.h"

#include "libdialect/commontypes.h"
#include "libdialect/io.h"
#include "libdialect/util.h"
#include "libdialect/graphs.h"
#include "libdialect/ortho.h"
#include "libdialect/constraints.h"
#include "libdialect/chains.h"

using namespace dialect;

using std::vector;
using std::deque;
using std::cout;
using std::endl;

// This time our goal is to test the ability of AestheticBends to set bend nodes
// in their associated Edges.

int main(void) {
    Graph_SP graph = buildGraphFromTglfFile("graphs/" "special/chainconfig01.tglf");
    graph->getSepMatrix().setCardinalOP(0, 1, CardinalDir::SOUTH);
    graph->getSepMatrix().setCardinalOP(16, 15, CardinalDir::NORTH);
    writeStringToFile(graph->writeTglf(), "output/" "chainconfig03_" "00_initial.tglf");

    // Work on a copy of the original graph.
    Graph_SP H = std::make_shared<Graph>(*graph);
    vector<deque<Node_SP>> chains, cycles;
    H->getChainsAndCycles(chains, cycles);
    COLA_ASSERT(chains.size() == 1);
    COLA_ASSERT(cycles.size() == 0);
    Chain chain(H, chains[0]);
    chain.takeShapeBasedConfiguration();
    if (true) {
        ColaOptions opts;
        H->project(opts, vpsc::XDIM);
        H->project(opts, vpsc::YDIM);
    }
    // Set the routes in the original graph.
    graph->clearAllRoutes();
    chain.addAestheticBendsToEdges();
    graph->buildRoutes();
    writeStringToFile(graph->writeTglf(), "output/" "chainconfig03_" "01_with_routes.tglf");

    return 0;
}
