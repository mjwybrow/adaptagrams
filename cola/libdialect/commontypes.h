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

#ifndef DIALECT_COMMONTYPES_H
#define DIALECT_COMMONTYPES_H

#include <memory>
#include <map>
#include <utility>
#include <vector>
#include <functional>

#include "libavoid/geomtypes.h"

namespace dialect {

class ACALayout;
struct Assignment;
struct ColaGraphRep;
class Edge;
struct EdgeSegment;
class ExpansionGoal;
class Face;
class FaceSet;
class GhostNode;
class Graph;
struct LineSegment;
struct Nbr;
class Nexus;
class Node;
class ProjSeq;
struct Quad;
struct SepPair;
class Side;
class Tree;
class TreePlacement;

// Shared Pointers
typedef std::shared_ptr<ACALayout> ACALayout_SP;
typedef std::shared_ptr<Assignment> Assignment_SP;
typedef std::shared_ptr<ColaGraphRep> ColaGraphRep_SP;
typedef std::shared_ptr<Edge> Edge_SP;
typedef std::shared_ptr<ExpansionGoal> ExpansionGoal_SP;
typedef std::shared_ptr<Face> Face_SP;
typedef std::shared_ptr<FaceSet> FaceSet_SP;
typedef std::shared_ptr<GhostNode> GhostNode_SP;
typedef std::shared_ptr<Graph> Graph_SP;
typedef std::shared_ptr<LineSegment> LineSegment_SP;
typedef std::shared_ptr<Nbr> Nbr_SP;
typedef std::shared_ptr<Nexus> Nexus_SP;
typedef std::shared_ptr<Node> Node_SP;
typedef std::shared_ptr<ProjSeq> ProjSeq_SP;
typedef std::shared_ptr<Quad> Quad_SP;
typedef std::shared_ptr<SepPair> SepPair_SP;
typedef std::shared_ptr<Side> Side_SP;
typedef std::shared_ptr<Tree> Tree_SP;
typedef std::shared_ptr<TreePlacement> TreePlacement_SP;

// Weak Pointers
typedef std::weak_ptr<Node> Node_WP;

// Plurals (vectors of shared pointers)
typedef std::vector<Assignment_SP> Assignments;
typedef std::vector<Edge_SP> Edges;
typedef std::vector<ExpansionGoal_SP> ExpansionGoals;
typedef std::vector<Face_SP> Faces;
typedef std::vector<Graph_SP> Graphs;
typedef std::vector<LineSegment_SP> LineSegments;
typedef std::vector<Nbr_SP> Nbrs;
typedef std::vector<Nexus_SP> Nexes;
typedef std::vector<Node_SP> Nodes;
typedef std::vector<Quad_SP> Quads;
typedef std::vector<Side_SP> Sides;
typedef std::vector<Tree_SP> Trees;
typedef std::vector<TreePlacement_SP> TreePlacements;

// Organisation by IDs
typedef unsigned id_type;

typedef std::map<id_type, Node_SP> NodesById;
typedef std::map<id_type, Edge_SP> EdgesById;
typedef std::map<id_type, Face_SP> FacesById;
typedef std::map<id_type, Nexus_SP> NexesById;

typedef std::pair<id_type, id_type> IdPair;
typedef std::pair<id_type, Node_SP> IdNodePair;
typedef std::pair<id_type, Edge_SP> IdEdgePair;

typedef std::map<id_type, id_type> id_map;

/* SparseIdMatrix2d
 *
 * Initially tried
 *     template <typename T>
 *     using SparseIdMatrix2d = std::map<id_type, std::map<id_type, T>>;
 * taking advantage of the C++11 "alias declaration". Builds fine, but SWIG complains.
 * Despite following the instructions here
 *     http://www.swig.org/Doc3.0/CPlusPlus11.html#CPlusPlus11_alias_templates
 * and adding
 *     %template(MapIdMapIdUnsigned) std::map<id_type, std::map<id_type, unsigned>>;
 *     %template() SparseIdMatrix2d<unsigned>;
 * to the interface file, SWIG still complains that SparseIdMatrix2d is not defined.
 * So we instead fall back on the C++03 hack, with thanks to https://stackoverflow.com/a/2795024.
 * Usage looks e.g. like:
 *              SparseIdMatrix2d<unsigned>::type mymatrix;
 */
template <typename T>
struct SparseIdMatrix2d
{
    typedef std::map<id_type, std::map<id_type, T>> type;
};

// Misc
typedef std::pair<double, double> dimensions;

typedef std::function<Avoid::Point(Avoid::Point)> PlaneMap;
typedef std::function<void(Avoid::Point&)> InplacePlaneMap;

} // namespace dialect

#endif // DIALECT_COMMONTYPES_H
