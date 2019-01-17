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

#include <iostream>
#include <vector>

#include "libvpsc/assertions.h"
#include "libdialect/io.h"
#include "libdialect/graphs.h"

using namespace dialect;

using std::cout;
using std::endl;
using std::vector;

int main(void) {
    Graph graph = *buildGraphFromTglfFile("graphs/" "special/core_with_trees.tglf");
    // First check the bounding box of the whole graph.
    BoundingBox bb1 = graph.getBoundingBox();
    cout << "Whole: " << bb1.repr() << endl;
    COLA_ASSERT(bb1 == BoundingBox(196, 811, 67, 653));
    // Now try ignoring some nodes.
    NodesById ignore_map;
    const NodesById &lookup = graph.getNodeLookup();
    // First try ignoring a set that actually changes the box.
    vector<id_type> ignore{7, 8, 9, 10};
    for (id_type id : ignore) {
        ignore_map.emplace(id, lookup.at(id));
    }
    BoundingBox bb2 = graph.getBoundingBox(ignore_map);
    cout << "Minus NE tree: " << bb2.repr() << endl;
    COLA_ASSERT(bb2 == BoundingBox(196, 720, 178, 653));
    // Now instead try ingoring a set that doesn't change the box.
    ignore = vector<id_type>{6, 2, 3, 5};
    ignore_map.clear();
    for (id_type id : ignore) {
        ignore_map.emplace(id, lookup.at(id));
    }
    BoundingBox bb3 = graph.getBoundingBox(ignore_map);
    cout << "Minus some nodes in the middle: " << bb3.repr() << endl;
    COLA_ASSERT(bb3 == bb1);
    return 0;
}
