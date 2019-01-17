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

int main(void) {
    Graph_SP graph = buildGraphFromTglfFile("graphs/" "special/chainconfig01.tglf");
    graph->getSepMatrix().setCardinalOP(0, 1, CardinalDir::SOUTH);
    graph->getSepMatrix().setCardinalOP(16, 15, CardinalDir::NORTH);
    writeStringToFile(graph->writeTglf(), "output/" "chainconfig01_" "00_initial.tglf");
    vector<deque<Node_SP>> chains, cycles;
    graph->getChainsAndCycles(chains, cycles);
    COLA_ASSERT(chains.size() == 1);
    COLA_ASSERT(cycles.size() == 0);
    Chain chain(graph, chains[0]);
    chain.takeShapeBasedConfiguration();
    ColaOptions opts;
    graph->project(opts, vpsc::XDIM);
    writeStringToFile(graph->writeTglf(), "output/" "chainconfig01_" "01_projX.tglf");
    graph->project(opts, vpsc::YDIM);
    writeStringToFile(graph->writeTglf(), "output/" "chainconfig01_" "02_projY.tglf");
    graph->destress();
    writeStringToFile(graph->writeTglf(), "output/" "chainconfig01_" "03_destress.tglf");
    return 0;
}
