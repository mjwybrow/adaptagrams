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
#include "libavoid/geomtypes.h"

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
#include "libdialect/expansion.h"

using namespace dialect;

using Avoid::Point;

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
    writeStringToFile(P->writeTglf(), "output/" "containedsegment01_" "planar_layout.tglf");

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
    //writeStringToFile(t0->underlyingGraph()->writeTglf(), "output/" "containedsegment01_" "tree_layout.tglf");

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

    // Compute the ContainedSegments for this placement.
    vector<ContainedSegment_SP> cses;
    ExpansionManager em(best, vpsc::HORIZONTAL);
    for (ExpansionGoal_SP eg : em.getGoals()) {
        bool ignoreCollateralTreeBoxes = true;
        ContainedSegment_SP cs = eg->computeContainedSegment(ignoreCollateralTreeBoxes);
        cses.push_back(cs);
        cout << cs->toString() << endl;
    }

    // Insert the tree box for this placement.
    double padding = iel/4.0;
    best->insertTreeNode(padding);
    // Show the result.
    std::string tglfWithBox = P->writeTglf();
    writeStringToFile(tglfWithBox, "output/" "containedsegment01_" "with_tree_box.tglf");

    // Check
    Node_SP u0 = graph->getNode(0),
            u1 = graph->getNode(1),
            u2 = graph->getNode(2),
            u7 = graph->getNode(7),
            ub = best->getBoxNode();
    cout << "box node id: " << ub->id() << endl;
    Point   c0 = u0->getCentre(),
            c1 = u1->getCentre(),
            c7 = u7->getCentre();
    BoundingBox b0 = u0->getBoundingBox(),
                b2 = u2->getBoundingBox(),
                bb = ub->getBoundingBox();

    cout << "Basept should be: " << c0.x << "," << b0.Y << endl;
    cout << "South goal should be to " << b2.y << " toward " << bb.Y << endl;
    cout << "West goal should be to " << c7.x << " toward " << bb.x << endl;
    cout << "East goal should be to " << c1.x << " toward " << bb.X << endl;

    Point bp = Point(c0.x, b0.Y);
    COLA_ASSERT(cses[0]->basept == bp);
    COLA_ASSERT(cses[1]->basept == bp);
    COLA_ASSERT(cses[2]->basept == bp);

    COLA_ASSERT(cses[0]->localGoalPt.y  == b2.y);
    COLA_ASSERT(cses[0]->globalGoalPt.y == bb.Y);

    COLA_ASSERT(cses[1]->localGoalPt.x  == c7.x);
    COLA_ASSERT(cses[1]->globalGoalPt.x == bb.x);

    COLA_ASSERT(cses[2]->localGoalPt.x  == c1.x);
    COLA_ASSERT(cses[2]->globalGoalPt.x == bb.X);

    return 0;
}
