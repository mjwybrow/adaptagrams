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
    Graph_SP graph = buildGraphFromTglfFile("graphs/" "special/3cross4cross_wTrees.tglf");
    // Peel.
    Trees trees = peel(*graph);
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
    writeStringToFile(P->writeTglf(), "output/" "treeplacement01_" "planar_layout.tglf");

    // Compute the face set.
    FaceSet faceSet(P);

    // Get ahold of the trees we're interested in.
    Tree_SP t4 = nullptr,
            t11 = nullptr;
    for (Tree_SP t : trees) {
        if (t->getRootNodeID() == 4) t4 = t;
        else if (t->getRootNodeID() == 11) t11 = t;
    }

    bool verbose = false;

    // Check the possible tree placements at node 4 (the "3-crossing").
    // There should be 18 of them.
    // Here the asymmetry of the tree rooted at node 4 contributes a factor of 2
    // in the total number of placements, since asymmetric trees can be flipped.
    opts.treePlacement_favourCardinal = false;
    TreePlacements tps1 = faceSet.listAllPossibleTreePlacements(t4);
    if (verbose) for (TreePlacement_SP tp : tps1) cout << tp->toString();
    COLA_ASSERT(tps1.size() == 18);

    // Favouring cardinal placements should *not* change the number of placements.
    // (In an older version of HOLA, this used to eliminate all ordinal placements
    // from even being considered.)
    opts.treePlacement_favourCardinal = true;
    TreePlacements tps2 = faceSet.listAllPossibleTreePlacements(t4);
    if (verbose) for (TreePlacement_SP tp : tps2) cout << tp->toString();
    COLA_ASSERT(tps2.size() == 18);

    // Next we move to node 11 (the "4-crossing").
    // Here we should get 8 ordinal placments.
    // Also, it is because this tree has a symmetric layout that we get only 8,
    // not 16 placements.
    TreePlacements tps3 = faceSet.listAllPossibleTreePlacements(t11);
    if (verbose) for (TreePlacement_SP tp : tps3) cout << tp->toString();
    COLA_ASSERT(tps3.size() == 8);

    // Now we want to see which placements are chosen as best.
    // For the tree at node 4, while favouring cardinal placement, obviously one of
    // the EAST placements should be chosen.
    TreePlacement_SP best1 = chooseBestPlacement(tps1, opts);
    if (verbose) cout << best1->toString();
    COLA_ASSERT(best1->getPlacementDir() == CompassDir::EAST);

    // As for the tree at node 11, favouring cardinal placement does not matter,
    // since only ordinal placements are possible. Favouring the external face
    // also does not matter, since all placements are external.
    // However we should find that favouring isolation forces choice of the
    // southeast placement direction, due to the trees rooted at nodes 12 and 21.
    TreePlacement_SP best2 = chooseBestPlacement(tps3, opts);
    if (verbose) cout << best2->toString();
    COLA_ASSERT(best2->getPlacementDir() == CompassDir::SE);

    return 0;
}
