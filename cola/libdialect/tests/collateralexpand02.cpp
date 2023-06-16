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

using namespace dialect;

using std::vector;
using std::cout;
using std::endl;

vector<id_type> nodes2ids(Nodes nodes) {
    vector<id_type> ids;
    for (Node_SP u : nodes) ids.push_back(u->id());
    return ids;
}

int main(void) {

    HolaOpts opts;
    // Build a graph.
    Graph_SP graph = buildGraphFromTglfFile("graphs/" "special/treeplacement5.tglf");
    // Peel.
    Trees trees = peel(*graph);
    // There should be two trees.
    COLA_ASSERT(trees.size() == 2);
    // Give the trees their symmetric layout.
    double iel = graph->getIEL();
    for (Tree_SP t : trees) t->symmetricLayout(CardinalDir::EAST, iel/4.0, iel, opts.preferConvexTrees);
    // Do an ortho routing and planarise the core.
    LeaflessOrthoRouter lor(graph, opts);
    lor.setShapeBufferDistanceIELScalar(0.125);
    lor.route();
    OrthoPlanariser op(graph);
    Graph_SP P = op.planarise();
    writeStringToFile(P->writeTglf(), "output/" "collateralexpand02_" "planar_layout.tglf");

    // Compute the face set.
    FaceSet faceSet(P);

    // Get ahold of the two trees, rooted at nodes 0 and 9.
    Tree_SP t0 = nullptr,
            t9 = nullptr;
    for (Tree_SP t : trees) {
        if (t->getRootNodeID() == 0) t0 = t;
        else if (t->getRootNodeID() == 9) t9 = t;
        if (t0 != nullptr && t9 != nullptr) break;
    }
    // Show the trees' layout.
    writeStringToFile(t0->underlyingGraph()->writeTglf(), "output/" "collateralexpand02_" "tree0_layout.tglf");
    writeStringToFile(t9->underlyingGraph()->writeTglf(), "output/" "collateralexpand02_" "tree9_layout.tglf");

    //bool verbose = false;

    // Get all possible placements, with the usual settings:
    opts.treePlacement_favourCardinal = true;
    opts.treePlacement_favourExternal = true;
    opts.treePlacement_favourIsolation = true;
    TreePlacements tps0 = faceSet.listAllPossibleTreePlacements(t0);
    //if (verbose) for (TreePlacement_SP tp : tps) cout << tp->toString();
    TreePlacements tps9 = faceSet.listAllPossibleTreePlacements(t9);

    // Get the NORTH placements.
    TreePlacement_SP north0 = nullptr,
                     north9 = nullptr;
    for (TreePlacement_SP tp : tps0) if (tp->getPlacementDir() == CompassDir::NORTH) {north0 = tp; break;}
    for (TreePlacement_SP tp : tps9) if (tp->getPlacementDir() == CompassDir::NORTH) {north9 = tp; break;}

    // Insert the tree box for the placement at node 0.
    double padding = iel/4.0;
    north0->insertTreeNode(padding);
    // Show the result.
    writeStringToFile(P->writeTglf(), "output/" "collateralexpand02_" "00_with_tree_box_at_0.tglf");

    // Now do collateral expansion for the placement at node 9.
    ProjSeq_SP ps9 = faceSet.getExternalFace()->doCollateralExpansion(north9);
    cout << ps9->toString() << endl;
    // Show the result.
    writeStringToFile(P->writeTglf(), "output/" "collateralexpand02_" "01_expanded_for_box_at_9.tglf");

    // Insert the tree box for the placement at node 9.
    north9->insertTreeNode(padding);
    // Show the result.
    writeStringToFile(P->writeTglf(), "output/" "collateralexpand02_" "02_with_tree_box_at_9.tglf");

    return 0;
}
