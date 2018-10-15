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

using namespace dialect;

using std::vector;
using std::cout;
using std::endl;

int main(void) {

    HolaOpts opts;
    // Build a graph.
    Graph_SP graph = buildGraphFromTglfFile("graphs/" "special/expand03.tglf");
    // Peel.
    Trees trees = peel(*graph);
    // Do an OrthoHub layout on the core.
    OrthoHubLayoutOptions ohl_opts;
    OrthoHubLayout ohl(graph, ohl_opts);
    ohl.layout();
    // Show result.
    writeStringToFile(graph->writeTglf(), "output/" "expand03_" "ortho_core.tglf");
    // There should be four trees.
    COLA_ASSERT(trees.size() == 4);
    // Give the trees their symmetric layout.
    double iel = graph->getIEL();
    for (Tree_SP t : trees) t->symmetricLayout(CardinalDir::EAST, iel/4.0, iel, opts.preferConvexTrees);
    // Do an ortho routing and planarise the core.
    LeaflessOrthoRouter lor(graph, opts);
    lor.setShapeBufferDistanceIELScalar(0.125);
    lor.route();
    OrthoPlanariser op(graph);
    Graph_SP P = op.planarise();
    //writeStringToFile(P->writeTglf(), "output/" "expand03_" "planar_layout.tglf");

    // Reattach trees.
    //P->m_debugOutputPath = "output/" "expand03_";
    //P->m_projectionDebugLevel = 2;
    reattachTrees(P, trees, opts);
    writeStringToFile(P->writeTglf(), "output/" "expand03_" "with_tree_boxes.tglf");

    return 0;
}
