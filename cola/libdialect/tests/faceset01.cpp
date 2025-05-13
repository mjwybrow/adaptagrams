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

    // Build a 4-planar, orthogonal layout.
    Graph_SP graph = buildGraphFromTglfFile("graphs/" "special/bundlecross.tglf");
    HolaOpts opts;
    LeaflessOrthoRouter lor(graph, opts);
    lor.setShapeBufferDistanceIELScalar(0.125);
    lor.route();
    OrthoPlanariser op(graph);
    Graph_SP P = op.planarise();
    writeStringToFile(P->writeTglf(), "output/" "faceset01_" "planar_layout.tglf");
    // Compute the face set.
    FaceSet faceSet(P);

    // There should be seven faces.
    COLA_ASSERT(faceSet.getNumFaces() == 7);

    // Check each face's node sequence.
    vector<id_type> idSeqs[7] = {
        {8, 5, 39, 22, 20, 0},
        {5, 8, 0, 7, 6, 18, 4, 23, 19},
        {5, 19, 23, 21, 39},
        {0, 20, 1, 2, 22, 39, 6, 7},
        {20, 22, 2, 1},
        {21, 23, 4, 3},
        {3, 4, 18, 6, 39, 21}
    };
    Faces faces = faceSet.getFaces();
    for (size_t i = 0; i < 7; ++i) {
        Face_SP F = faces[i];
        vector<id_type> ids = nodes2ids(F->getNodeSeq());
        for (id_type id : ids) cout << id << " ";
        cout << endl;
        COLA_ASSERT(ids == idSeqs[i]);
    }

    // Check external face.
    for (size_t i = 0; i < 7; ++i) {
        Face_SP F = faces[i];
        if (i == 1) { COLA_ASSERT(F->isExternal()); }
        else { COLA_ASSERT(!F->isExternal()); }
    }

    // Look at the Sides and Nexes for the concave face #3:
    Face_SP cc = faces[3];
    cout << "Sides and Nexes for lower-left concave Face:" << endl;
    for (Side_SP S : cc->getSides()) cout << S->toString() << endl;
    for (auto p : cc->getNexusLookup()) cout << p.second->toString();

    return 0;
}
