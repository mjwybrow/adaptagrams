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
#include <string>

#include "libvpsc/assertions.h"
#include "libvpsc/rectangle.h"

#include "libdialect/commontypes.h"
#include "libdialect/io.h"
#include "libdialect/graphs.h"
#include "libdialect/util.h"
#include "libdialect/routing.h"
#include "libdialect/opts.h"
#include "libdialect/planarise.h"

using namespace dialect;

using std::string;

int main(void) {

    // Testing goals:
    // We want to try solidifying aligned edges in each dimension, in a copy H of
    // a Graph G having planar orthogonal layout.
    //
    // We want to see both that the generated constraints on H look right, and that
    // the existing constraints on G are unaffected. The reason for the test is that,
    // while each Graph has its own SepMatrix, they share SepPairs.

    // (1) Make a planar orthogonal layout, as in test case, plaraise02.
    Graph_SP graph = buildGraphFromTglfFile("graphs/" "special/bundlecross.tglf");
    HolaOpts opts;
    ColaOptions colaOpts;
    LeaflessOrthoRouter lor(graph, opts);
    lor.setShapeBufferDistanceIELScalar(0.125);
    lor.route();
    OrthoPlanariser op(graph);
    Graph_SP P = op.planarise();
    string P_tglf_orig = P->writeTglf();
    writeStringToFile(P_tglf_orig, "output/" "solidify_" "planarise.tglf");

    // (2) Make a copy of the planar Graph P, and try solidifying horizontal edges in it.
    Graph H0(*P);
    H0.solidifyAlignedEdges(vpsc::HORIZONTAL, colaOpts);
    // Draw the result.
    writeStringToFile(H0.writeTglf(), "output/" "solidify_" "solid_horiz.tglf");
    // And write the TGLF for the planar graph P too, so we can examine its constraints.
    string P_tglf_after_solid_horiz = P->writeTglf();
    COLA_ASSERT(P_tglf_after_solid_horiz == P_tglf_orig);

    // (3) Try same, only this time with the vertical edges.
    Graph H1(*P);
    H1.solidifyAlignedEdges(vpsc::VERTICAL, colaOpts);
    // Draw the result.
    writeStringToFile(H1.writeTglf(), "output/" "solidify_" "solid_vert.tglf");
    // And write the TGLF for the planar graph P too, so we can examine its constraints.
    string P_tglf_after_solid_vert = P->writeTglf();
    COLA_ASSERT(P_tglf_after_solid_vert == P_tglf_orig);

    // (4) Try destressing.
    // We will try it only for the case with solid horizontal edges.
    // In this case, we should only operate in the y-dimension, i.e. only destress in that dimension.
    // We also test that we can set an extra boundary gap in the sep matrix, and this won't matter; that's
    // because even though the solidifyAlignedEdges function uses BDRY gaps between nodes and edge-nodes,
    // the "extra gap" is in the x-dimension, and we're not operating in that dimension.
    H0.getSepMatrix().setExtraBdryGap(15);
    colaOpts.xAxis = false;
    H0.destress(colaOpts);
    writeStringToFile(H0.writeTglf(), "output/" "solidify_" "solid_horiz_destressed.tglf");
    writeStringToFile(P->writeTglf(), "output/" "solidify_" "planar_after_horiz_destressed.tglf");
    /*
    ColaOptions opts;
    opts.preventOverlaps = true;
    H0.destress(opts);
    */


    return 0;
}
