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

#ifndef DIALECT_QUADACTION_H
#define DIALECT_QUADACTION_H

#include <string>
#include <vector>

#include "libdialect/commontypes.h"

namespace dialect {

//! @brief  Look up legal quad actions
//!
//! @param[in]  p  = 0, 1, or 2, indicating whether there are 0, 1, or >= 2
//!                nodes in the first quadrant
//! @param[in]  q  like p, only for the second quadrant
//! @param[in]  r  like p, only for the third quadrant
//! @param[in]  s  like p, only for the fourth quadrant
//! @param[in]  c  binary coded representation of which semiaxes are to be used:
//!                 1's bit: 1 if EAST is to be used; else 0
//!                 2's bit: 1 if SOUTH is to be used; else 0
//!                 4's bit: 1 if WEST is to be used; else 0
//!                 8's bit: 1 if NORTH is to be used; else 0
//!
//! @return  vector of strings, naming the legal quad actions
//!
std::vector<std::string> lookupQuadActions(size_t p, size_t q, size_t r, size_t s, size_t c);

//! Using the binary coding for vacant semiaxes described in the doctext for
//! the lookupQuadActions function, each integer 0, 1, ..., 15 describes a subset
//! of the set of all semiaxes. It is useful to have these subset codes sorted
//! by cardinality of the set.
const std::vector<unsigned> SEMIAXIS_SETS_BY_CARDINALITY[5] = {
    {0},
    {1, 2, 4, 8},
    {3, 5, 9, 6, 10, 12},
    {14, 13, 11, 7},
    {15}
};

//! @brief  Represents a neighbouring node to a central node.
struct Nbr {

    //! @brief  Standard constructor.
    //!
    //! @param[in]  id  a unique identifier; useful for associating an
    //!             instance with the node it represents
    //! @param[in]  dx  x-displacement of this Nbr relative to central node
    //! @param[in]  dy  y-displacement of this Nbr relative to central node
    Nbr(id_type id, double dx, double dy);

    //! @return  A code number from 0 to 7, indicating where this neighbour
    //!         lies relative to the central node:
    //!             Semiaxes 0,1,2,3 get octal codes 0,2,4,6;
    //!             Quadrants 0,1,2,3 get octal codes 1,3,5,7.
    unsigned octalCode(void) const;

    //! @brief  A measure of how far into its quadrant this nbr lies, in the
    //!         clockwise direction.
    //!
    //! @return If this nbr lies in quadrant n or on semiaxis n, then return
    //!         the squared sine of the angle that this nbr makes with semiaxis n.
    double deflection(void) const;

    id_type id;
    double x;
    double y;
};

//! @brief  Represents an assignment of nbrs to semiaxes, and records the
//!         cost of this assignment.
struct Assignment {

    //! @brief  Standard constructor.
    //!
    //! @param[in]  semis  vector of four shared ptrs to Nbrs, representing
    //!                    the Nbrs assigned to the four semiaxes, starting
    //!                    with East and proceding clockwise. Should be equal
    //!                    to nullptr if no Nbr is present on a given semiaxis.
    //! @param[in]  cost  the cost of this assignment, which measures the sum
    //!                   of angular displacements of Nbrs from their original
    //!                   positions
    Assignment(const Nbrs &semis, double cost) : semis(semis), cost(cost) {}

    //! @brief  Default constructor.
    //!
    //! Constructs an "empty" Assignment, with no Nbrs and zero cost.
    Assignment(void) : semis({nullptr, nullptr, nullptr, nullptr}), cost(0) {}

    //! @brief  Return the "cardinality" of this Assignment, i.e. the number
    //!         of Nbrs assigned to semiaxes.
    size_t cardinality(void) const;

    //! @brief  Create union of two Assignments.
    //!
    //! @return  A new Assignment representing the union of this one with
    //!          the other.
    //!
    //! @note  This function assumes you never try to put more than one
    //!        Nbr on a given semiaxis. If you do, you will get an incorrect
    //!        result; an exception will not be raised.
    Assignment makeUnion(const Assignment &other) const;

    //! @brief  Write a string representation, listing the id of the Nbr
    //!         assigned to each semiaxis. For example if Nbrs 3, 7, 8 are
    //!         assigned to semiaxes E, S, N resp., represent this as "3 7 - 8".
    //!
    //! @return  string representation.
    std::string toString(void) const;

