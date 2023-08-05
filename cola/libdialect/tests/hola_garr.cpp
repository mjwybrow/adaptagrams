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

#include "libdialect/commontypes.h"
#include "libdialect/io.h"
#include "libdialect/util.h"
#include "libdialect/graphs.h"
#include "libdialect/opts.h"
#include "libdialect/logging.h"
#include "libdialect/hola.h"

using namespace dialect;

int main(void) {
    Graph_SP graph = buildGraphFromTglfFile("graphs/" "special/Garr201001.tglf");
    HolaOpts opts;
    std::string prefix = "hola_garr_";
    bool debug = false;
    if (debug) doHOLA(*graph, opts, new Logger("output/", prefix, true, true));
    else doHOLA(*graph, opts);
    writeStringToFile(graph->writeTglf(), "output/" + prefix + "layout.tglf");
    writeStringToFile(graph->writeSvg(), "output/" "svg/" + prefix + "layout.svg");
}
