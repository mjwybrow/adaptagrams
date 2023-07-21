/*
 * vim: ts=4 sw=4 et tw=0 wm=0
 *
 * libdialect - A library for computing DiAlEcT layouts:
 *                 D = Decompose/Distribute
 *                 A = Arrange
 *                 E = Expand/Emend
 *                 T = Transform
 *
 * Copyright (C) 2023  Monash University
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
#include <cmath>

#include "libavoid/geomtypes.h"

#include "libdialect/commontypes.h"
#include "libdialect/io.h"
#include "libdialect/util.h"
#include "libdialect/graphs.h"
#include "libdialect/opts.h"
#include "libdialect/hola.h"

using std::vector;
using Avoid::Point;

using namespace dialect;

/* In the HOLA layout we compute for the graph "graphs/random/v10e10.tglf", the
 * three directed edges (1, 0), (4, 1), (7, 1) will all leave node 1 on the same
 * side, and libavoid nudging will be employed to separate them.
 * 
 * The route points where these three edges connect to node 1 will be equal in one
 * coordinate, and differ in the other.
 * 
 * This function returns a vector of three doubles, being the coords of the connection
 * points on these three edges, in the dimension in which they differ.
 * The difference between adjacent pairs of these coordinates should equal the nuding
 * distance set in the HolaOpts.
 */
vector<double> checkCoords(Graph_SP graph) {
    EdgesById edges = graph->getEdgeLookup();

    Edge_SP e10, e41, e71;
    for (auto p : edges) {
        Edge_SP e = p.second;
        unsigned s = e->getSourceEnd()->getExternalId();
        unsigned t = e->getTargetEnd()->getExternalId();
        if (s == 1 && t == 0) {
            e10 = e;
        } else if (s == 4 && t == 1) {
            e41 = e;
        } else if (s == 7 && t == 1) {
            e71 = e;
        }
    }
    
    vector<Point> e10Route = e10->getRoute();
    vector<Point> e41Route = e41->getRoute();
    vector<Point> e71Route = e71->getRoute();
    
    Point a = e41Route.at(e41Route.size() - 1);
    Point b = e10Route.at(0);
    Point c = e71Route.at(e71Route.size() - 1);

    std::cout << a.x << ", " << a.y << std::endl;
    std::cout << b.x << ", " << b.y << std::endl;
    std::cout << c.x << ", " << c.y << std::endl;

    vector<double> xCoords = {a.x, b.x, c.x};
    vector<double> yCoords = {a.y, b.y, c.y};
    vector<double> coords = a.y == b.y ? xCoords : yCoords;
    return coords;
}

int main(void) {

    std::string name = "graphs/random/v10e10.tglf";

    // Layout with default nudging distance of 4.0 pixels:
    HolaOpts opts;
    Graph_SP graph = buildGraphFromTglfFile(name);
    doHOLA(*graph, opts);
    vector<double> coords = checkCoords(graph);
    COLA_ASSERT(std::abs(coords[0] - coords[1]) == 4.0);
    COLA_ASSERT(std::abs(coords[1] - coords[2]) == 4.0);
    writeStringToFile(graph->writeTglf(), "output/" "nudgeopt_4.tglf");
    writeStringToFile(graph->writeSvg(), "output/" "nudgeopt_4.svg");

    // Repeat same layout, but now with nudging distance set to 5.0 pixels:
    opts.routingAbs_nudgingDistance = 5.0;
    graph = buildGraphFromTglfFile(name);
    doHOLA(*graph, opts);
    coords = checkCoords(graph);
    COLA_ASSERT(std::abs(coords[0] - coords[1]) == 5.0);
    COLA_ASSERT(std::abs(coords[1] - coords[2]) == 5.0);
    writeStringToFile(graph->writeTglf(), "output/" "nudgeopt_5.tglf");
    writeStringToFile(graph->writeSvg(), "output/" "nudgeopt_5.svg");
    
}
