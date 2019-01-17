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
#include <iostream>

#include "libvpsc/assertions.h"

#include "libdialect/commontypes.h"
#include "libdialect/io.h"
#include "libdialect/util.h"
#include "libdialect/graphs.h"
#include "libdialect/opts.h"
#include "libdialect/nearalign.h"

using namespace dialect;

using std::vector;
using std::cout;
using std::endl;

int main(void) {
    Graph_SP graph = buildGraphFromTglfFile("graphs/" "special/nearalign01.tglf");
    HolaOpts opts;
    NodesById ignore;
    AlignmentTable atab(*graph, ignore);
    size_t sc = doNearAlignments(*graph, atab, ignore, opts);
    COLA_ASSERT(sc == 2);
    writeStringToFile(graph->writeTglf(), "output/" "nearalign01_" "01_nearalign.tglf");
    graph->destress();
    writeStringToFile(graph->writeTglf(), "output/" "nearalign01_" "02_destress.tglf");
    return 0;
}
