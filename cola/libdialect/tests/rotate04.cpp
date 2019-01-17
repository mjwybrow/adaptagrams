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
#include "libdialect/ortho.h"
#include "libdialect/constraints.h"

using namespace dialect;

int main(void) {

    // Test rotation with constraints.

    Graph_SP graph = buildGraphFromTglfFile("graphs/" "special/rotate02.tglf");
    SepMatrix &m = graph->getSepMatrix();
    m.addSep(0, 1, GapType::BDRY, SepDir::EAST, SepType::INEQ, 30);
    m.addSep(0, 2, GapType::BDRY, SepDir::DOWN, SepType::INEQ, 150);
    m.addSep(1, 2, GapType::BDRY, SepDir::DOWN, SepType::INEQ, 150);
    ColaOptions colaOpts;
    colaOpts.idealEdgeLength = 30;
    graph->destress();

    HolaOpts opts;
    LeaflessOrthoRouter lor(graph, opts);
    lor.setShapeBufferDistanceIELScalar(0.125);
    lor.route();
    writeStringToFile(graph->writeTglf(), "output/" "rotate04_" "00_routing.tglf");

    graph->rotate90cw(&colaOpts);
    LeaflessOrthoRouter lor1(graph, opts);
    lor1.setShapeBufferDistanceIELScalar(0.125);
    lor1.route();
    writeStringToFile(graph->writeTglf(), "output/" "rotate04_" "01_rotate90cw.tglf");

    graph->rotate90cw(&colaOpts);
    LeaflessOrthoRouter lor2(graph, opts);
    lor2.setShapeBufferDistanceIELScalar(0.125);
    lor2.route();
    writeStringToFile(graph->writeTglf(), "output/" "rotate04_" "02_rotate90cw.tglf");

    graph->rotate90cw(&colaOpts);
    LeaflessOrthoRouter lor3(graph, opts);
    lor3.setShapeBufferDistanceIELScalar(0.125);
    lor3.route();
    writeStringToFile(graph->writeTglf(), "output/" "rotate04_" "03_rotate90cw.tglf");

    return 0;
}
