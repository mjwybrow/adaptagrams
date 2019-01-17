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

#include "libvpsc/assertions.h"

#include "libdialect/commontypes.h"
#include "libdialect/io.h"
#include "libdialect/graphs.h"
#include "libdialect/peeling.h"
#include "libdialect/util.h"
#include "libdialect/trees.h"
#include "libdialect/ortho.h"

using namespace dialect;

using std::vector;

int main(void) {
    Graph graph = *buildGraphFromTglfFile("graphs/" "special/core_with_trees.tglf");
    double iel = graph.getIEL();
    Trees trees = peel(graph);
    // Check that there are four trees.
    COLA_ASSERT(trees.size() == 4);
    // And also write each component to a separate file for visual check.
    writeStringToFile(graph.writeTglf(), "output/" "symmtree_" "core.tglf");
    size_t i = 0;
    vector<CardinalDir> dirs{
        CardinalDir::NORTH, CardinalDir::EAST, CardinalDir::SOUTH, CardinalDir::WEST
    };
    for (Tree_SP t : trees) {
        t->symmetricLayout(dirs[i], iel/4, iel);
        writeStringToFile(
            t->underlyingGraph()->writeTglf(),
            string_format("output/" "symmtree_" "tree_%d.tglf", i++)
        );
    }
    return 0;
}
