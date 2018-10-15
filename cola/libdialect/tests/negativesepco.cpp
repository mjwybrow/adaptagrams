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

#include <memory>
#include <iostream>

#include "libvpsc/assertions.h"
#include "libvpsc/rectangle.h"
#include "libavoid/geomtypes.h"

#include "libdialect/commontypes.h"
#include "libdialect/io.h"
#include "libdialect/graphs.h"
#include "libdialect/util.h"
#include "libdialect/constraints.h"
#include "libdialect/logging.h"

using namespace dialect;

using Avoid::Point;

int main(void) {

    Logger *logger = new Logger("output/", "negativesepco_");

    Graph G;
    Node_SP a = Node::allocate();
    a->setDims(30, 30);
    a->setCentre(0, 0);
    G.addNode(a);
    Node_SP b = Node::allocate();
    b->setDims(30, 30);
    b->setCentre(200, 100);
    G.addNode(b);

    logger->log(G, "00_initial");

    SepCo_SP sc = std::make_shared<SepCo>(vpsc::XDIM, b, a, -100);
    ColaOptions opts;
    G.projectOntoSepCo(opts, sc);

    logger->log(G, "01_projection");

    Point p = a->getCentre(),
          q = b->getCentre();

    double dx = q.x - p.x;
    std::cout << dx << std::endl;
    COLA_ASSERT(approx_equal(dx, 100));

}
