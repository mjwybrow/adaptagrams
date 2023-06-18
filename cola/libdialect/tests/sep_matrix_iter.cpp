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
#include "libdialect/constraints.h"
#include "libdialect/graphs.h"
#include "libdialect/io.h"

using namespace dialect;

std::string expected_closed = R">>>(0 0 0 5 5
1 10 10 5 5
2 20 20 5 5
3 30 30 5 5
4 40 40 5 5
5 50 50 5 5
6 60 60 5 5
7 70 70 5 5
8 80 80 5 5
#
#
2 3 C E >= 10.000
2 5 C E >= 10.000
2 7 C E >= 10.000
3 5 C E >= 10.000
3 6 C S >= 10.000
7 8 C E >= 10.000
)>>>";

std::string expected_open = R">>>(0 0 0 5 5
1 10 10 5 5
2 20 20 5 5
3 30 30 5 5
4 40 40 5 5
5 50 50 5 5
6 60 60 5 5
7 70 70 5 5
8 80 80 5 5
#
#
2 3 C S >= 10.000
2 5 C E >= 10.000
2 7 C E >= 10.000
3 5 C S >= 10.000
3 6 C W >= 10.000
7 8 C E >= 10.000
)>>>";

std::string expected_corresp = R">>>(1 10 10 5 5
3 30 30 5 5
4 40 40 5 5
6 60 60 5 5
#
#
3 6 C W >= 10.000
)>>>";

std::string expected_remove = R">>>(0 0 0 5 5
2 20 20 5 5
5 50 50 5 5
7 70 70 5 5
8 80 80 5 5
#
#
2 5 C E >= 10.000
2 7 C E >= 10.000
7 8 C E >= 10.000
)>>>";

/* The `SepMatrix` class defines four operations,
 *   SepMatrix::transformClosedSubset()
 *   SepMatrix::transformOpenSubset()
 *   SepMatrix::removeNodes()
 *   SepMatrix::setCorrespondingConstraints()
 * each of which involves complicated, dual iterations over ordered sets.
 * Here we examine cases that provide complete coverage for these methods, i.e.
 * hitting all clauses within them.
 */
int main(void) {
    Graph_SP graph = buildGraphFromTglfFile("graphs/" "special/SepMatrixIter.tglf");
    std::set<id_type> ids = {1, 3, 4, 6};

    // SepMatrix::transformClosedSubset()
    // Only the constraint on (3, 6) is transformed.
    graph->transformClosedSubset(SepTransform::ROTATE90CW, ids);
    std::string result_closed = graph->writeTglf();
    writeStringToFile(result_closed, "output/" "sep_matrix_iter_" "closed.tglf");
    COLA_ASSERT(result_closed == expected_closed);

    // SepMatrix::transformOpenSubset()
    // The constraints on (3, 5), (3, 6), and (2, 3) are transformed.
    graph->transformOpenSubset(SepTransform::ROTATE90CW, ids);
    std::string result_open = graph->writeTglf();
    writeStringToFile(result_open, "output/" "sep_matrix_iter_" "open.tglf");
    COLA_ASSERT(result_open == expected_open);

    // SepMatrix::setCorrespondingConstraints
    // The second graph only has nodes {0, 1, 3, 4, 6}. So it should only get the
    // constraint on (3, 6).
    Graph_SP graph2 = std::make_shared<Graph>(*graph);
    graph2->clearAllConstraints();

    NodesById toRemove;
    toRemove.insert({0, graph2->getNode(0)});
    toRemove.insert({2, graph2->getNode(2)});
    toRemove.insert({5, graph2->getNode(5)});
    toRemove.insert({7, graph2->getNode(7)});
    toRemove.insert({8, graph2->getNode(8)});
    graph2->removeNodes(toRemove);
    
    graph->setCorrespondingConstraints(*graph2);
    std::string result_corresp = graph2->writeTglf();
    writeStringToFile(result_corresp, "output/" "sep_matrix_iter_" "corresp.tglf");
    COLA_ASSERT(result_corresp == expected_corresp);

    // SepMatrix::removeNodes()
    // First check special case that removing *no* nodes leaves the constraint set
    // unchanged.
    toRemove.clear();
    graph->removeNodes(toRemove);
    std::string result_remove_none = graph->writeTglf();
    writeStringToFile(result_remove_none, "output/" "sep_matrix_iter_" "remove_none.tglf");
    COLA_ASSERT(result_remove_none == expected_open);

    // After removing nodes {1, 3, 4, 6}, all that remains should be the
    // constraints on (2, 5), (2, 7), and (7, 8).
    toRemove.clear();
    toRemove.insert({1, graph->getNode(1)});
    toRemove.insert({3, graph->getNode(3)});
    toRemove.insert({4, graph->getNode(4)});
    toRemove.insert({6, graph->getNode(6)});
    graph->removeNodes(toRemove);
    std::string result_remove = graph->writeTglf();
    writeStringToFile(result_remove, "output/" "sep_matrix_iter_" "remove.tglf");
    COLA_ASSERT(result_remove == expected_remove);

}
