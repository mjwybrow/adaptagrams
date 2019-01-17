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
#include <memory>

#include "libvpsc/assertions.h"
#include "libvpsc/rectangle.h"

#include "libdialect/commontypes.h"
#include "libdialect/io.h"
#include "libdialect/graphs.h"
#include "libdialect/util.h"
#include "libdialect/constraints.h"

using namespace dialect;

using std::string;

int main(void) {

    // Testing goals:
    // We want to try using a ProjSeq and Graph::applyProjSeq to progressively
    // add constraints to a layout.
    // We want to see that if any of the projections in a given sequence fails,
    // then the result is accurately reported.

    // (1) Build a graph.
    Graph_SP graph = buildGraphFromTglfFile("graphs/" "special/column8.tglf");
    // Show initial layout.
    writeStringToFile(graph->writeTglf(), "output/" "projseq01_" "layout00.tglf");

    // (2) Try first ProjSeq. Just one projection, with two sepcos.
    ProjSeq ps;
    SepCoSet sepcos;
    SepCo_SP s0 = std::make_shared<SepCo>(
        vpsc::XDIM,
        graph->getNode(0),
        graph->getNode(1),
        60);
    SepCo_SP s1 = std::make_shared<SepCo>(
        vpsc::XDIM,
        graph->getNode(1),
        graph->getNode(2),
        60);
    sepcos.insert(s0);
    sepcos.insert(s1);
    ps.addProjection(sepcos, vpsc::XDIM);
    ColaOptions opts;
    graph->applyProjSeq(opts, ps);
    writeStringToFile(graph->writeTglf(), "output/" "projseq01_" "layout01.tglf");

    // Next: Two projections, with two sepcos each.
    // We also test the automatic monotonicity of the constraint sets by using a fresh set.
    SepCoSet sepcos2;
    SepCo_SP s2 = std::make_shared<SepCo>(
        vpsc::XDIM,
        graph->getNode(2),
        graph->getNode(3),
        60);
    SepCo_SP s3 = std::make_shared<SepCo>(
        vpsc::XDIM,
        graph->getNode(3),
        graph->getNode(4),
        60);
    sepcos2.insert(s2);
    sepcos2.insert(s3);
    ps.addProjection(sepcos, vpsc::XDIM);
    SepCo_SP s4 = std::make_shared<SepCo>(
        vpsc::XDIM,
        graph->getNode(4),
        graph->getNode(5),
        60);
    SepCo_SP s5 = std::make_shared<SepCo>(
        vpsc::XDIM,
        graph->getNode(5),
        graph->getNode(6),
        60);
    sepcos2.insert(s4);
    sepcos2.insert(s5);
    ps.addProjection(sepcos2, vpsc::XDIM);
    graph->applyProjSeq(opts, ps);
    string tglf2 = graph->writeTglf();
    writeStringToFile(tglf2, "output/" "projseq01_" "layout02.tglf");

    // Now add a projection that should fail.
    SepCo_SP s6 = std::make_shared<SepCo>(
        vpsc::XDIM,
        graph->getNode(6),
        graph->getNode(7),
        60);
    SepCo_SP s7n = std::make_shared<SepCo>(
        vpsc::XDIM,
        graph->getNode(7),
        graph->getNode(0),
        60);
    SepCoSet sepcos3;
    sepcos3.insert(s6);
    sepcos3.insert(s7n);
    ps.addProjection(sepcos3, vpsc::XDIM);
    bool success = graph->applyProjSeq(opts, ps);
    COLA_ASSERT(!success);
    // Node positions should be unaffected as well.
    string tglf2b = graph->writeTglf();
    COLA_ASSERT(tglf2b == tglf2);

    return 0;
}
