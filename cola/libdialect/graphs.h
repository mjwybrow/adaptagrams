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

#ifndef DIALECT_GRAPHS_H
#define DIALECT_GRAPHS_H

#include <vector>
#include <cfloat>
#include <string>
#include <map>
#include <utility>
#include <stack>
#include <deque>
#include <functional>

#include "libvpsc/rectangle.h"
#include "libcola/compound_constraints.h"
#include "libcola/cluster.h"
#include "libcola/cola.h"
#include "libavoid/libavoid.h"

#include "libdialect/commontypes.h"
#include "libdialect/constraints.h"
#include "libdialect/routing.h"
#include "libdialect/ortho.h"
#include "libdialect/logging.h"

namespace dialect {

//! @brief  A bounding box, given by the extreme coordinates.
struct BoundingBox {

    //! @brief  Standard constructor.
    //!
    //! @param[in]  x  Lower bound on x-coordinates of the box.
    //! @param[in]  X  Upper bound on x-coordinates of the box.
    //! @param[in]  y  Lower bound on y-coordinates of the box.
    //! @param[in]  Y  Upper bound on y-coordinates of the box.
    BoundingBox(double x, double X, double y, double Y) :
        x(x), X(X), y(y), Y(Y) {}

    //! @brief  Default constructor.
    //!
    //! Constructs a bounding box that serves as "zero" when adding boxes together.
    //! Can be thought of as the box whose interior consists solely of the point
    //! at infinity on the Riemann Sphere.
    BoundingBox(void) :
        x(DBL_MAX), X(-DBL_MAX), y(DBL_MAX), Y(-DBL_MAX) {}

    //! @brief  Adding two bounding boxes returns the bounding box of their union.
    BoundingBox &operator+=(const BoundingBox &rhs);

    //! @brief  Write a simple representation of the bounding box.
    std::string repr(void) const;

    //! @brief  Get the width of the box.
    double w(void) const { return X - x; }

    //! @brief  Get the height of the box.
    double h(void) const { return Y - y; }

    //! @brief  Get the interval in a given dimension.
    //! @param[in] dim  The desired interval's variable dimension; thus, X if you want the box's
    //!                 horizontal interval, Y if you want its vertical interval.
    interval getInterval(vpsc::Dim dim) { return dim == vpsc::XDIM ? std::make_pair(x, X) : std::make_pair(y, Y); }

    //! @brief  Get the centre of the box.
    Avoid::Point centre(void) const { return Avoid::Point((x+X)/2.0, (y+Y)/2.0); }

    //! @brief  Build a LineSegment representing a side of the box.
    LineSegment_SP buildSideSegment(CardinalDir side) const;

    //! @brief  Compute the perimeter of the box.
    double perimeter(void) const { return 2*(w() + h()); }

    double x;
    double X;
    double y;
    double Y;
};

//! @brief  Provides a simple way to set any or all of the various
//!         optional arguments to libcola layout methods.
struct ColaOptions {
    //! Leave the ideal edge length set to zero if you want the Graph to
    //! automatically substitute its own ideal edge length.
    double idealEdgeLength = 0;
    //! Prevent overlaps between nodes?
    bool preventOverlaps = false;
    //! Solidify aligned edges? When used in conjunction with the preventOverlaps
    //! option, this can prevent overlaps between nodes and aligned edges.
    bool solidifyAlignedEdges = false;
    //! Specify edges that should not be solidified.
    EdgesById solidEdgeExemptions;
    //! Work in the x-dimension?
    bool xAxis = true;
    //! Work in the y-dimension?
    bool yAxis = true;
    //! When using a ConstrainedFDLayout, do makeFeasible before running?
    bool makeFeasible = false;
    //! The CFDL makeFeasible operation allows you to set extra border thickness
    //! on rectangles in the x- and y-dimensions. You can set those values here.
    double makeFeasible_xBorder = 0;
    double makeFeasible_yBorder = 0;
    //! Use neighbour stress? If true this means that only those stress terms
    //! will be counted for pairs of nodes connected by an edge.
    bool useNeighbourStress = false;
    //! Ordinarily we use neighbour stress in an attempt to make the layout more
    //! compact. In order to encourage this, we usually scale the IEL by a small
    //! fraction as well. The scalar can be set here.
    double nbrStressIELScalar = 1/20.0;
    //! If you want to use a ConstrainedFDLayout, leave useMajorization false;
    //! if you want to use a ConstrainedMajorizationLayout, set it true.
    bool useMajorization = false;
    //! If using a ConstrainedMajorizationLayout, say whether you want scaling.
    bool useScaling = false;
    //! Any /additional/ constraints in the form of cola::CompoundConstraints
    //! may be set here. Note that these are added to the constraints already
    //! recorded in the graph's SepMatrix.
    cola::CompoundConstraints ccs;
    //! Collection of Node lookups, in which to register clusters.
    std::vector<NodesById> nodeClusters;
    //! Just as with cola::ConstrainedFDLayout, you may set individual ideal
    //! edge lengths, as well as a custom convergence test, and a function to
    //! be called prior to each iteration.
    cola::EdgeLengths eLengths = cola::StandardEdgeLengths;
    cola::TestConvergence* doneTest = nullptr;
    cola::PreIteration* preIteration = nullptr;
    //! Optional logger.
    Logger *logger = nullptr;
};

//! @brief  Bundles those data structures required in order to represent
//!         a Graph in libcola, and to map infomration between the libcola
//!         and libdialect representations.
struct ColaGraphRep {
    vpsc::Rectangles rs;
    std::vector<cola::Edge> es;
    cola::RootCluster *rc = nullptr;
    //! A mapping from Node IDs to indices in the rs Rectangles vector:
    std::map<id_type, size_t> id2ix;
    //! The inverse mapping, from indices in the rs Rectangles vector to Node IDs:
    std::map<size_t, id_type> ix2id;
};

//! @brief  Useful for set operations on Node lookups.
//! @note  Thanks to https://stackoverflow.com/a/15579928
struct NodeIdCmp {
    bool operator()(id_type i, const std::pair<id_type, Node_SP> &p) const {
        return i < p.first;
    }
    bool operator()(const std::pair<id_type, Node_SP> &p, id_type i) const {
        return p.first < i;
    }
};

//! @brief  The Graph class represents graphs consisting of nodes and edges.
//!
class Graph {
public:
    //! @brief  Default constructor.
    Graph(void) : m_sepMatrix(this) {}