    Nbrs semis;
    double cost;
};

//! @brief  Represents a quadrant, relative to a central node.
struct Quad {

    //! @brief  Standard constructor.
    //!
    //! @param[in]  num  The number of this quadrant in {0, 1, 2, 3}
    Quad(unsigned int num);

    //! @brief  Add a neighbour.
    void addNbr(const Nbr_SP &nbr) { nbrs.push_back(nbr); }

    //! @brief  Say how many Nbrs are in this quadrant.
    size_t size(void) const { return nbrs.size(); }

    //! @brief  To be called after all nbrs have been added.
    //!         Sorts the nbrs into clockwise order, and then computes
    //!         and stores the costs associated with the clockwise and
    //!         anticlockwise actions.
    void sortAndComputeCosts(void);

    //! @brief  Construct an Assignment indicating which Nbr(s) were assigned
    //!         to which semiaxes, and representing the cost.
    //!
    //! @param[in]  action  an action code letter, in {A, B, C, D}:
    //!                         A: Anticlockwise
    //!                         B: Both
    //!                         C: Clockwise
    //!                         D: Do nothing
    //!
    //! @return  the Assignment
    Assignment constructAssignmentForAction(char action) const;

    //! The number of this quadrant, 0, 1, 2, or 3
    unsigned num;
    //! The Nbrs that lie in this quadrant
    Nbrs nbrs;
    //! Cost for anticlockwise action
    double ACost;
    //! Cost for clockwise action
    double CCost;
};

//! @brief  Represents the arrangement of all Nbrs around a centre node.
struct Arrangement {

    //! @brief  Standard constructor.
    //!
    //! @param[in]  nbrs  all the Nbrs of the (implied) centre node
    Arrangement(Nbrs nbrs);

    //! @brief  Get the "vacancy vector" for this Arrangement
    //!
    //! @return  vector of 0's and 1's, where 0 means the corresponding semiaxis
    //!          is vacant, and 1 means it is occupied
    std::vector<unsigned> vacancy(void) const;

    //! @brief  Get the "distribution vector" for this Arrangement
    //!
    //! @return  vector telling how many Nbrs are in each quadrant
    std::vector<unsigned> dist(void) const;

    //! @brief  Get the "reduced distribution vector" for this Arrangement
    //!
    //! @return  vector of 0's, 1's, and 2's telling, respectively, whether there
    //!          are zero, one, or >= 2 Nbrs are in each quadrant
    std::vector<unsigned> rDist(void) const;

    //! @brief  Get the "basic Assignment" for this Arrangement.
    //!
    //! @return  the Assignment in which no quad actions have been taken, i.e.
    //!          in which the semiaxees are just as initially defined, and in
    //!          which the cost is zero.
    Assignment_SP getBasicAssignment(void) const;

    //! @brief  Get the Assignment resulting from a given quad action.
    //!
    //! @param[in]  quadAction  a string of four letters chosen from among
    //!                     {A, B, C, D}, representing the action to be taken
    //!                     in each of the four quadrants.
    //!
    //! @return  the Assignment resulting from the quad action.
    Assignment_SP getAssignmentForQuadAction(std::string quadAction) const;

    //! @brief  Compute all possible Assignments in which precisely
    //!         N semiaxes are occupied, sorted by ascending cost.
    //!
    //! @param[in]  N  the number of semiaxes to be occupied
    //!
    //! @return  vector of Assignment_SPs, sorted by ascending cost.
    Assignments computeNAssignments(unsigned N) const;

    //! @brief  Compute all possible Assignments.
    //!
    //! @return  vector of Assignment_SPs. The ordering is such that any and
    //!          all 4-assignments come first, in order of ascending cost,
    //!          followed by any and all 3-assignments, again in order of
    //!          ascending cost, then likewise for 2- and 1-assignments.
    Assignments computeAllAssignments(void) const;

    //! All Nbrs in the Arrangement
    Nbrs nbrs;
    //! The semiaxes
    Nbrs semis;
    //! The quadrants
    Quads quads;
};


} // namespace dialect

#endif  // DIALECT_QUADACTION_H
