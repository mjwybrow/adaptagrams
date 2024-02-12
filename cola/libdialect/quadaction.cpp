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
#include <algorithm>
#include <vector>
#include <memory>

#include "libvpsc/assertions.h"
#include "libdialect/util.h"
#include "libdialect/quadaction.h"

using std::make_shared;
using std::vector;
using std::string;
using std::sort;

using namespace dialect;

Nbr::Nbr(id_type id, double dx, double dy) :
    id(id), x(dx), y(dy) {
    // At least one of dx and dy must be nonzero, since otherwise
    // this Nbr would sit right atop the centre node, which is not
    // allowed.
    COLA_ASSERT(!(dx==0 && dy==0));
}

unsigned Nbr::octalCode() const {
    // Semiaxes 0,1,2,3 get octal codes 0,2,4,6;
    // Quadrants 0,1,2,3 get octal codes 1,3,5,7.
    return (
        x > 0 ? (y > 0 ? 1 : y == 0 ? 0 : 7) :
        x < 0 ? (y > 0 ? 3 : y == 0 ? 4 : 5) :
                (y > 0 ? 2 : 6)
    );
}

double Nbr::deflection() const {
    double x2 = x*x, y2 = y*y, l2 = x2 + y2;
    unsigned o = octalCode();
    // If we are in the clopen first or third quadrants, then the
    // squared sign of the angle from the start of the quadrant is
    // y^2 over radius^2; else the numerator is x^2 instead.
    return (o==0 || o==1 || o==4 || o==5) ? y2/l2 : x2/l2;
}

size_t Assignment::cardinality() const {
    // To compute the cardinality of the Assignment we initialise
    // a counter to zero, and increment for each non null ptr.
    size_t c = 0;
    for (auto p : semis) {
        if (p != nullptr) {
            ++c;
        }
    }
    return c;
}

Assignment Assignment::makeUnion(const Assignment &other) const {
    // If any entry in this Assignment is null, copy the entry from the other;
    // else copy the entry from this.
    // Meanwhile, costs sum.
    return Assignment(
        {
            semis[0] == nullptr ? other.semis[0] : semis[0],
            semis[1] == nullptr ? other.semis[1] : semis[1],
            semis[2] == nullptr ? other.semis[2] : semis[2],
            semis[3] == nullptr ? other.semis[3] : semis[3]
        },
        cost + other.cost
    );
}

string Assignment::toString() const {
    // E.g. if Nbrs 3, 7, 8 are assigned to semiaxes E, S, N resp.,
    // represent this as "3 7 - 8".
    string s;
    for (unsigned i = 0; i < 4; ++i) {
        if (i > 0) s += " ";
        if (semis[i] == nullptr) {
            s += "-";
        } else {
            s += string_format("%d", semis[i]->id);
        }
    }
    return s;
}

Quad::Quad(unsigned int num) : num(num) {
    // The only valid num's for quadrants are 0, 1, 2, 3.
    COLA_ASSERT(num <= 3);
}

void Quad::sortAndComputeCosts() {
    // If we have no Nbrs, then there's no sorting, and no cost.
    if (nbrs.empty()) {
        ACost = 0;
        CCost = 0;
        return;
    }
    // Sort the Nbrs by deflection
    sort(nbrs.begin(), nbrs.end(),
        [](const Nbr_SP &a, const Nbr_SP &b) -> bool {
            return a->deflection() < b->deflection();
        }
    );
    // Anticlockwise assignment cost equals deflection of first Nbr
    ACost = nbrs[0]->deflection();
    // Clockwise assignment cost equals 1 - deflection of last Nbr
    CCost = 1 - nbrs[size()-1]->deflection();
}

Assignment Quad::constructAssignmentForAction(char action) const {
    Nbrs semis{nullptr, nullptr, nullptr, nullptr};
    double cost = 0;
    // If doing anticlockwise assignment, add ACost, and assign first Nbr to
    // semiaxis of same number as this quadrant.
    if (action == 'A' || action == 'B') {
        semis[num] = nbrs[0];
        cost += ACost;
    }
    // If doing clockwise assignment, add CCost, and assign last Nbr to semiaxis
    // of number one greater than this quadrant, mod 4.
    if (action == 'C' || action == 'B') {
        semis[(num + 1) % 4] = nbrs[size()-1];
        cost += CCost;
    }
    return Assignment(semis, cost);
}

Arrangement::Arrangement(Nbrs nbrs) :
    nbrs(nbrs),
    semis({nullptr, nullptr, nullptr, nullptr}),
    quads({make_shared<Quad>(0), make_shared<Quad>(1), make_shared<Quad>(2), make_shared<Quad>(3)})
{
    // Assign Nbrs to semis and quads based on their octals.
    for (auto p : nbrs) {
        unsigned o = p->octalCode();
        if (o % 2 == 0) {
            unsigned s = o/2;
            semis[s] = p;
        } else {
            unsigned q = (o-1)/2;
            quads[q]->addNbr(p);
        }
    }
    // Now all Quads can sort and compute costs.
    for (auto p : quads) {
        p->sortAndComputeCosts();
    }
}