    //! @brief  Copy constructor.
    Graph(const Graph &G);

    //! @brief  Destructor
    ~Graph(void);

    //! @brief  Swap operator.
    friend void swap(Graph &first, Graph &second) {
        using std::swap;
        swap(first.m_debugOutputPath, second.m_debugOutputPath);
        swap(first.m_projectionDebugLevel, second.m_projectionDebugLevel);
        swap(first.m_sepMatrix, second.m_sepMatrix);
        swap(first.m_iel, second.m_iel);
        swap(first.m_cgr, second.m_cgr);
        swap(first.m_needNewRectangles, second.m_needNewRectangles);
        swap(first.m_cfdl, second.m_cfdl);
        swap(first.m_nodes, second.m_nodes);
        swap(first.m_edges, second.m_edges);
        swap(first.m_maxDeg, second.m_maxDeg);
    }

    //! @brief  Copy-assignment operator.
    //! @note  Pass-by-value is deliberate. See https://stackoverflow.com/a/3279550
    Graph &operator=(const Graph other);

    //! @brief  Reports the maximum degree of any Node in this Graph.
    //!
    //! The value is automatically maintained as you add or remove Nodes
    //! from the Graph.
    //!
    //! @return  Maximum degree of any Node in the Graph.
    //!
    unsigned getMaxDegree(void) const;

    //! @brief  Add a Node to this Graph.
    //!
    //! @param[in]  node  The Node to be added to the Graph.
    //! @param[in]  takeOwnership  Controls whether the Node will record this Graph
    //!                            as being the one to which it belongs.
    void addNode(Node_SP node, bool takeOwnership = true);

    //! @brief  Add a new Node to this Graph.
    //! @return  The new Node.
    Node_SP addNode(void);

    //! @brief  Add a new Node to this Graph, setting dimensions.
    //! @param[in] w  Width of Node.
    //! @param[in] h  Height of Node.
    //! @return  The new Node.
    Node_SP addNode(double w, double h);

    //! @brief  Add a new Node to this Graph, setting position and dimensions.
    //! @param[in] cx  Centre x-coordinate of Node.
    //! @param[in] cy  Centre y-coordinate of Node.
    //! @param[in] w  Width of Node.
    //! @param[in] h  Height of Node.
    //! @return  The new Node.
    Node_SP addNode(double cx, double cy, double w, double h);

    //! @brief  Add an Edge to this Graph.
    //!
    //! @param[in]  edge  The Edge to be added to the Graph.
    //! @param[in]  takeOwnership  Controls whether the Edge will record this Graph
    //!                            as being the one to which it belongs.
    void addEdge(Edge_SP edge, bool takeOwnership = true);

    //! @brief  Add a new Edge to this Graph.
    //! @param[in, out]  src  The source Node.
    //! @param[in, out]  tgt  The target Node.
    //! @note  Nodes are also informed of their linkage to this new Edge.
    //! @return  The new Edge.
    Edge_SP addEdge(Node_SP src, Node_SP tgt);

    //! @brief  Add an Edge by specifying the IDs of its endpoint Nodes.
    //!
    //! @param[in] srcID  The ID of the Node that is to sit at the source end of the new Edge.
    //! @param[in] tgtID  The ID of the Node that is to sit at the target end of the new Edge.
    //! @return  A shared pointer to the new Edge.
    Edge_SP addEdge(const id_type &srcID, const id_type &tgtID);

    //! @brief  Say whether this Graph has a Node of the given ID.
    //!
    //! @return  boolean
    bool hasNode(const id_type &id) const;

    //! @brief  Say whether this Graph has an Edge of the given ID.
    //!
    //! @return  boolean
    bool hasEdge(const id_type &id) const;

    //! @brief  Sever an Edge in this Graph.
    //!
    //! The Edge is removed from the Graph, and from both of its endpoint
    //! Nodes as well.
    //!
    //! @param[in]  edge  The Edge to be severed from the Graph.
    void severEdge(dialect::Edge &edge);

    //! @brief  Sever all the Edges incident to a Node in this Graph.
    //!
    //! @note  This method only cuts the incident Edges; it does /not/ also
    //!        remove the Node itself from the Graph.
    //!
    //! @param[in]  node  The Node whose Edges are to be severed.
    //!
    //! @sa severAndRemoveNode
    void severNode(const dialect::Node &node);

    //! @brief  Like severNode but also returns a vector of all Nodes that
    //!         were neighbours before severing.
    //!
    //! @sa  severNode
    std::vector<Node_SP> severNodeNotingNeighbours(const dialect::Node &node);

    //! @brief  Remove a Node from this Graph.
    //!
    //! @note  This method only removes the Node itself; it does not also
    //!        sever the incident Edges.
    //!
    //! @param[in]  node  The Node to be removed from the Graph.
    //!
    //! @sa severAndRemoveNode
    void removeNode(const dialect::Node &node);

    //! @brief  Remove several Nodes from this Graph.
    //!
    //! @param[in]  nodes  The Nodes to be removed from the Graph.
    //!
    //! @sa removeNode
    void removeNodes(const NodesById &nodes);

    //! @brief Convenience method to completely remove a Node from the Graph.
    //!
    //! @sa severNode
    //! @sa removeNode
    void severAndRemoveNode(const dialect::Node &node);

    //! @brief Convenience method to completely remove a Node from the Graph.
    //!
    //! @sa severNode
    //! @sa removeNode
    void severAndRemoveNode(id_type nodeID);

    //! @brief  Clone a node completely. There will be as many copies of the original
    //!         node as it had edges, and each clone will be a leaf.
    //! @param[in] id  The ID of the node to be cloned.
    //! @return Vector of new clones added to the graph.
    Nodes cloneNode(id_type id);

