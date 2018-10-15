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
    string inputFilename = "tree01m";
    Graph_SP graph = buildGraphFromTglfFile("graphs/" "trees/"+inputFilename+".tglf");
    Node_SP root = graph->getNode(0);
    Tree tree(graph, root);
    string rep = tree.repr();
    cout << rep << endl;
    const char *expected =
        "Tree: 0 1 2 3 4 5 6 7 8 9 10 11 12\n"
        "Rank 0: 0\n"
        "Rank 1: 1^ 2\n"
        "Rank 2: 3 4^ 5\n"
        "Rank 3: 6 8^\n"
        "Rank 4: 7\n"
        "Rank 5: 9^ 10^ 11^ 12^\n"
        "Isom: 0,0,0,0:1:0;1:0,1,1:0,1\n";
    COLA_ASSERT(rep == expected);

    // Now build a new Tree off the same Graph, but naming a different Node as root.
    Node_SP new_root = graph->getNode(6);
    Tree subtree(graph, new_root);
    rep = subtree.repr();
    cout << rep << endl;
    const char *e2 =
        "Tree: 6 7 9 10 11 12\n"
        "Rank 0: 6\n"
        "Rank 1: 7\n"
        "Rank 2: 9^ 10^ 11^ 12^\n"
        "Isom: 0,0,0,0:1\n";
    COLA_ASSERT(rep == e2);

    // Try symmetric layout.
    tree.symmetricLayout(CardinalDir::SOUTH, 15, 60);
    writeStringToFile(
        tree.underlyingGraph()->writeTglf(),
        "output/" "trees_" +inputFilename+".tglf"
    );

    return 0;
}
