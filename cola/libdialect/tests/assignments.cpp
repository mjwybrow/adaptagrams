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

#include <memory>
#include <iostream>
#include <vector>

#include "libvpsc/assertions.h"
#include "libdialect/commontypes.h"
#include "libdialect/util.h"
#include "libdialect/quadaction.h"

using std::cout;
using std::endl;
using std::vector;
using std::make_shared;

using namespace dialect;

int main(void) {

    // Test constructing some Assignments, and checking
    // their cardinality.

    Nbr_SP n0 = make_shared<Nbr>(0, 1, 0);
    Nbr_SP n1 = make_shared<Nbr>(1, 1, 1);
    Nbr_SP n2 = make_shared<Nbr>(2, 1, 2);
    Nbr_SP n3 = make_shared<Nbr>(3, 1, 3);

    Nbrs semis{n0, n1, nullptr, n3};

    Assignment a(semis, 0);
    COLA_ASSERT(a.cardinality() == 3);

    Assignment a2({nullptr, nullptr, n2, nullptr}, 0);
    COLA_ASSERT(a2.cardinality() == 1);

    Assignment a3 = a2.makeUnion(a);
    COLA_ASSERT(a3.cardinality() == 4);

    Assignment a4;
    COLA_ASSERT(a4.cardinality() == 0);

    // Build a Quad, then sort and compute costs.
    Quad q(0);
    q.addNbr(n1);
    q.addNbr(n2);
    q.sortAndComputeCosts();

    /*
    std::cout << "ACost: " << q.ACost << std::endl;
    std::cout << "CCost: " << q.CCost << std::endl;

    std::cout << q.ACost - 0.5 << std::endl;
    std::cout << q.CCost - 0.2 << std::endl;
    */

    COLA_ASSERT(approx_equal(q.ACost, 0.5));
    COLA_ASSERT(approx_equal(q.CCost, 0.2));

    // Ask a Quad to construct the Assignment for a given action

    Assignment qa = q.constructAssignmentForAction('B');
    COLA_ASSERT(qa.cardinality() == 2);
    COLA_ASSERT(approx_equal(qa.cost, 0.7));

    // Build an Arrangement

    // One on East semiaxis
    Nbr_SP u0 = make_shared<Nbr>(0, 1, 0);
    // One in Q0
    Nbr_SP u1 = make_shared<Nbr>(1, 1, 1);
    // Two in Q1
    Nbr_SP u2 = make_shared<Nbr>(2, -1, 2);
    Nbr_SP u3 = make_shared<Nbr>(3, -1, 1);
    // Three in Q2
    Nbr_SP u4 = make_shared<Nbr>(4, -2, -1);
    Nbr_SP u5 = make_shared<Nbr>(5, -1, -1);
    Nbr_SP u6 = make_shared<Nbr>(6, -1, -2);
    // One in Q3
    Nbr_SP u7 = make_shared<Nbr>(7, 1, -1);

    Arrangement r({u0, u1, u2, u3, u4, u5, u6, u7});

    // Check the vacancy, distribution, and reduced distribution vectors.
    vector<unsigned> vac = {1, 0, 0, 0};
    COLA_ASSERT(r.vacancy() == vac);
    vector<unsigned> d = {1, 2, 3, 1};
    COLA_ASSERT(r.dist() == d);
    vector<unsigned> rdist = {1, 2, 2, 1};
    COLA_ASSERT(r.rDist() == rdist);
    // Check the basic assignment.
    Assignment_SP ba = r.getBasicAssignment();
    COLA_ASSERT(ba->cardinality() == 1);
    COLA_ASSERT(ba->cost == 0);
    // Check the assignment for quad action CDBD
    Assignment_SP qaa = r.getAssignmentForQuadAction("CDBD");
    // cardinality:
    COLA_ASSERT(qaa->cardinality() == 4);
    // cost:
    //cout << qaa.cost << endl;
    COLA_ASSERT(approx_equal(qaa->cost, 0.9));
    // check that the expected nodes got assigned to the semiaxes:
    COLA_ASSERT(qaa->semis[0]->id == 0);
    COLA_ASSERT(qaa->semis[1]->id == 1);
    COLA_ASSERT(qaa->semis[2]->id == 4);
    COLA_ASSERT(qaa->semis[3]->id == 6);

    // Full test: Check the list of Assignments for an Arrangement.
    /* We consider a case where the most
    preferable 3-assignment can fail due to an existing alignment
    constraint, and we need to choose the best Plan B.

           v
               c   w
        u          x

    Alignments: H(c,w); V(w,x); H(u,x)

    If a and b are the angles that uc and vc make with semiaxis 2,
    respectively, then assume we have 0 < a < b < 45.
    In this case the assignment (w - u v) is the most preferred,
    but it is thwarted by the existing alignments.
    Our algorithm should successfully identify (w u v -) as the next
    best alternative.
    */
    Nbr_SP v0 = make_shared<Nbr>(0, 1, 0);
    Nbr_SP v1 = make_shared<Nbr>(1, -1, 0.5);
    Nbr_SP v2 = make_shared<Nbr>(2, -0.866, -0.5);
    Arrangement r2({v0, v1, v2});
    Assignments A = r2.computeAllAssignments();
    /*
    for (auto p : A) {
        cout << p->toString() << endl;
    }
    */
    COLA_ASSERT(A[0]->toString() == "0 - 1 2");
    COLA_ASSERT(A[1]->toString() == "0 1 2 -");

    return 0;
}