    //! @brief  Look up a Node by ID.
    //!
    //! @throws  out_of_range exception if there is no Node by the given ID.
    //!
    //! @return  The Node with the given ID, if present.
    Node_SP getNode(const id_type &id) const;

    //! @brief  Read-only access to this Graph's lookup map for Nodes by their ID.
    const NodesById &getNodeLookup(void) const { return m_nodes; }

    //! @brief  Build a NodesById lookup with some Nodes omitted.
    NodesById getNodeLookupWithIgnore(const Nodes &ignore) const;

    //! @brief  Build a NodesById lookup with some Nodes omitted.
    NodesById getNodeLookupWithIgnore(const NodesById &ignore) const;

    //! @brief  Read-only access to this Graph's lookup map for Edges by their ID.
    const EdgesById &getEdgeLookup(void) const { return m_edges; }

    //! @brief  Say how many Nodes there are in this Graph.
    size_t getNumNodes(void) const { return m_nodes.size(); }

    //! @brief  Say how many Edges there are in this Graph.
    size_t getNumEdges(void) const { return m_edges.size(); }

    //! @brief  Say whether the Graph is empty, meaning that it has no Nodes.
    bool isEmpty(void) const { return m_nodes.empty(); }

    //! @brief  Say whether the Graph is a tree.
    bool isTree(void) const { return getNumEdges() == getNumNodes() - 1; }

    //! @brief  Compute the average of all heights and widths of Nodes
    //!         in this Graph.
    //!
    //! @return  The average.
    double computeAvgNodeDim(void) const;

    //! @brief  Read the ideal edge length of this Graph.
    //!
    //! @note  If the current value is zero (which is true of all new Graphs), the Graph
    //!        will first auto-infer an ideal edge length equal to twice the average node
    //!        dimension, store that value, and return it.
    //!
    //! @warning  Once this method has been called, the Graph will NOT recompute its IEL
    //!           on subsequent calls, even if Nodes have been added or taken away.
    //!
    //! @sa Graph::recomputeIEL
    double getIEL(void);

    //! @brief  Recompute, store, and return the Graph's ideal edge length.
    //!
    //! @note  As in the getIEL method, the value will be twice the average dimension
    //!        of Nodes currently in the Graph.
    //!
    //! @sa Graph::getIEL
    double recomputeIEL(void);

    //! @brief  Get the bounding box for this Graph.
    //!
    //! @param[in]  ignore  Optional set of Nodes (as NodesByID) to leave out of the box.
    //! @param[in]  includeBends  Say whether bend points of connector routes
    //!                           should be included in the box.
    //!
    //! @return  The bounding box.
    //!
    //! @sa getBoundingBoxDimensions
    BoundingBox getBoundingBox(
                const NodesById &ignore = NodesById(),
                bool includeBends = false
            ) const;

    //! @brief  Get the connected components of this Graph.
    //!
    //! Note that the Nodes and Edges in the component Graphs will be
    //! the same objects as those in this Graph.
    //!
    //! @return  A vector of shared pointers to Graphs.
    std::vector<Graph_SP> getConnComps(void) const;

    //! @brief  Identify all sequences of consecutive "links" (degree-2 nodes) in this graph.
    //! @param[out] chains  Vector of deques of Nodes, where each identified "chain" will be recorded.
    //!                     A "chain" is a consecutive sequence of "links" whose endpoints are distinct.
    //! @param[out] cycles  Vector of deques of Nodes, where each identified "cycle" will be recorded.
    //!                     A "cycles" is a consecutive sequence of "links" that forms a closed loop.
    void getChainsAndCycles(std::vector<std::deque<Node_SP>> &chains, std::vector<std::deque<Node_SP>> &cycles);

    //! @brief  Write TGLF to represent this Graph.
    //! @param[in] useExternalIds  When a Graph is built from TGLF its Nodes store the IDs that
    //!                            were used there. Set true if you want these same IDs to be
    //!                            written out as well. Otherwise the internal unique Node IDs are used.
    //! @return  A string containing the TGLF.
    std::string writeTglf(bool useExternalIds = false) const;

    //! @brief  Write SVG to represent this Graph.
    //! @param[in] useExternalIds  When a Graph is built from TGLF its Nodes store the IDs that
    //!                            were used there. Set true if you want these same IDs to be
    //!                            written out as well. Otherwise the internal unique Node IDs are used.
    //! @return  A string containing the SVG.
    std::string writeSvg(bool useExternalIds = false) const;

    //! @brief  Write the Node ID --> Rectangle Index map.
    //!         Useful for debugging.
    //! @return  String representation of the map.
    std::string writeId2Ix(void) const;

    //! @brief  Write the Rectangle Index --> Node ID map.
    //!         Useful for debugging.
    //! @return  String representation of the map.
    std::string writeIx2Id(void) const;

    //! @brief  Rotate the layout -- and the constraints -- 90 degrees clockwise.
    //! @param[in] opts  ColaOptions to control destressing operation after rotation.
    //!                  This is optional; pass nullptr if you do not want to destress.
    //! @note If any nodes are non-square, i.e. have aspect ratio different from 1:1, then destressing
    //!       is recommended after a 90-degree rotation. The relationships between oblong nodes can
    //!       change dramatically when rotating 90 degrees.
    //! @sa Graph::rotate90acw
    //! @sa Graph::rotate180
    void rotate90cw(ColaOptions *opts=nullptr);

    //! @brief  Rotate the layout -- and the constraints -- 90 degrees anticlockwise.
    //! @param[in] opts  ColaOptions to control destressing operation after rotation.
    //!                  This is optional; pass nullptr if you do not want to destress.
    //! @note If any nodes are non-square, i.e. have aspect ratio different from 1:1, then destressing
    //!       is recommended after a 90-degree rotation. The relationships between oblong nodes can
    //!       change dramatically when rotating 90 degrees.
    //! @sa Graph::rotate90cw
    //! @sa Graph::rotate180
    void rotate90acw(ColaOptions *opts=nullptr);

