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
#include <memory>

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
#include "libdialect/nodeconfig.h"
#include "libdialect/logging.h"

using namespace dialect;

using std::vector;
using std::cout;
using std::endl;

int main(void) {

    Logger *logger = new Logger("output/", "expand06_", true);

    Graph G;
    Node_SP u0 = Node::allocate();
    u0->setDims(30, 30);
    u0->setCentre(0, 0);
    G.addNode(u0);
    Node_SP u1 = Node::allocate();
    u1->setDims(30, 30);
    u1->setCentre(100, 0);
    G.addNode(u1);
    Edge_SP e0 = Edge::allocate(u0, u1);
    G.addEdge(e0);

#define twoD
#ifdef twoD
    Node_SP u2 = Node::allocate();
    u2->setDims(30, 30);
    u2->setCentre(100, 100);
    G.addNode(u2);
    Node_SP u3 = Node::allocate();
    u3->setDims(30, 30);
    u3->setCentre(0, 100);
    G.addNode(u3);
    Edge_SP e1 = Edge::allocate(u1, u2);
    Edge_SP e2 = Edge::allocate(u2, u3);
    Edge_SP e3 = Edge::allocate(u3, u0);
    G.addEdge(e1);
    G.addEdge(e2);
    G.addEdge(e3);
#endif

    SepMatrix &m = G.getSepMatrix();

#define BG
#ifdef BG
    m.setCardinalOP(0, 1, CardinalDir::EAST);
#ifdef twoD
    m.setCardinalOP(1, 2, CardinalDir::SOUTH);
    m.setCardinalOP(2, 3, CardinalDir::WEST);
    m.setCardinalOP(3, 0, CardinalDir::NORTH);
#endif
#else
    m.hAlign(0, 1);
#ifdef twoD
    m.hAlign(2, 3);
    m.vAlign(1, 2);
    m.vAlign(0, 3);
#endif
#endif

#define EBG
#ifdef EBG
    m.setExtraBdryGap(20);
#endif

    logger->log(G, "00_initial");

    ColaOptions colaOpts;
    colaOpts.preventOverlaps = true;
    colaOpts.solidifyAlignedEdges = true;
    colaOpts.logger = logger;
    for (int i = 1; i < 3; ++i) {
        logger->nextLoggingIndex = i;
        G.makeFeasible(colaOpts);
        logger->log(G, string_format("%02d_make_feasible_result.tglf", i));
    }

    return 0;
}
