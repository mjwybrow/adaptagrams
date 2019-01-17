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
#include <memory>

#include "libvpsc/assertions.h"
#include "libdialect/commontypes.h"
#include "libdialect/trees.h"
#include "libdialect/io.h"
#include "libdialect/graphs.h"
#include "libdialect/ortho.h"

using std::string;
using std::cout;
using std::endl;

using namespace dialect;

int main(void) {

    // Build a tree.
    string inputFilename = "tree02";
    Graph_SP graph = buildGraphFromTglfFile("graphs/" "trees/"+inputFilename+".tglf");
    Node_SP root = graph->getNode(0);
    Tree tree(graph, root);
    string rep = tree.repr();
    cout << rep << endl;
    // Try symmetric layout with convex ordering...
    tree.symmetricLayout(CardinalDir::SOUTH, 15, 60);
    writeStringToFile(
        tree.underlyingGraph()->writeTglf(),
        "output/" "trees2_" +inputFilename+"_convex.tglf"
    );
    // ...and with concave ordering:
    tree.symmetricLayout(CardinalDir::SOUTH, 15, 60, false);
    writeStringToFile(
        tree.underlyingGraph()->writeTglf(),
        "output/" "trees2_" +inputFilename+"_concave.tglf"
    );
    return 0;
}