    //! @brief  Rotate the layout -- and the constraints -- 180 degrees.
    //! @note Unlinke the 90-degree rotations, rotating 180 degrees can be viewed as a simple rigid
    //!       transformation, and there is no reason to destress afterward.
    //! @sa Graph::rotate90cw
    //! @sa Graph::rotate90acw
    void rotate180(void);

    //! @brief  Translate the entire layout by a given amount in each dimension.
    //! @param[in] dx  The amount by which to translate in the x-dimension.
    //! @param[in] dy  The amount by which to translate in the y-dimension.
    void translate(double dx, double dy);

    //! @brief  Put the Graph into a basic position useful for making unit test inputs.
    //!         The Nodes are put in a row, all Edge routes are cleared, and all constraints
    //!         are cleared.
    //!
    //!         The criteria are: (1) all nodes have distinct positions, and (2) the
    //!         layout is a bad one. Condition (1) is needed so that Cola doesn't generate
    //!         random starting positions.
    void putInBasePosition(void);

    //! @brief  Check whether this Graph has the same layout as another, up to a given
    //!         tolerance.
    //! @note   Checks Node positions and Edge routes, but not constraints.
    //! @param[in] other  The other Graph.
    //! @param[in] tol  Tolerance for checking equality of floats.
    //! @param[in] idMap  Optional mapping from the IDs of the other Graph to the IDs of
    //!                   the corresponding Nodes of this Graph. If not provided, the
    //!                   correspondence is by the Nodes' external IDs.
    bool hasSameLayoutAs(const Graph &other, double tol=0.001, id_map *idMap = nullptr) const;

    //! @brief  Get a lookup for the Edges of this Graph by the IDs of their source and
    //!         target Nodes, in that order.
    SparseIdMatrix2d<Edge_SP>::type getEdgeBySrcIdTgtIdLookup(void) const;

    //! @brief  Reduce stress via libcola's gradient-descent procedures.
    //!
    //! @param[in]  opts  options to control the layout.
    //!
    //! @note  In opts, set the idealEdgeLength to 0 if you want the Graph to automatically
    //!        supply a reasonable ideal edge length.
    void destress(const ColaOptions &opts);

    //! @brief  Convenience method to destress with default options.
    void destress(void);

    //! @brief  Add Nodes to represent aligned Edges in one dimension, constraining them to stay aligned.
    //! @param[in] dim  Solidify only those aligned Edges whose variable coordinate is in this dimension.
    //!                 Thus, horizontally aligned edges for XDIM; vertically aligned for YDIM.
    //! @param[in] opts  Here you can set Edge exemptions, i.e. a set of Edges that should not be solidified.
    void solidifyAlignedEdges(vpsc::Dim dim, const ColaOptions &opts);

    //! @brief  Make feasible. This means that, among those constraints that offer alternatives,
    //!         we look for satisfiable alternatives in order of increasing cost (cost = separation violation).
    //! This is useful with nonoverlap constraints.
    //!
    //! @param[in] opts  The usual ColaOptions.
    //! @note  Simply uses the method by the same name in the cola::ConstrainedFDLayout class.
    void makeFeasible(const ColaOptions &opts);

    //! @brief  Project onto cola constraints.
    //! @param[in] opts  Options, including any additional constraints onto which to project
    //!                  (in addition to the Graph's existing SepMatrix).
    //! @note            All options relating to edge length are ignored, since this is only a projection.
    //! @param[in] dim  The dimension in which to project.
    //! @param[in] accept  Acceptance level. See doctext for cola::projectOntoCCs.
    //! @return  Error level. See cola::projectOntoCCs.
    int project(const ColaOptions &opts, vpsc::Dim dim, int accept=0);

    //! @brief  Convenience method to project onto a single SepCo object.
    //! Apart from the SepCo, parameters and return value are as for the ordinary project method.
    //! @sa Graph::project.
    int projectOntoSepCo(const ColaOptions &opts, SepCo_SP sepco, int accept=0);

    //! @brief  Attempt to apply the projections given by a ProjSeq object.
    //!         Give up as soon as any of them fails.
    //! @param[in] opts  Options.
    //! @param[in] ps  The ProjSeq to be applied.
    //! @param[in] accept  Acceptance level. See doctext for cola::projectOntoCCs.
    //!
    //! @note  In addition to the zero idealEdgeLength convention employed by the destress
    //!        method, here a negative idealEdgeLength in the opts parameter may be used
    //!        to indicate that you do not want the stress changes induced by the projections
    //!        to be evaluated. Otherwise they will be.
    //! @return boolean, saying whether all the projections were successful, at the given
    //!         acceptance level.
    bool applyProjSeq(const ColaOptions &opts, ProjSeq &ps, int accept=0);


    //! @brief  For use with various layout actions, this method asks the Graph
    //!         to update Node positions based on its internal vpsc Rectangles that
    //!         were used in the layout operation.
    //! @param[in] xAxis  Set true iff the x-coordinates of the nodes should be updated.
    //! @param[in] yAxis  Set true iff the y-coordinates of the nodes should be updated.
    void updateNodesFromRects(bool xAxis=true, bool yAxis=true);

    //! @brief  Refresh, as needed, the data structures necessary for applying the
    //!         methods of libcola to this Graph.
    //!
    //! @warning  If Nodes have been added to or removed from the Graph since the
    //!           last time this method was called, the old Rectangles will be
    //!           deleted.
    //!
    //!           Clients are therefore advised to utilise methods like Graph::destress
    //!           instead of creating their own instances of ConstrainedFDLayout. At the
    //!           least, they must not reuse layout objects that were created before
    //!           adding or removing Nodes from the Graph (which makes sense anyway).
    //!
    //! @return  A reference to the (updated) ColaGraphRep.
    ColaGraphRep &updateColaGraphRep(void);

    //! @brief  Build a cola::RootCluster based on the node clusters specified in a ColaOptions
    //!         object.
    //! @note  The RootCluster will also be stored in the Graph's ColaGraphRep.
    //! @warning The old RootCluster stored in the Graph's ColaGraphRep (if any) will be deleted.
    //! @return The RootCluster.
    cola::RootCluster *buildRootCluster(const ColaOptions &opts);