vector<unsigned> Arrangement::vacancy() const {
    // for each semiaxis, write 0 if vacant, 1 if occupied
    return {
        semis[0] == nullptr ? (unsigned) 0 : (unsigned) 1,
        semis[1] == nullptr ? (unsigned) 0 : (unsigned) 1,
        semis[2] == nullptr ? (unsigned) 0 : (unsigned) 1,
        semis[3] == nullptr ? (unsigned) 0 : (unsigned) 1,
    };
}

vector<unsigned> Arrangement::dist() const {
    // write the size of each quad
    return {
        (unsigned) quads[0]->size(), (unsigned) quads[1]->size(),
        (unsigned) quads[2]->size(), (unsigned) quads[3]->size()
    };
}

vector<unsigned> Arrangement::rDist() const {
    // get basic distribution vector, then reduce
    vector<unsigned> d = dist();
    return {
        d[0] > 2 ? (unsigned) 2 : d[0],
        d[1] > 2 ? (unsigned) 2 : d[1],
        d[2] > 2 ? (unsigned) 2 : d[2],
        d[3] > 2 ? (unsigned) 2 : d[3]
    };
}

Assignment_SP Arrangement::getBasicAssignment() const {
    // the basic assignment is "as is", i.e. with existing occupations,
    // and no cost
    return make_shared<Assignment>(semis, 0);
}

Assignment_SP Arrangement::getAssignmentForQuadAction(std::string quadAction) const {
    // Start with the basic assignment.
    Assignment_SP a = getBasicAssignment();
    // Then in each quadrant, try applying the corresponding character out of the
    // quad action string.
    // Unite the results.
    for (unsigned i = 0; i < 4; ++i) {
        Assignment aq = quads[i]->constructAssignmentForAction(quadAction[i]);
        a = make_shared<Assignment>(a->makeUnion(aq));
    }
    return a;
}

Assignments Arrangement::computeNAssignments(unsigned N) const {
    Assignments asgns;
    // There are only four compass directions, so N cannot exceed 4.
    if (N > 4) return asgns;
    // We need to have at least as many neighbours as the number of
    // assignments we are asked to make.
    if (nbrs.size() < N) return asgns;
    // Some semiaxes may already have Nbrs assigned to them.
    // Let n be the number of semiaxes waiting to be filled before
    // we have an N-assignment.
    vector<unsigned> vac = vacancy();
    int n = N;
    for (unsigned i = 0; i < 4; ++i) n -= vac[i];
    // If n < 0 then we already have /more/ than N semiaxes with neighbours
    // on them, so an N-assignment, strictly speaking, is not possible.
    if (n < 0) return asgns;
    // If n == 0, then there is only one possible N-assignment, namely,
    // the one that is already there.
    if (n == 0) {
        asgns.push_back(getBasicAssignment());
        return asgns;
    }
    // Otherwise n > 0.
    // Let f be the number of "free" Nbrs, i.e. those in the interiors
    // of the quadrants.
    int f = 0;
    for (unsigned i = 0; i < 4; ++i) f += quads[i]->size();
    // If f < n, then there are n > 0 semiaxes waiting to be filled before
    // we have an N-assignment, but this node has fewer than n free nbrs,
    // so there is no way to make an N-assignment.
    if (f < n) return asgns;
    // Otherwise, there are n > 0 semiaxes waiting to be filled before we
    // have an N-assignment, and this node has at least n free nbrs, so it
    // is potentially possible to make an N-assignment.
    // Get the reduced distribution.
    vector<unsigned> d = rDist();
    // Get the semiaxis codes with the right cardinality.
    vector<unsigned> codes = SEMIAXIS_SETS_BY_CARDINALITY[n];
    // Make binary code for the vacancy vector.
    unsigned a = 1;
    unsigned vacCode = 0;
    for (unsigned i = 0; i < 4; ++i) {
        vacCode += (1-vac[i])*a;
        a *= 2;
    }
    // Iterate over the codes.
    for (unsigned c : codes) {
        // If it's not a subset of the vacant semiaxes, then skip it.
        if ((c & vacCode) != c) continue;
        // Otherwise look up the quad actions.
        vector<string> quadActions = lookupQuadActions(d[0], d[1], d[2], d[3], c);
        // For each action, record an assignment.
        for (string qa : quadActions) {
            asgns.push_back(getAssignmentForQuadAction(qa));
        }
    }
    // Sort the assignments by cost.
    sort(asgns.begin(), asgns.end(),
        [](const Assignment_SP &a, const Assignment_SP &b) -> bool {
            return a->cost < b->cost;
        }
    );
    return asgns;
}

Assignments Arrangement::computeAllAssignments() const {
    // Concatenate the results for N-assignments with N equal to 4 first,
    // then 3, 2, and 1 in that order.
    Assignments a4 = computeNAssignments(4);
    Assignments a3 = computeNAssignments(3);
    Assignments a2 = computeNAssignments(2);
    Assignments a1 = computeNAssignments(1);
    Assignments a;
    a.reserve(a4.size()+a3.size()+a2.size()+a1.size());
    a.insert(a.end(), a4.cbegin(), a4.cend());
    a.insert(a.end(), a3.cbegin(), a3.cend());
    a.insert(a.end(), a2.cbegin(), a2.cend());
    a.insert(a.end(), a1.cbegin(), a1.cend());
    return a;
}
