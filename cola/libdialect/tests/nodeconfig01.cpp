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
#include "libdialect/nodeconfig.h"

using namespace dialect;

using std::vector;

int main(void) {

    // Obtain the core of a graph.
    Graph_SP graph = buildGraphFromTglfFile("graphs/" "special/core_with_trees.tglf");
    peel(*graph);
    // Do an OrthoHub layout on the core.
    OrthoHubLayoutOptions ohl_opts;
    OrthoHubLayout ohl(graph, ohl_opts);
    ohl.layout();
    // Write out.
    writeStringToFile(graph->writeTglf(), "output/" "nodeconfig01_" "core.tglf");
    return 0;
}
