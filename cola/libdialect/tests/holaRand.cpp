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

#include <string>
#include <vector>
#include <iostream>
#include <chrono>

#include "libdialect/commontypes.h"
#include "libdialect/io.h"
#include "libdialect/util.h"
#include "libdialect/graphs.h"
#include "libdialect/opts.h"
#include "libdialect/logging.h"
#include "libdialect/hola.h"

using namespace dialect;

using std::vector;
using std::string;

int main(void) {

    // Thanks to https://stackoverflow.com/a/12231232 for timing code.

    HolaOpts opts;
    string *debugName = nullptr;
    //debugName = new string("v100e120");
    for (int j = 0; j < 4; ++j) {
        for (int i = 1; i <= 10; ++i) {
            std::string name = string_format("v%de%d", 10*i, (10 + j)*i);
            if (debugName != nullptr && name != *debugName) continue;
            std::cout << name << ": ";
            Graph_SP graph = buildGraphFromTglfFile("graphs/" "random/"+name+".tglf");
            if (debugName != nullptr && name==*debugName) doHOLA(*graph, opts, new Logger("output/", "holaRand_", true));
            else {
                auto t1 = std::chrono::high_resolution_clock::now();
                doHOLA(*graph, opts);
                auto t2 = std::chrono::high_resolution_clock::now();
                auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(t2-t1).count();
                std::cout << string_format("%10d ms", ms) << std::endl;
            }
            writeStringToFile(graph->writeTglf(), "output/" "holaRand_" +name+".tglf");
        }
    }

}