    //! @brief  Access the cola graph rep for this Graph.
    ColaGraphRep &getColaGraphRep(void) { return m_cgr; }

    //! @brief  Access the separation matrix for this Graph.
    SepMatrix &getSepMatrix(void) { return m_sepMatrix; }

    //! @brief  Build and return Nodes representing every point at which any Edge
    //!         has a bend in its connector route. Importantly, for any given point
    //!         in the plane, at most one Node will built to represent that point,
    //!         even if different edges have a bend there. While perhaps counterintuitive,
    //!         this is most helpful in the operation of planarising a given Graph.
    //!
    //! @note  The new Nodes are /not/ added to the Graph.
    //!
    //! @note  As an important side effect, the sequence of Nodes representing
    //!        the bend points of each Edge is set as the "bend nodes" of that Edge.
    NodesById buildUniqueBendPoints(void);

    //! @brief  Save node positions on internal stack.
    void pushNodePositions(void);

    //! @brief  Restore node positions from internal stack.
    //! @return  true if positions were restored; false if stack was empty
    bool popNodePositions(void);

    //! @brief  Set the edge thickness.
    void setEdgeThickness(double t) { m_edge_thickness = t; }

    //! @brief  Get the edge thickness.
    double getEdgeThickness(void) { return m_edge_thickness; }

    //! @brief  Add padding to all ndoes.
    void padAllNodes(double dw, double dh);

    //! @brief  Update positions of Nodes in a given Graph to equal those of the
    //!         corresponding Nodes (same ID) in this Graph.
    //! @param[out] H  The Graph whose Node positions are to be updated.
    void setPosesInCorrespNodes(Graph &H);

    //! @brief  Add padding to those Nodes in a given Graph that
    //!         correspond to Nodes (same ID) in this Graph.
    //! @param[out] H  The Graph whose corresp. Nodes are to be padded.
    //! @param[in] dw  width padding
    //! @param[in] dh  height padding
    //! @param[in] ignore  Nodes in *this* graph (not H) that should be skipped.
    void padCorrespNodes(Graph &H, double dw, double dh, const NodesById &ignore = {});

    //! @brief  Update routes of Edges in a given Graph to equal those of the
    //!         corresponding Edges (same source and target) in this Graph.
    //! @param[out] H  The Graph whose Edge routes are to be updated.
    //! @param[in] directed  Set true if Edges are to be understood as directed, i.e. if
    //!                      in order to match the Edges have to have the same source and
    //!                      the same target. Otherwise only the set {source ID, target ID}
    //!                      has to be the same. Default: false (i.e. undirected edges).
    void setRoutesInCorrespEdges(Graph &H, bool directed=false);

    //! @brief  Do a libavoid connector routing on all Edges in the Graph.
    //! @param[in] routingType  The type of routing you want (orthogonal or polyline).
    void route(Avoid::RouterFlag routingType);

    //! @brief  Clear all Edge routes.
    void clearAllRoutes(void);

    //! @brief  Ask all Edges to build their routes based on their bend nodes.
    void buildRoutes(void);

    //! @brief Add all Nodes, and all those Edges having no bend nodes within them, to a given
    //!        RoutingAdapter. This is useful when precisely those Edges are viewed as needing
    //!        a route which do not already have any bend nodes.
    //! @param[out] ra  The RoutingAdapter to which the Nodes and Edges are to be added.
    void addBendlessSubnetworkToRoutingAdapter(RoutingAdapter &ra);

    //! @brief  Clear all constraints in this Graph's SepMatrix.
    void clearAllConstraints(void);

    //! @brief  Set corresponding constraints in another Graph.
    //!         This means that for each constraint between nodes of IDs id1 and id2 in this
    //!         Graph's SepMatrix, we set that constraint in the other Graph if and only if it too
    //!         contains Nodes of IDs id1 and id2.
    //! @param[out] H  The other Graph.
    void setCorrespondingConstraints(Graph &H);

    //! @brief  Apply a transformation to a closed subset of all Nodes.
    //!
    //! @param[in]  tf  the transformation to be performed
    //! @param[in]  ids  the set of IDs of all Nodes to which the transformation
    //!                  should be applied. /Both/ Nodes must be in the set.
    //! @sa transformOpenSubset
    void transformClosedSubset(SepTransform tf, const std::set<id_type> &ids);
    
    //! @brief  Apply a transformation to an open subset of all Nodes.
    //!
    //! @param[in]  tf  the transformation to be performed
    //! @param[in]  ids  the set of IDs of all Nodes to which the transformation
    //!                  should be applied. /At least one/ Node must be in the set.
    //! @sa transformClosedSubset
    void transformOpenSubset(SepTransform tf, const std::set<id_type> &ids);

    // For debugging:
    std::string m_debugOutputPath = "";
    unsigned m_projectionDebugLevel = 0;

private:
    SepMatrix m_sepMatrix;

    //! Ideal edge length
    double m_iel = 0;

    //! For some operations we represent aligned edges by (narrow) dummy nodes.
    //! Here we can set the thickness of those nodes.
    double m_edge_thickness = 10;

    //! @brief  Automatically pick a good ideal edge length.
    //!
    //! @note  The chosen length will be twice the average node dimension.
    //!
    //! @note  Besides being returned, the chosen ideal edge length is
    //!        also stored in the Graph.
    //!
    //! @return  The chosen ideal edge length.
    double autoInferIEL(void);

    //! Sometimes we need to recompute the max degree, as when a Graph is
    //! modified or constructed programmatically in certain ways.
    void recomputeMaxDegree(void);

    //! Compute the stress of the current position.
    double computeStress(void);

    //! Common implementation for the two directional 90-degree rotation methods.
    void rotate90(PlaneMap nodeMap, std::function<void(Edge_SP)> edgeMap, SepTransform st, ColaOptions *opts=nullptr);

    //! For building ConstrainedFDLayout objects we keep a ColaGraphRep.
    ColaGraphRep m_cgr;
    //! Keep track of whether the set of Nodes has changed since last time we
    //! computed Rectangles for the CGR.
    bool m_needNewRectangles = true;
    //! We also keep a ConstrainedFDLayout, for use in computing stress.
    cola::ConstrainedFDLayout *m_cfdl = nullptr;

