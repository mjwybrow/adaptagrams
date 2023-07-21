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
    Graph_SP graph = buildGraphFromTglfFile("graphs/" "special/treeplacement2.tglf");
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
    writeStringToFile(P->writeTglf(), "output/" "treeplacement02_" "planar_layout.tglf");

    // Compute the face set.
    FaceSet faceSet(P);

    // Get ahold of the trees we're interested in.
    Tree_SP t0 = nullptr,
            t1 = nullptr;
    for (Tree_SP t : trees) {
        if (t->getRootNodeID() == 0) t0 = t;
        else if (t->getRootNodeID() == 1) t1 = t;
    }

    bool verbose = false;

    // Consider the tree rooted at node 0.
    // There should be ten placements.
    opts.treePlacement_favourCardinal = false;
    opts.treePlacement_favourExternal = false;
    TreePlacements tps1 = faceSet.listAllPossibleTreePlacements(t0);
    if (verbose) for (TreePlacement_SP tp : tps1) cout << tp->toString();
    COLA_ASSERT(tps1.size() == 10);

    return 0;
}
