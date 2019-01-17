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

#include "libvpsc/assertions.h"
#include "libdialect/io.h"
#include "libdialect/util.h"
#include "libdialect/graphs.h"

using namespace dialect;

using std::vector;

int main(void) {
    Graph graph = *buildGraphFromTglfFile("graphs/" "special/components.tglf");
    vector<Graph_SP> comps = graph.getConnComps();
    // We designed the test case with 7 components.
    // First check the raw number.
    COLA_ASSERT(comps.size() == 7);
    // And also write each component to a separate file for visual check.
    size_t i = 0;
    for (auto p : comps) {
        writeStringToFile(
            p->writeTglf(),
            string_format("output/" "conncomps_" "comp_%d.tglf", i++)
        );
    }
    return 0;
}