    //! Lookup table for Nodes by ID:
    NodesById m_nodes;
    //! Lookup table for Edges by ID:
    EdgesById m_edges;
    //! We keep track of the maximum degree of any Node in this Graph:
    unsigned m_maxDeg = 0;

    //! Node position stack, for saving positions while attempting
    //! possible layouts.
    std::stack<std::map<id_type, Avoid::Point>> m_posStack;

};

//! @brief  The Node class represents nodes in a graph.
//!
class Node {
public:
    //! @brief  Factory function, to get a shared pointer to a Node
    //!         allocated on the heap. We make the constructors protected,
    //!         in order to ensure that Nodes always come with a control block.
    //! @return  A shared_ptr to Node (Node_SP).
    static Node_SP allocate(void);

    //! @brief  Convenience factory function to set dimensions.
    static Node_SP allocate(double w, double h);

    //! @brief  Convenience factory function to set position and dimensions.
    static Node_SP allocate(double cx, double dy, double w, double h);

    //! @brief  Allocate a GhostNode of this Node.
    GhostNode_SP makeGhost(void) const;

    //! @brief  Copy-assignment operator
    dialect::Node &operator=(const dialect::Node&) = default;

    //! @brief  Destructor
    virtual ~Node(void) = default;

    //! @brief  Access the unique ID of a given instance.
    //!
    //! @return  The ID.
    virtual id_type id(void) const { return m_ID; }

    //! @brief  Check the degree (number of incident Edges) of the Node.
    //!
    //! @return  The degree of the Node
    unsigned getDegree(void) const { return m_degree; }

    //! @brief  Tell the Node which Graph it belongs to.
    //!
    //! @param[in]  graph  The Graph to which the Node is to belong.
    void setGraph(Graph &graph) { m_graph = &graph; }

    //! @brief  Access the Graph to which the Node belongs.
    Graph *getGraph(void) { return m_graph; }

    //! @brief  Add an incident Edge.
    void addEdge(const Edge_SP &edge);

    //! @brief  Remove an incident Edge.
    void removeEdge(const dialect::Edge &edge);

    //! @brief  Read-only access to this Node's lookup map for Edges by their ID.
    const EdgesById &getEdgeLookup(void) const { return m_edges; }

    //! @brief  Get a copy of this Node's lookup map for Edges by their ID.
    EdgesById getCopyOfEdgeLookup(void) const { return m_edges; }

    //! @brief  Give this Node the same coordinates and dimensions as another.
    void copyGeometry(const dialect::Node &other);

    //! @brief  Besides copying geometry, there may be other properties we wish
    //!         to copy; in particular, properties that are suitable to be copied
    //!         by a GhostNode.
    void copyOtherGhostProperties(const dialect::Node &other);

    //! @brief  Get an ordered pair (half-width, half-height) for this Node.
    dimensions getHalfDimensions(void) const;

    //! @brief  Get an ordered pair (width, height) for this Node.
    dimensions getDimensions(void) const;

    //! @brief  Get the bounding box for this Node.
    BoundingBox getBoundingBox(void) const;

    //! @brief  Get the point on the boundary of this Node in a given direction
    //!         from its centre.
    Avoid::Point getBoundaryCompassPt(CompassDir dir) const;

    //! @brief  Set the position of the node, by setting its centre coordinates.
    void setCentre(double cx, double cy);

    //! @brief  Update the position of the node, by adding to its centre coordinates.
    void translate(double dx, double dy);

    //! @brief  Apply a mapping from libavoid Points to libavoid Points, to this Node's centre.
    void applyPlaneMap(PlaneMap map);

    //! @brief  Get the centre coordinates of the node.
    Avoid::Point getCentre(void) const;

    //! @brief  Set an externally-determined ID. (This is useful for TGLF and other
    //!         interfacing operations.)
    void setExternalId(unsigned id) { m_externalID = id; }

    //! @brief  Get the external ID.
    int getExternalId(void) { return m_externalID; }

    //! @brief  Get the neighbours of this Node.
    dialect::Nodes getNeighbours(void) const;

    //! @brief  Get the neighbours of this Node, listed in clockwise cyclic order
    //!         (assuming the usual graphics convention of x increasing to the right
    //!         and y increasing downward).
    dialect::Nodes getNeighboursCwCyclic(void) const;

    //! @brief  Get the neighbours of this Node that sit as the target
    //!         end of the connecting Edge.
    //!
    //! @return  vector of the children of this Node
    virtual dialect::Nodes getChildren(void) const;

    //! @brief  Set the dimensions of the node.
    void setDims(double w, double h);

    //! @brief  Set the bounding box of the node. This sets both the dimensions and the centre point.
    //! @param[in] x  The minimum x-coord of the box.
    //! @param[in] X  The maximum x-coord of the box.
    //! @param[in] y  The minimum y-coord of the box.
    //! @param[in] Y  The maximum y-coord of the box.
    void setBoundingBox(double x, double X, double y, double Y);

    //! @brief  Add padding to the node's dimensions.
    //! @note  Amounts can be positive or negative, thus adding or subtracting padding.
    void addPadding(double dw, double dh);

    //! @brief  Update the position of this Node to equal that of the given Rectangle.
    void updatePosnFromRect(vpsc::Rectangle *r);

    //! @brief  Update the x-coordinate of this Node to equal that of the given Rectangle.
    void updateXCoordFromRect(vpsc::Rectangle *r);

    //! @brief  Update the y-coordinate of this Node to equal that of the given Rectangle.
    void updateYCoordFromRect(vpsc::Rectangle *r);

    //! @brief  Build and return a Polygon to represent this Node in libavoid.
    Avoid::Polygon makeLibavoidPolygon(void) const;

    //! @brief  Check whether this Node has been marked as being a root. This is
    //!         useful when working with trees, and can be safely ignored when
    //!         working with other sorts of graphs.
    bool isRoot(void) const { return m_isRoot; }

