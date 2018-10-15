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

#include "libdialect/commontypes.h"
#include "libdialect/io.h"
#include "libdialect/graphs.h"

using namespace dialect;

using std::cout;
using std::endl;

int main(void) {

    ColaOptions opts;

    // CFDL
    cout << "Destress metro/sydney using CFDL..." << endl;
    Graph_SP graph = buildGraphFromTglfFile("graphs/" "metro/sydney.tglf");
    graph->destress(opts);
    writeStringToFile(graph->writeTglf(), "output/" "destress_" "metro_sydney_cfdl.tglf");

    // CML
    cout << "Destress metro/sydney using CML..." << endl;
    Graph_SP graph2 = buildGraphFromTglfFile("graphs/" "metro/sydney.tglf");
    opts.useMajorization = true;
    graph2->destress(opts);
    writeStringToFile(graph2->writeTglf(), "output/" "destress_" "metro_sydney_cml.tglf");

    return 0;
}
