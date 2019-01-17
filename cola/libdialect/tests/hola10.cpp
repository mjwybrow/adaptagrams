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
#include <chrono>

#include "libdialect/commontypes.h"
#include "libdialect/io.h"
#include "libdialect/util.h"
#include "libdialect/graphs.h"
#include "libdialect/opts.h"
#include "libdialect/hola.h"

using namespace dialect;

int main(void) {

    HolaOpts opts;
    int d = 0;
    for (int i = 1; i <= 5; ++i) {
        std::string name = string_format("v%de%d", 10*i, (10 + d)*i);
        std::cout << name << ": ";
        Graph_SP graph = buildGraphFromTglfFile("graphs/" "random/"+name+".tglf");
        auto t1 = std::chrono::high_resolution_clock::now();
        doHOLA(*graph, opts);
        auto t2 = std::chrono::high_resolution_clock::now();
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(t2-t1).count();
        std::cout << string_format("%10d ms", ms) << std::endl;
        writeStringToFile(graph->writeTglf(), "output/" "hola10_" +name+".tglf");
        writeStringToFile(graph->writeSvg(), "output/" "svg/" "hola10_" +name+".svg");
    }

}