    //! @brief  Say whether this Node is a root. This is useful when working with
    //!         trees, and can be safely ignored when working with other sorts of graphs.
    void setIsRoot(bool isRoot) { m_isRoot = isRoot; }

    //! @brief  Check whether this Node lies opposite a LineSegment, i.e. whether
    //!         the sides of the Node lying parallel to the segment intersect its
    //!         interval.
    //! @param seg  The LineSegment in question.
    //! @param openInterval  Boolean saying whether we should treat the interval
    //!                      in question as an open one (i.e. not including its
    //!                      endpoints). Defaults to false, so that the closed
    //!                      interval is considered by default.
    bool liesOppositeSegment(const LineSegment &seg, bool openInterval=false);

    //! @brief  Write SVG to represent this Node.
    //! @param[in] useExternalId  If true, write external ID as label; otherwise write
    //!                           internal unique ID.
    //! @return  A string containing the SVG.
    std::string writeSvg(bool useExternalId = false) const;

protected:

    //! @brief  Default constructor.
    //!
    //! Each Node has an ID number, unique among Nodes (but not necessarily
    //! distinct from ID numbers of objects of other classes).
    Node(void) : m_ID(nextID++) {}

    //! @brief  Construct with dimensions.
    Node(double w, double h) : m_ID(nextID++), m_w(w), m_h(h) {}

    //! @brief  Construct with position and dimensions.
    Node(double cx, double cy, double w, double h) : m_ID(nextID++), m_cx(cx), m_cy(cy), m_w(w), m_h(h) {}

    //! @brief  Copy constructor.
    Node(const dialect::Node&) = default;

    //! An instance's own unique ID:
    const id_type m_ID;

    //! A Node can store an ID specified externally, as by TGLF.
    //! Although external IDs are restricted to non-negative integers, we store the ID as
    //! an integer, so that we can use a negative value to indicate that none has been set.
    int m_externalID = -1;

    //! Lookup table for incident Edges by Edge's ID:
    EdgesById m_edges;

private:
    //! For class-specific generation of unique IDs:
    static id_type nextID;

    //! A Node can exist outside of any Graph; when it does belong to a Graph,
    //! we keep a pointer to it here:
    Graph *m_graph = nullptr;

    //! Lookup table for incident Edges by the ID of the Node at the opposite end:
    EdgesById m_edges_by_opp_id;

    //! Degree of the Node, i.e. number of incident edges:
    unsigned m_degree = 0;

    //! Coordinates of the centre point of the Node:
    double m_cx = 0.0;
    double m_cy = 0.0;

    //! Dimensions:
    //! We provide a reasonable non-zero default size, because our algorithms require
    //! nodes to have positive dimensions.
    //! The default size is deliberately non-square in order to better test the code,
    //! which must handle non-square nodes.
    double m_w = 100.0;
    double m_h = 60.0;

    //! We often work with trees, in which case it can be useful to be able to mark
    //! a Node as being a root. While this field is meaningless in the many cases where
    //! we work with graphs which are not trees, it can be safely ignored in those cases.
    bool m_isRoot = false;

};

//! @brief  A GhostNode represents another Node.
//!
//! When working with techniques that involve decomposing a graph into parts,
//! and then reassembling those parts, it is often useful to have a node in one
//! part represent a node in another part. For example this can be a useful way
//! to manage the nodes where the two parts intersect. The GhostNode class
//! supports this by representing a given Node.
class GhostNode : public Node {
public:

    //! @brief  Factory function.
    //!
    //! @return  A shared_ptr to GhostNode (GhostNode_SP).
    static GhostNode_SP allocate(const Node &node) { return GhostNode_SP(new GhostNode(node)); }

    //! @brief  Return an appropriate ID number.
    //!
    //! Whether the GhostNode's actual ID is returned, or that of the
    //! Node it represents, depends whether it is set to masquerade.
    virtual id_type id(void) const {
        return m_doMasquerade ? m_originalNodeId : m_ID;
    }

    //! @brief  Simple way to get the true ID of this GhostNode, even
    //!         if it is currently set to masquerade as the Node it represents.
    id_type trueID(void) const { return m_ID; }

    //! @brief  As in the Node class, get the neighbours of this Node
    //!         that sit as the target end of the connecting Edge.
    //!
    //! This method performs the same function as the overriden method
    //! in the base Node class. It is only reimplemented in order to
    //! ensure that the GhostNode's id() function is used properly.
    virtual Nodes getChildren(void) const;

    //! @brief  Say whether the GhostNode should masquerade as the original Node.
    void setMasquerade(bool doMasquerade) { m_doMasquerade = doMasquerade; }

protected:

    //! @brief  We always make a GhostNode as a copy of a plain Node.
    GhostNode(const Node &node)
      : Node(),
        m_doMasquerade(false),
        m_originalNodeId(node.id())
    {
        // We also copy the given Node's geometry.
        copyGeometry(node);
        // And copy other properties.
        copyOtherGhostProperties(node);
    }

private:

    //! @brief  Default constructor is deleted.
    GhostNode(void) = delete;

    //! @brief  Copy constructor is deleted.
    GhostNode(const GhostNode&) = delete;

    //! Sometimes you want a GhostNode to masquerade as the Node it represents by
    //! returning that Node's ID instead of its own.
    bool m_doMasquerade;

    //! As Nodes, GhostNodes get a unique ID drawn from the same ID space as all Nodes.
    //! This is their "true ID". However, they also keep a record of the ID of the
    //! proper Node they represent.
    id_type m_originalNodeId;

};


//! @brief  The Edge class represents edges in a graph.
//!
class Edge {
public:
    //! @brief  Factory function.
    //!
    //! We make all constructors private and offer a factory function
    //! instead, in order to ensure that instances are allocated on the
    //! heap, and always come with a shared ptr control block.
    //!
    //! @return  A shared_ptr to Node (Node_SP).
    static Edge_SP allocate(const Node_SP &src, const Node_SP &tgt);

    //! @brief  Copy-assignment operator
    Edge &operator=(const dialect::Edge&) = default;

    //! @brief  Destructor
    ~Edge(void) = default;

    //! @brief  Access the unique ID of this instance.
    //!
    //! @return  The ID.
    id_type id(void) const { return m_ID; }

