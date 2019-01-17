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
#include <memory>

#include "libvpsc/assertions.h"

#include "libdialect/commontypes.h"
#include "libdialect/io.h"
#include "libdialect/graphs.h"
#include "libdialect/util.h"
#include "libdialect/routing.h"
#include "libdialect/opts.h"
#include "libdialect/planarise.h"
#include "libdialect/faces.h"
#include "libdialect/peeling.h"
#include "libdialect/trees.h"
#include "libdialect/treeplacement.h"
#include "libdialect/constraints.h"
#include "libdialect/nodeconfig.h"
#include "libdialect/logging.h"

using namespace dialect;

using std::vector;
using std::cout;
using std::endl;

int main(void) {

    Logger *logger = new Logger("output/", "expand07_", true);
    Graph G = *buildGraphFromTglfFile("graphs/" "special/expand07.tglf");

//#define cut10
#ifdef cut10
    G.severAndRemoveNode(10);
    G.addEdgeByIDs(9, 14);
    G.getSepMatrix().vAlign(9, 14);
    logger->log(G, "000_cut_10");
#endif

    vector<id_type> ids{
        4, 3, 13, 5, 6, 7
    };
    for (id_type id : ids) G.severAndRemoveNode(id);

//#define EBG
#ifdef EBG
    G.getSepMatrix().setExtraBdryGap(20);
#endif

    logger->log(G, "00_initial");

    ColaOptions colaOpts;
    colaOpts.preventOverlaps = true;
    colaOpts.solidifyAlignedEdges = true;
    colaOpts.logger = logger;
    for (int i = 1; i < 5; ++i) {
        logger->nextLoggingIndex = i;
        G.makeFeasible(colaOpts);
        logger->log(G, string_format("%02d_make_feasible_result.tglf", i));
    }

    return 0;
}
