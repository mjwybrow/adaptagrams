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
    // Build a graph.
    Graph_SP graph = buildGraphFromTglfFile("graphs/" "special/expand04.tglf");
    // Peel.
    Trees trees = peel(*graph);

    // Give the trees their symmetric layout.
    double iel = graph->getIEL();
    Tree_SP t0 = nullptr,
            t1 = nullptr;
    for (Tree_SP t : trees) {
        t->symmetricLayout(CardinalDir::EAST, iel/4.0, iel, opts.preferConvexTrees);
        id_type rid = t->getRootNodeID();
        if (rid == 0) t0 = t;
        else if (rid == 1) t1 = t;
    }

    // Planarise the core.
    OrthoPlanariser op(graph);
    Graph_SP P = op.planarise();
    writeStringToFile(P->writeTglf(), "output/" "expand04_" "00_planar_layout.tglf");

    // Compute the faces of the core.
    FaceSet_SP faceset = std::make_shared<FaceSet>(P);
    // Get all placements for both trees.
    TreePlacements tps0 = faceset->listAllPossibleTreePlacements(t0);
    TreePlacements tps1 = faceset->listAllPossibleTreePlacements(t1);
    // Find the ones we want.
    TreePlacement_SP tp0 = nullptr,
                     tp1 = nullptr;
    for (TreePlacement_SP tp : tps0) {
        if (tp->getPlacementDir() == CompassDir::EAST) {
            tp0 = tp;
            break;
        }
    }
    for (TreePlacement_SP tp : tps1) {
        if (tp->getPlacementDir() == CompassDir::WEST) {
            tp1 = tp;
            break;
        }
    }
    // Attach the one at Node 1.
    double padding = P->getIEL()/4.0;
    tp1->insertTreeNode(padding);
    writeStringToFile(P->writeTglf(), "output/" "expand04_" "01_with_first_tree_box.tglf");

    // Now try to expand for the placement at Node 0.
    ProjSeq_SP ps = tp0->buildBestProjSeq(padding, true);
    writeStringToFile(P->writeTglf(), "output/" "expand04_" "02_build_proj_seq.tglf");

    ColaOptions colaOpts;
    colaOpts.preventOverlaps = true;
    colaOpts.solidifyAlignedEdges = true;
    P->applyProjSeq(colaOpts, *ps);
    writeStringToFile(P->writeTglf(), "output/" "expand04_" "03_apply_proj_seq.tglf");

    // Insert the other tree node.
    tp0->insertTreeNode(padding);
    writeStringToFile(P->writeTglf(), "output/" "expand04_" "04_insert_second_tree_box.tglf");

    return 0;
}
