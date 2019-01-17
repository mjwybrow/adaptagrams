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

#include "libvpsc/assertions.h"

#include "libdialect/commontypes.h"
#include "libdialect/io.h"
#include "libdialect/util.h"
#include "libdialect/graphs.h"

using namespace dialect;

int main(void) {

    Graph_SP graph1 = buildGraphFromTglfFile("graphs/" "special/cmplayout01.tglf");
    Graph_SP graph1b = buildGraphFromTglfFile("graphs/" "special/cmplayout01.tglf");
    Graph_SP graph2 = buildGraphFromTglfFile("graphs/" "special/cmplayout02.tglf");
    Graph_SP graph3 = buildGraphFromTglfFile("graphs/" "special/cmplayout03.tglf");
    Graph_SP graph4 = buildGraphFromTglfFile("graphs/" "special/cmplayout04.tglf");

    COLA_ASSERT(graph1->hasSameLayoutAs(*graph1b));
    COLA_ASSERT(!graph1->hasSameLayoutAs(*graph2));
    COLA_ASSERT(!graph1->hasSameLayoutAs(*graph3));
    COLA_ASSERT(!graph1->hasSameLayoutAs(*graph4));

}
