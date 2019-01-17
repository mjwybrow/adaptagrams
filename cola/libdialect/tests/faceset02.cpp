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
#include <iostream>
#include <map>
#include <utility>
#include <set>

#include "libvpsc/assertions.h"

#include "libdialect/commontypes.h"
#include "libdialect/io.h"
#include "libdialect/graphs.h"
#include "libdialect/util.h"
#include "libdialect/routing.h"
#include "libdialect/opts.h"
#include "libdialect/planarise.h"
#include "libdialect/faces.h"

using namespace dialect;

using std::vector;
using std::cout;
using std::endl;

vector<id_type> nodes2ids(Nodes nodes) {
    vector<id_type> ids;
    for (Node_SP u : nodes) ids.push_back(u->id());
    return ids;
}

int main(void) {

    // Build a 4-planar, orthogonal layout.
    Graph_SP graph = buildGraphFromTglfFile("graphs/" "special/3cross4cross.tglf");
    HolaOpts opts;
    LeaflessOrthoRouter lor(graph, opts);
    lor.setShapeBufferDistanceIELScalar(0.125);
    lor.route();
    OrthoPlanariser op(graph);
    Graph_SP P = op.planarise();
    writeStringToFile(P->writeTglf(), "output/" "faceset02_" "planar_layout.tglf");
    // Compute the face set.
    FaceSet faceSet(P);

    // Get the external face.
    Face_SP ext = faceSet.getExternalFace();

    // Display the ids in the node sequence.
    cout << "External face:" << endl << "    ";
    Nodes extNodes = ext->getNodeSeq();
    vector<id_type> extIds = nodes2ids(extNodes);
    for (id_type id : extIds) cout << id << " ";
    cout << endl;

    // We check that data structures have been properly constructed for
    // our 3-crossing (at node #4) and 4-crossing (at node #11).
    vector<id_type> crossings{4, 11};

    // Test the neighbour pairs.
    std::map<id_type, std::vector<std::pair<Node_SP, Node_SP>>> nbrPairs = ext->getNbrPairs();
    for (id_type id : crossings) {
        auto pairs = nbrPairs.at(id);
        cout << string_format("Pairs for node %d:", id) << endl;
        for (auto q : pairs) {
            cout << string_format("    %d, %d", q.first->id(), q.second->id()) << endl;
        }
    }
    // There should be the right number of them.
    COLA_ASSERT(nbrPairs[4].size() == 3);
    COLA_ASSERT(nbrPairs[11].size() == 4);

    // Show the Sides that include our crossing nodes.
    cout << "Sides including crossing Nodes 4 and 11:" << endl;
    Sides sides = ext->getSides();
    for (size_t i = 0; i < sides.size(); ++i) {
        Side_SP S = sides[i];
        if (S->containsNode(4) || S->containsNode(11)) {
            cout << i << ": " << S->toString() << endl;
        }
    }

    // Show the Nexes, and check that they determine the correct neighbours of important directions.
    std::map<id_type, vector<CompassDir>> direcsPerCrossing{
        {4,  {CompassDir::NE, CompassDir::EAST, CompassDir::SE, CompassDir::SW, CompassDir::NW}},
        {11, {CompassDir::NE, CompassDir::SE, CompassDir::SW, CompassDir::NW}}
    };
    std::map<id_type, vector<std::set<Side_SP>>> answersPerCrossing{
        {4, {
                {sides[31]}, {sides[31]}, {sides[31]}, {sides[24], sides[25]}, {sides[37], sides[38]}
            }
        },
        {11, {
                {sides[42], sides[43]}, {sides[19], sides[20]}, {sides[12], sides[13]}, {sides[3], sides[4]}
            }
        }
    };
    NexesById nexusLookup = ext->getNexusLookup();
    for (id_type id : crossings) {
        cout << "Crossing at " << id << endl;
        Nexus_SP nexus = nexusLookup.at(id);
        //cout << nexus->toString();
        vector<CompassDir> direcs = direcsPerCrossing.at(id);
        vector<std::set<Side_SP>> answers = answersPerCrossing.at(id);
        for (size_t i = 0; i < direcs.size(); ++i) {
            CompassDir d = direcs.at(i);
            std::set<Side_SP> answer = answers.at(i);
            std::set<Side_SP> sides = nexus->getNeighboursOfADirection(d);
            COLA_ASSERT(sides == answer);
            cout << "Neighbours of " << Compass::dirToString(d) << ":" << endl;
            for (Side_SP S : sides) cout << "    " << S->toString() << endl;
        }
    }

    return 0;
}
