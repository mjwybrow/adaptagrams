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
#include <set>
#include <iostream>

#include "libvpsc/assertions.h"

#include "libdialect/commontypes.h"
#include "libdialect/io.h"
#include "libdialect/graphs.h"
#include "libdialect/peeling.h"
#include "libdialect/util.h"
#include "libdialect/trees.h"

using namespace dialect;

using std::vector;
using std::cout;
using std::endl;

int main(void) {

    // First try peeling several trees off of a core:
    Graph_SP graph = buildGraphFromTglfFile("graphs/" "special/core_with_trees.tglf");
    Trees trees = peel(*graph);
    // Check that there are four trees.
    COLA_ASSERT(trees.size() == 4);
    // And also write each component to a separate file for visual check.
    writeStringToFile(graph->writeTglf(), "output/" "peel_" "core.tglf");
    size_t i = 0;
    for (Tree_SP t : trees) {
        writeStringToFile(
            t->underlyingGraph()->writeTglf(),
            string_format("output/" "peel_" "tree_%d.tglf", i++)
        );
    }
    // And check that all the right nodes were marked as root.
    Graphs allGraphs;
    allGraphs.push_back(graph);
    for (Tree_SP t : trees) allGraphs.push_back(t->underlyingGraph());
    std::set<id_type> rootIds{0, 2, 4, 5};
    auto end = rootIds.end();
    for (Graph_SP g : allGraphs) {
        for (auto p : g->getNodeLookup()) {
            Node_SP &u = p.second;
            if (rootIds.find(p.first) != end) {
                cout << "Node " << p.first << " is root." << endl;
                COLA_ASSERT(u->isRoot());
            } else {
                COLA_ASSERT(!u->isRoot());
            }
        }
    }

    // Now try a case where the whole graph is already a tree.
    Graph t = *buildGraphFromTglfFile("graphs/" "trees/tree01.tglf");
    trees = peel(t);
    // Check that there is exactly one tree.
    COLA_ASSERT(trees.size() == 1);
    // Also the original graph should have exactly one node left, which should
    // correspond to the root of the one tree.
    COLA_ASSERT(t.getNumNodes() == 1);
    Tree_SP tree = trees.front();
    for (auto p : t.getNodeLookup()) COLA_ASSERT(p.first == tree->getRootNodeID());

    return 0;
}
