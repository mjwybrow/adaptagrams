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

#ifndef DIALECT_NODECONFIG_H
#define DIALECT_NODECONFIG_H

#include "libdialect/commontypes.h"
#include "libdialect/graphs.h"
#include "libdialect/util.h"
#include "libdialect/aca.h"
#include "libdialect/quadaction.h"
#include "libdialect/logging.h"

namespace dialect {

//! @brief  Options to control OrthoHubLayout
struct OrthoHubLayoutOptions {
    //! If three nodes u, v, w in a graph form a triangle -- i.e. a subgraph isomorphic
    //! to K3 -- then we may wish to prevent the "flattening"
    //! of this triangle, i.e. the configuration of any one of the three nodes in such
    //! a way that the other two are assigned to opposite compass directions, e.g. assigning
    //! u and w to be north and south, resp., of v. To prevent this, set this option to true.
    //! 
    //! As an example of why you might want to set this option to false, consider that then
    //! the graph K4 gets a planar layout. When used in combination with routing by libavoid,
    //! flat triangles are not always bad.
    bool avoidFlatTriangles = true;

    //! By default, we attempt to configure only Nodes of degree 3 or higher.
    //! The idea is that nodes of degree 2 (a.k.a. "links") be handled by a different
    //! procedure, such as the ACA algorithm.
    //! However, good results can also be obtained by performing ortho hub layout on
    //! all nodes, of any degree. If that is desired, set the includeLinks option to true.
    bool includeLinks = false;
};

//! @brief  A layout object that tries to orthogonalise hubs.
//!         This means it visits nodes of degrees 3 or higher, and
//!         tries to set their neighbours in cardinal compass directions
//!         from it.
class OrthoHubLayout {
public:
    //! @brief  Standard constructor
    //!
    //! @param[in, out]  G  the underlying Graph
    //! @param[in]  opts  options to control the layout
    OrthoHubLayout(Graph_SP G, OrthoHubLayoutOptions opts);

    //! @brief  Try to orthogonalise hubs.
    //!
    //! @param[in] logger  Optional logger to record the individual layout steps.
    void layout(Logger *logger=nullptr);

private:

    bool makesFlatTriangle(const Assignment_SP &asgn);
    Assignments getAssignmentsForNode(const Node_SP &node) const;

    Graph_SP m_graph;
    OrthoHubLayoutOptions m_opts;
    ColaGraphRep &m_cgr;
    ColaOptions m_colaOpts;
    ACALayout m_aca;
    Nodes m_hubs;
    // We can use a dense data structure, Matrix2d, for the edge lookup, because
    // edges name rectangles by the latter's index in the rectangles vector. Therefore if the size of
    // the matrix equals that of the rectangles vector, there is always a place for every index.
    Matrix2d<size_t> m_edgeLookup;
    // On the other hand, we must use a sparse data structure for the node adjacency matrix, since
    // there can easily be a Node ID that is greater than or equal to the total number of Nodes in
    // the Graph. For example, such a situation is especially common in cases where a Graph has been
    // broken into separate pieces.
    SparseIdMatrix2d<unsigned>::type m_adjMat;
};


} // namespace dialect

#endif // DIALECT_NODECONFIG_H
