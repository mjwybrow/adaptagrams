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
    // bundle several edge routes together.

    // Here we only aim to test the edge overlap removal phase of the planarisation
    // process (not the subsequent edge intersection removal).

    // (1) Connector routing
    Graph_SP graph = buildGraphFromTglfFile("graphs/" "special/bundlerouting2.tglf");
    HolaOpts opts;
    LeaflessOrthoRouter lor(graph, opts);
    lor.setShapeBufferDistanceIELScalar(0.125);
    // We ask the router to record all routing attempts.
    lor.recordEachAttempt = true;
    lor.route();
    for (size_t j = 0; j < lor.routingAttemptTglf.size(); ++j) {
        writeStringToFile(lor.routingAttemptTglf[j], string_format("output/" "planarise01_" "routing_attempt_%d.tglf", j));
    }

    // (2) Edge overlap removal
    OrthoPlanariser op(graph);
    Graph_SP Q = op.planarise();
    writeStringToFile(Q->writeTglf(), "output/" "planarise01_" "planarise.tglf");

    return 0;
}
