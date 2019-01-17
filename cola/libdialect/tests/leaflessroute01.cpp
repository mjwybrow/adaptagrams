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

int main(void) {

    // Peel off trees to obtain a core graph.
    Graph_SP graph = buildGraphFromTglfFile("graphs/" "random/v40e42.tglf");
    peel(*graph);
    // Destress.
    graph->destress();
#define OHL
#ifdef OHL
    // Do an ortho hub layout.
    OrthoHubLayoutOptions ohl_opts;
    OrthoHubLayout ohl(graph, ohl_opts);
    ohl.layout();
#endif
    // Now do leafless routing.
    HolaOpts opts;
    LeaflessOrthoRouter lor(graph, opts);
    lor.route();
    writeStringToFile(graph->writeTglf(), "output/" "leaflessroute01_" "core.tglf");
    return 0;
}
