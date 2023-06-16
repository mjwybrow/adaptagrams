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

using namespace dialect;

using std::vector;
using std::cout;
using std::endl;

int main(void) {

    HolaOpts opts;
    // Build the graph.
    Graph_SP graph = buildGraphFromTglfFile("graphs/" "special/expand05.tglf");
    // Peel.
    Trees trees = peel(*graph);

    // Give the trees their symmetric layout.
    double iel = graph->getIEL();
    Tree_SP t0 = nullptr;
    for (Tree_SP t : trees) {
        t->symmetricLayout(CardinalDir::EAST, iel/4.0, iel, opts.preferConvexTrees);
        id_type rid = t->getRootNodeID();
        if (rid == 0) t0 = t;
    }

    // Planarise the core.
    OrthoPlanariser op(graph);
    Graph_SP P = op.planarise();
    writeStringToFile(P->writeTglf(), "output/" "expand05_" "00_planar_layout.tglf");

    // Compute the faces of the core.
    FaceSet_SP faceset = std::make_shared<FaceSet>(P);
    // Get all placements for the tree.
    TreePlacements tps0 = faceset->listAllPossibleTreePlacements(t0);
    // Find the one we want.
    TreePlacement_SP tp0 = nullptr;
    for (TreePlacement_SP tp : tps0) {
        if (tp->getPlacementDir() == CompassDir::SE && tp->getGrowthDir() == CardinalDir::EAST) {
            tp0 = tp;
            break;
        }
    }

    // Attempt to expand for the placement.
    double padding = P->getIEL()/4.0;
    ProjSeq_SP ps = tp0->buildBestProjSeq(padding, true);

    ColaOptions colaOpts;
    colaOpts.preventOverlaps = true;
    colaOpts.solidifyAlignedEdges = true;
    P->applyProjSeq(colaOpts, *ps);

    // Insert the tree node.
    tp0->insertTreeNode(padding);
    writeStringToFile(P->writeTglf(), "output/" "expand05_" "01_insert_tree_box.tglf");

    return 0;
}
