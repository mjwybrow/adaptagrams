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

#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <memory>
#include <sys/stat.h>
#include <stdexcept>

#include "libvpsc/assertions.h"
#include "libdialect/commontypes.h"
#include "libdialect/graphs.h"
#include "libdialect/io.h"
#include "libdialect/constraints.h"

using namespace dialect;

using std::istream;
using std::ifstream;
using std::ofstream;
using std::string;
using std::getline;
using std::istringstream;
using std::vector;
using std::endl;

Graph_SP dialect::buildGraphFromTglf(std::string &s) {
    istringstream in(s);
    return buildGraphFromTglf(in);
}

// Simple line-based parsing:
//   https://stackoverflow.com/questions/7868936/read-file-line-by-line
//   https://stackoverflow.com/a/7868998
Graph_SP dialect::buildGraphFromTglf(istream &in) {
    Graph_SP graph = std::make_shared<Graph>();
    NodesById nodesByExternalId;
    unsigned extId;
    string line;
    unsigned state = 0;
    double cx, cy, w, h;
    unsigned i1, i2;
    char gtc, dir, rel1, rel2;
    double gap;
    Node_SP node;
    Edge_SP edge;
    while (getline(in, line)) {
        // Skip empty lines.
        if (line.empty()) continue;
        // Comment lines begin with "//". (See https://stackoverflow.com/a/40441240)
        if (line.rfind("//", 0) == 0) continue;
        // Check for "#" lines.
        if (line == "#") {
            // Time to change state.
            COLA_ASSERT(++state < 3);
            // And continue to the next line.
            continue;
        }
        // Otherwise it should be a data line.
        istringstream iss(line);
        switch (state) {
        case 0:
            // NODES
            COLA_ASSERT(iss >> extId >> cx >> cy >> w >> h);
            COLA_ASSERT(w > 0);
            COLA_ASSERT(h > 0);
            node = Node::allocate();
            node->setExternalId(extId);
            node->setCentre(cx, cy);
            node->setDims(w, h);
            graph->addNode(node);
            nodesByExternalId.insert({extId, node});
            break;
        case 1:
            // LINKS
            COLA_ASSERT(iss >> i1 >> i2);
            edge = Edge::allocate(nodesByExternalId[i1], nodesByExternalId[i2]);
            while (iss >> cx >> cy) {
                edge->addRoutePoint(cx, cy);
            }
            graph->addEdge(edge);
            break;
        case 2:
            // SEPCOS
            COLA_ASSERT(iss >> i1 >> i2 >> gtc >> dir >> rel1 >> rel2 >> gap);
            id_type j1 = nodesByExternalId[i1]->id(),
                    j2 = nodesByExternalId[i2]->id();
            GapType gt{};
            SepDir sd{};
            SepType st{};
            switch(gtc) {
                case 'B': gt=GapType::BDRY; break;
                case 'C': gt=GapType::CENTRE; break;
                default:
                    COLA_ASSERT(false);
            }
            switch(dir) {
                case 'E': sd=SepDir::EAST; break;
                case 'S': sd=SepDir::SOUTH; break;
                case 'W': sd=SepDir::WEST; break;
                case 'N': sd=SepDir::NORTH; break;
                case 'R': sd=SepDir::RIGHT; break;
                case 'D': sd=SepDir::DOWN; break;
                case 'L': sd=SepDir::LEFT; break;
                case 'U': sd=SepDir::UP; break;
                case 'X': sd=SepDir::RIGHT; break;
                case 'Y': sd=SepDir::DOWN; break;
                default:
                    COLA_ASSERT(false);
            }
            switch(rel1) {
                case '=': st=SepType::EQ; break;
                case '>': st=SepType::INEQ; break;
                default:
                    COLA_ASSERT(false);
            }
            graph->getSepMatrix().addSep(j1, j2, gt, sd, st, gap);
            break;
        }
    }
    return graph;
}

Graph_SP dialect::buildGraphFromTglfFile(const string &filepath) {
    struct stat buf;
    if (stat(filepath.c_str(), &buf) == -1) {
        const std::string msg = "File does not exist: " + filepath;
        throw std::runtime_error(msg);
    }
    ifstream infile(filepath);
    return buildGraphFromTglf(infile);
}

void dialect::writeStringToFile(const std::string &s, const std::string &filepath) {
    ofstream outfile(filepath);
    outfile << s;
    outfile.close();
}
