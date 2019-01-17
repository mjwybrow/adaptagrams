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

#ifndef DIALECT_NEARALIGN_H
#define DIALECT_NEARALIGN_H

#include <vector>
#include <memory>

#include "libdialect/constraints.h"
#include "libdialect/opts.h"
#include "libdialect/commontypes.h"

namespace dialect {

enum class AlignmentFlag {
    NONE = 0,
    HALIGN = 1,
    VALIGN = 2,
    HINFEAS = 4,
    VINFEAS = 8
};

//! Thanks to https://stackoverflow.com/a/1448478
inline AlignmentFlag operator&(AlignmentFlag a, AlignmentFlag b)
{return static_cast<AlignmentFlag>(static_cast<int>(a) & static_cast<int>(b));}

inline AlignmentFlag& operator|= (AlignmentFlag& a, AlignmentFlag b) { return (AlignmentFlag&)((int&)a |= (int)b); }

struct AlignmentTable {

    //! @brief  Construct an alignment table for a given Graph, optionally ignoring certain Nodes.
    AlignmentTable(Graph &graph, const Nodes &ignore);

    //! @brief  Construct an alignment table for a given Graph, optionally ignoring certain Nodes.
    AlignmentTable(Graph &graph, const NodesById &ignore);

    //! @brief  Check whether two nodes are aligned.
    //! @param[in] uid  The id of one of the ndoes.
    //! @param[in] vid  The id of the other node.
    //! @param[in] flag  The type of alignment to check for.
    //! @return true iff the nodes are aligned according to the given flag.
    bool areAligned(id_type uid, id_type vid, AlignmentFlag flag);

    //! @brief  Add an alignment with transitive closure.
    //! @param[in] uid  The id of one of the nodes to be aligned.
    //! @param[in] vid  The id of the other node to be aligned.
    //! @param[in] flag  The type of alignment to be recorded.
    void addAlignment(id_type uid, id_type vid, AlignmentFlag flag);

    //! @brief  Add all existing alignments for a set of Nodes, based on a given SepMatrix.
    //! @param[in] nodes  The set of nodes.
    //! @param[in] matrix  The SepMatrix.
    void addAlignments(const NodesById &nodes, const SepMatrix &matrix);

    //! @brief  Get all IDs that are aligned with a given one.
    //! @param[in] uid  The given ID.
    //! @param[in] flag  The type of alignment you are interested in.
    //! @return  Vector of IDs of all nodes -- including this one -- that are aligned with this one.
    std::vector<id_type> getAlignedIds(id_type uid, AlignmentFlag flag);

    //! @brief  Note that a particular alignment type is infeasible.
    //! @param[in] uid  The id of one of the nodes that cannot be aligned.
    //! @param[in] vid  The id of the other node that cannot be aligned.
    //! @param[in] flag  The type of alignment whose infeasibility is to be recorded.
    //!
    //! @note  Client should pass HALIGN or VALIGN, i.e. the type of alignment that is infeasible,
    //!         NOT one of the _INFEAS flags itself.
    void noteInfeasibility(id_type uid, id_type vid, AlignmentFlag flag);

    //! @brief  Check whether a given type of alignemnt has been marked infeasible.
    //! @param[in] uid  The id of one of the nodes in question.
    //! @param[in] vid  The id of the other node in question.
    //! @param[in] flag  The type of alignment in question.
    //! @return  true iff the alignment in question has been marked infeasible.
    bool isMarkedInfeasible(id_type uid, id_type vid, AlignmentFlag flag);

    //! The sparse table where all flags are stored:
    SparseIdMatrix2d<AlignmentFlag>::type state;
};


//! @brief  Report the "Manhattan distance" between two nodes.
double manhattan(Node_SP u, Node_SP v);


struct PotentialAlignment {

    //! @brief  Standard Constructor.
    PotentialAlignment(Node_SP u, Node_SP v, AlignmentFlag flag)
        : u(u),
          v(v),
          flag(flag),
          cost(manhattan(u, v)) {}

    //! @brief  Remove from linked list.
    void remove(void);

    //! @brief  Add this alignment to an AlignmentTable.
    void addToTable(AlignmentTable &table) { table.addAlignment(u->id(), v->id(), flag); }

    //! @brief  Note infeasibility of this alignment in in AlignmentTable.
    void noteInfeasibility(AlignmentTable &table) { table.noteInfeasibility(u->id(), v->id(), flag); }

    //! @brief  Write a SepCo to represent this alignment.
    SepCo_SP writeSepCo(void);

    //! @brief  Add this alignment to a Graph.
    void addToGraph(Graph &G);

    Node_SP u;
    Node_SP v;
    AlignmentFlag flag;
    double cost;
    // For managing linked list:
    bool removed = false;
    PotentialAlignment *prev = nullptr;
    PotentialAlignment *next = nullptr;
};

typedef std::vector<PotentialAlignment*> PotentialAlignments;

//! @brief  Look for nodes that are nearly aligned, and try to align them.
//!
//! @param[in, out] graph  The Graph whose nodes are to be aligned, and in whose SepMatrix alignments
//!                        should be recorded when made.
//! @param[in, out] atab  An AlignmentTable for the given Graph.
//! @param[in] opts  HolaOpts object to set parameters for the process.
//! @param[in] reattempt  Set true for a more aggressive process that reattempts alignments even after
//!                       they have been marked infeasible (in case other changes in the meantime might
//!                       have made them now feasible). Default: false.
//! @return  Number of successful alignments.
size_t doNearAlignments(dialect::Graph &graph, dialect::AlignmentTable &atab, dialect::NodesById &ignore, const dialect::HolaOpts &opts, bool reattempt=false);



} // namespace dialect

#endif // DIALECT_NEARALIGN_H
