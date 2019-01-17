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
#include <utility>
#include <iostream>

#include "libvpsc/assertions.h"
#include "libdialect/commontypes.h"
#include "libdialect/io.h"
#include "libdialect/graphs.h"
#include "libdialect/aca.h"

using namespace dialect;

using std::string;
using std::vector;
using std::pair;
using std::cout;
using std::endl;

vector< pair<string, string> > names {
      {"special", "X_4"}
    , {"random", "v40e44"}
    , {"metro", "sydney"}
};

int main(void) {
    cout << "ACA + Destress" << endl;
    ColaOptions opts;
    //opts.useMajorization = true;
    for (auto p : names) {
        string dir = p.first, file = p.second;
        Graph_SP graph = buildGraphFromTglfFile("graphs/"+dir+"/"+file+".tglf");
        cout << dir << "/" << file << "..." << std::flush;
        graph->getIEL();
        ACALayout aca(graph);
        //aca.outputInstanceToSVG("output/" "destress_aca_"+dir+"_"+file+"_instance");
        aca.layout();
        graph->updateNodesFromRects();
        aca.updateSepMatrix();
        graph->destress(opts);
        cout << "done" << endl;
        writeStringToFile(graph->writeTglf(), "output/" "destress_aca_"+dir+"_"+file+".tglf");
    }
    return 0;
}
