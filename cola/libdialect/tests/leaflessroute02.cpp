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

#include "libvpsc/assertions.h"

#include "libdialect/commontypes.h"
#include "libdialect/io.h"
#include "libdialect/graphs.h"
#include "libdialect/peeling.h"
#include "libdialect/util.h"
#include "libdialect/routing.h"
#include "libdialect/opts.h"
#include "libdialect/nodeconfig.h"

using namespace dialect;

using std::vector;

void addBendsToGraph(NodesById bends, Graph_SP G) {
    for (auto pair : bends) {
        Node_SP &u = pair.second;
        G->addNode(u);
    }
}

int main(void) {

    // This tests both leafless routing, and the building of unique bend points in a graph.

    // (1) Leafless routing.
    // We use a graph deliberately set up so that the initial routing will turn
    // one of the nodes into a "pseudoleaf", i.e. will route all its edges to
    // just one of its sides (in fact to just a single point on its boundary).
    Graph_SP graph = buildGraphFromTglfFile("graphs/" "special/samesiderouting.tglf");
    HolaOpts opts;
    LeaflessOrthoRouter lor(graph, opts);
    lor.setShapeBufferDistanceIELScalar(0.125);
    // We ask the router to record all routing attempts, so we can see the two steps.
    lor.recordEachAttempt = true;
    lor.route();
    for (size_t j = 0; j < lor.routingAttemptTglf.size(); ++j) {
        writeStringToFile(lor.routingAttemptTglf[j], string_format("output/" "leaflessroute02_" "attempt_%d.tglf", j));
    }

    // (2) Building unique bend points.
    // The two routing attempts from part (1) provide a nice example for testing Graph::buildUniqueBendPoints.
    // Start by building a Graph from the TGLF of each attempt.
    Graph_SP A0 = buildGraphFromTglf(lor.routingAttemptTglf[0]),
             A1 = buildGraphFromTglf(lor.routingAttemptTglf[1]);
    NodesById bends0 = A0->buildUniqueBendPoints(),
              bends1 = A1->buildUniqueBendPoints();
    // As a test of the uniqueness of the bend points, there should be only five (not six) bends for the
    // first routing attempt.
    COLA_ASSERT(bends0.size() == 5);
    // Beyond that, we add the bend points to the graphs and draw them, for a visual check.
    addBendsToGraph(bends0, A0);
    addBendsToGraph(bends1, A1);
    writeStringToFile(A0->writeTglf(), "output/" "leaflessroute02_" "bends_0.tglf");
    writeStringToFile(A1->writeTglf(), "output/" "leaflessroute02_" "bends_1.tglf");

    return 0;
}
