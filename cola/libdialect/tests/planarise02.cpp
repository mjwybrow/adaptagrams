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
#include "libdialect/util.h"
#include "libdialect/routing.h"
#include "libdialect/opts.h"
#include "libdialect/planarise.h"

using namespace dialect;

int main(void) {

    // We use a graph deliberately set up so that the initial routing will
    // not only bundle several edge routes together, but will also route
    // a connector across (intersecting) that bundle.

    // Here we aim to test both the edge overlap removal phase and the
    // subsequent edge intersection removal phase of the planarisation process.

    // (1) Connector routing
    Graph_SP graph = buildGraphFromTglfFile("graphs/" "special/bundlecross.tglf");
    HolaOpts opts;
    LeaflessOrthoRouter lor(graph, opts);
    lor.setShapeBufferDistanceIELScalar(0.125);
    // We ask the router to record all routing attempts.
    lor.recordEachAttempt = true;
    lor.route();
    for (size_t j = 0; j < lor.routingAttemptTglf.size(); ++j) {
        writeStringToFile(lor.routingAttemptTglf[j], string_format("output/" "planarise02_" "routing_attempt_%d.tglf", j));
    }

    // (2) Planarisation
    OrthoPlanariser op(graph);
    Graph_SP Q = op.planarise();
    writeStringToFile(Q->writeTglf(), "output/" "planarise02_" "planarise.tglf");

    return 0;
}
