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

    // Purpose of test: See that an interior face can be expanded to make room for a single tree box.

    HolaOpts opts;
    // Build a graph.
    Graph_SP graph = buildGraphFromTglfFile("graphs/" "special/treeplacement4.tglf");
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
    writeStringToFile(P->writeTglf(), "output/" "expand01_" "planar_layout.tglf");

    // Compute the face set.
    FaceSet faceSet(P);

    // Get ahold of the tree rooted at node 0.
    Tree_SP t0 = nullptr;
    for (Tree_SP t : trees) {
        if (t->getRootNodeID() == 0) {
            t0 = t;
            break;
        }
    }
    // Show the tree's layout.
    //writeStringToFile(t0->underlyingGraph()->writeTglf(), "output/" "expand01_" "tree_layout.tglf");

    bool verbose = false;

    // Get all possible placements, with the usual settings:
    opts.treePlacement_favourCardinal = true;
    opts.treePlacement_favourExternal = true;
    opts.treePlacement_favourIsolation = true;
    TreePlacements tps = faceSet.listAllPossibleTreePlacements(t0);
    if (verbose) for (TreePlacement_SP tp : tps) cout << tp->toString();

    // If in choosing a best placement we no longer favour external placement,
    // then the tree should be placed SOUTH, due to isolation.
    opts.treePlacement_favourExternal = false;
    TreePlacement_SP best = chooseBestPlacement(tps, opts);
    if (verbose) cout << best->toString();
    COLA_ASSERT(best->getPlacementDir() == CompassDir::SOUTH);

    // Expand the face to make room for the tree box.
    bool doCostlierDimFirst = true;
    double padding = iel/4.0;
    ProjSeq_SP ps = best->buildBestProjSeq(padding, doCostlierDimFirst);
    ColaOptions colaOpts;
    colaOpts.preventOverlaps = true;
    colaOpts.solidifyAlignedEdges = true;
    P->applyProjSeq(colaOpts, *ps);
    writeStringToFile(P->writeTglf(), "output/" "expand01_" "expanded.tglf");

    // Insert the tree box for this placement.
    best->insertTreeNode(padding);
    // Show the result.
    std::string tglfWithBox = P->writeTglf();
    writeStringToFile(tglfWithBox, "output/" "expand01_" "expanded_with_tree_box.tglf");




    return 0;
}