    //! @brief  Get the opposite endpt, from a given one
    //!
    //! @param[in]  end1  A Node lying at one end of this Edge.
    //!
    //! @return  The Node_SP lying at the other end.
    Node_SP getOtherEnd(const dialect::Node &end1) const;

    //! @brief  Get read-only access to the Node at the source end of this Edge.
    Node_SP getSourceEnd(void) const { return Node_SP(m_src); }

    //! @brief  Get read-only access to the Node at the target end of this Edge.
    Node_SP getTargetEnd(void) const { return Node_SP(m_tgt); }

    //! @brief  Get a pair {srcID, tgtID} giving the IDs of the source and target Nodes.
    std::pair<id_type, id_type> getEndIds(void) const { return {Node_SP(m_src)->id(), Node_SP(m_tgt)->id()}; }

    //! @brief  Tell the Edge which Graph it belongs to.
    //!
    //! @param[in]  graph  The Graph to which the Edge is to belong.
    void setGraph(Graph &graph) { m_graph = &graph; }

    //! @brief  "Sever" this Edge, i.e. remove it from the Nodes to which
    //!         it is attached.
    void sever(void);

    //! @brief  Get the bounding box for the edge,
    //!         including its end points and route points.
    BoundingBox getBoundingBox(void) const;

    //! @brief  Add a point to the route.
    void addRoutePoint(double x, double y);

    //! @brief  Set (overwriting) the entire route.
    void setRoute(std::vector<Avoid::Point> route);

    //! @brief  Get a copy of the Edge's route member. May be empty.
    //! @note  If you are interested in drawing a connector, see the getRoutePoints method.
    //! @sa getRoutePoints
    std::vector<Avoid::Point> getRoute(void) const { return m_route; }

    //! @brief  Get route points.
    //! @note  This method returns the points that form the route you would draw for
    //!        this edge. If the m_route vector is nonempty, a copy is returned.
    //!        Otherwise you get a vector of two points: the centre of the source
    //!        Node, and the centre of the target Node.
    //! @sa getRoute
    std::vector<Avoid::Point> getRoutePoints(void) const;

    //! @brief  Write TGLF to represent the route for this Edge.
    //!
    //! @return  A string containing the TGLF.
    std::string writeRouteTglf(void) const;

    //! @brief  Build and return a pair of libavoid ConnEnds to represent
    //!         the endpoints of this Edge.
    std::pair<Avoid::ConnEnd, Avoid::ConnEnd> makeLibavoidConnEnds(
            Avoid::ConnDirFlags srcDirs=Avoid::ConnDirAll, Avoid::ConnDirFlags tgtDirs=Avoid::ConnDirAll);

    //! @brief  Set the bend nodes. These should be Nodes representing the bend points
    //!         in the Edge's route.
    void setBendNodes(Nodes bends) { m_bendNodes = bends; }

    //! @brief  Add a single bend node.
    void addBendNode(Node_SP bn) { m_bendNodes.push_back(bn); }

    //! @brief  Access the Edge's bend nodes.
    Nodes getBendNodes(void) { return m_bendNodes; }

    //! @brief  Check whether this Edge has any bend nodes.
    bool hasBendNodes(void) { return !m_bendNodes.empty(); }

    //! @brief  Rotate the connector route 90 degrees clockwise.
    void rotate90cw(void);

    //! @brief  Rotate the connector route 90 degrees anticlockwise.
    void rotate90acw(void);

    //! @brief  Rotate the connector route 180 degrees.
    void rotate180(void);

    //! @brief  Translate the connector route by a given amount in each dimension.
    //! @param[in] dx  The amount by which to translate in the x-dimension.
    //! @param[in] dy  The amount by which to translate in the y-dimension.
    void translate(double dx, double dy);

    //! @brief  Clear the connector route and drop all bend nodes.
    void clearRouteAndBends(void);

    //! @brief  Build a connector route based on the bend nodes.
    void buildRouteFromBends(void);

    //! @brief  Write SVG to represent this Edge.
    //! @return  A string containing the SVG.
    std::string writeSvg(void) const;

    //! @brief  Write the data for a polyline SVG path for this Edge's connector route.
    std::string writePolylineConnectorData(void) const;

    //! @brief  Write the data for an orthogonal SVG path for this Edge's connector route,
    //!         using rounded bends.
    std::string writeRoundedOrthoConnectorData(void) const;

private:
    //! @brief  Standard constructor.
    //!
    //! Each Edge has an ID number, unique among Edges (but not necessarily
    //! distinct from ID numbers of objects of other classes).
    //!
    Edge(const Node_SP &src, const Node_SP &tgt);

    //! @brief  No default constructor; Edges must be initialised with their
    //!         endpoint Nodes.
    Edge(void) = delete;

    //! For class-specific generation of unique IDs:
    static id_type nextID;

    //! An instance's own unique ID:
    const id_type m_ID;

    //! An Edge can exist outside of any Graph; when it does belong to a Graph,
    //! we keep a pointer to it here:
    Graph *m_graph = nullptr;

    //! Whether or not the Edge will be depicted as directed, we keep track of
    //! the two endpoints as a "source" end and "target" end ("src" and "tgt").
    //! Since Nodes keep shared pointers to all their incident Edges, we break
    //! the cycle by having Edges keep only /weak/ pointers back to their
    //! endpoints. (Otherwise we'll have ref counts that never go to zero, hence
    //! a memory leak.)
    Node_WP m_src;
    Node_WP m_tgt;

    //! Points for connector routing for the Edge:
    std::vector<Avoid::Point> m_route;

    //! Nodes to represent bends in the route.
    Nodes m_bendNodes;
};


} // namespace dialect

// Global Operators
dialect::BoundingBox operator+(const dialect::BoundingBox &lhs, const dialect::BoundingBox &rhs);
bool operator==(const dialect::BoundingBox &lhs, const dialect::BoundingBox &rhs);
bool operator!=(const dialect::BoundingBox &lhs, const dialect::BoundingBox &rhs);

#endif // DIALECT_GRAPHS_H
