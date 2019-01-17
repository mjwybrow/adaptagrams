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

#include "libvpsc/assertions.h"
#include "libvpsc/rectangle.h"

#include "libdialect/io.h"
#include "libdialect/constraints.h"
#include "libdialect/ortho.h"
#include "libdialect/graphs.h"

using namespace dialect;

int main(void) {

    Graph_SP graph = buildGraphFromTglfFile("graphs/" "special/X_4.tglf");
    SepMatrix &m = graph->getSepMatrix();
    m.setCardinalOP(4, 9, CardinalDir::EAST);
    m.setCardinalOP(4, 5, CardinalDir::SOUTH);
    m.setCardinalOP(4, 3, CardinalDir::WEST);
    m.setCardinalOP(4, 13, CardinalDir::NORTH);
    ColaOptions opts;
    opts.idealEdgeLength = 10;
    opts.preventOverlaps = true;
    graph->destress(opts);
    writeStringToFile(graph->writeTglf(), "output/" "extrabdrygap_" "01_zero_gap.tglf");
    m.setExtraBdryGap(30);
    graph->destress(opts);
    writeStringToFile(graph->writeTglf(), "output/" "extrabdrygap_" "02_extra_gap.tglf");

}
