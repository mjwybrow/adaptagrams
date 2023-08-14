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
#include <functional>
#include <string>
#include <iostream>
#include <cmath>

#include "libvpsc/rectangle.h"
#include "libavoid/libavoid.h"

#include "libdialect/commontypes.h"
#include "libdialect/graphs.h"
#include "libdialect/peeling.h"
#include "libdialect/trees.h"
#include "libdialect/nodeconfig.h"
#include "libdialect/aca.h"
#include "libdialect/chains.h"
#include "libdialect/routing.h"
#include "libdialect/planarise.h"
#include "libdialect/faces.h"
#include "libdialect/treeplacement.h"
#include "libdialect/nearalign.h"
#include "libdialect/logging.h"
#include "libdialect/util.h"
#include "libdialect/hola.h"

using namespace dialect;

using std::string;

void dialect::doHOLA(Graph &G) {
    HolaOpts opts;
    doHOLA(G, opts);
}

void dialect::doHOLA(Graph &G, const HolaOpts &holaOpts, Logger *logger) {

    // If there's no edges, there's nothing to do.
    if (G.getNumEdges() == 0) return;

    // Prepare logging functions in case a logger is given.
    std::function<void(Graph&, string)> log = [logger](Graph &H, string name)->void{
        if (logger!=nullptr) logger->log(H, name);
    };
    std::function<void(unsigned)> nli = [logger](unsigned ln)->void{
        if (logger != nullptr) logger->nextLoggingIndex = ln;
    };
    // Initialise a logging index.
    unsigned ln = 0;

    // We let the given graph auto-infer its own ideal edge length, based on node sizes.
    double IEL = G.getIEL();
    // Pad nodes
    double nodePadding = holaOpts.nodePaddingScalar*IEL;
    G.padAllNodes(nodePadding, nodePadding);
    // We need to dismantle the graph, so we begin by making a copy and we work on that instead.
    // We allocate this copy on the heap, and manage it with a shared ptr, since many of our tools
    // require that.
    Graph_SP Gcopy = std::make_shared<Graph>(G);
    // Clear any existing connector routes, for better logging output.
    Gcopy->clearAllRoutes();

    // Peel.
    Trees trees = peel(*Gcopy);
    // After peeling, the input graph is peeled down to its own core.
    // Ac-cor-dingly : ) we rename it...
    Graph_SP &core = Gcopy;

    log(*core, string_format("%02d_core", ln++));

    // If it's just a tree, layout and quit.
    // We recognise this case by there being exactly one tree, containing the same number of
    // nodes as the original graph.
    if (trees.size() == 1 && trees.front()->underlyingGraph()->getNumNodes() == G.getNumNodes()) {
        // Give the tree a symmetric layout.
        Tree_SP &tree = trees.front();
        tree->symmetricLayout(
            holaOpts.defaultTreeGrowthDir,
            holaOpts.treeLayoutScalar_nodeSep*IEL,
            holaOpts.treeLayoutScalar_rankSep*IEL,
            holaOpts.preferConvexTrees
        );
        // Route the edges.
        RoutingAdapter ra(Avoid::OrthogonalRouting);
        ra.router.setRoutingOption(Avoid::nudgeOrthogonalSegmentsConnectedToShapes, true);
        ra.router.setRoutingOption(Avoid::nudgeSharedPathsWithCommonEndPoint, true);
        ra.router.setRoutingParameter(Avoid::idealNudgingDistance, holaOpts.routingAbs_nudgingDistance);
        tree->addNetworkToRoutingAdapter(ra, holaOpts.wholeTreeRouting);
        ra.route();
        // Remove node padding.
        G.padAllNodes(-nodePadding, -nodePadding);
        // Set layout data in original Graph.
        tree->underlyingGraph()->setPosesInCorrespNodes(G);
        tree->underlyingGraph()->setRoutesInCorrespEdges(G);
        tree->addConstraints(G, true);
        // Done.
        return;
    }

    // Otherwise we do have a core and trees.

    // Start with a plain destress -- no constraints, no overlap prevention -- in order to begin
    // giving the nodes a reasonable distribution in the plane.
    core->destress();

    log(*core, string_format("%02d_free_destress_core", ln++));

    // Now destress again, this time removing any node overlaps.
    ColaOptions colaOpts;
    colaOpts.preventOverlaps = true;
    core->destress(colaOpts);

    log(*core, string_format("%02d_OP_destress_core", ln++));

    // Layout the hubs.
    nli(ln);
    OrthoHubLayoutOptions ohlOpts;
    ohlOpts.avoidFlatTriangles = holaOpts.orthoHubAvoidFlatTriangles;
    OrthoHubLayout ohl(core, ohlOpts);
    ohl.layout(logger);

    log(*core, string_format("%02d_core_ortho_hub", ln++));

    // Set extra gap for boundary constraints.
    core->getSepMatrix().setExtraBdryGap(IEL/2.0);

    // Dissipate any stress accumulated during ortho hub layout, aiming to regain a natrual
    // distribution for the nodes that remain unconstrained, and perhaps regain natural symmetries.
    // This time, besides just preventing overlaps between nodes, we also prevent any nodes from
    // overlapping with aligned edges.
    colaOpts.solidifyAlignedEdges = true;
    colaOpts.logger = logger;
    nli(ln);
    core->destress(colaOpts);

    log(*core, string_format("%02d_EOP_destress_core", ln++));

    // Next we lay out the links.
    // We may or may not build Chains for this process. Later we will need to know whether chains
    // were built, so the vector of Chains is declared at this scope.
    Chains chains;
    if (holaOpts.useACAforLinks) {
        // Use ACA.
        ACALayout aca(core);
        aca.createAlignments();
        // ACA is an older algorithm, from before we used Graphs.
        // For backward compatibility, it does not automatically update its Graph with the
        // positions and constraints from the layout, because it does not always /have/ a Graph.
        // So we ask it to do the update.
        aca.updateGraph();
    } else {
        // Use shape-conforming chain layout.
        chains = buildAllChainsInGraph(core);
        for (Chain_SP chain : chains) chain->takeShapeBasedConfiguration();
        // We project before destressing with edge-node overlap prevention, so that the edges of
        // the chain can be axis aligned first.
        // We do NOT want overlap prevention for the projection, because the new chain configuration
        // constraints may very well reverse one or more orthogonal orderings, and we need them to
        // be free to do that.
        colaOpts.preventOverlaps = false;
        colaOpts.solidifyAlignedEdges = false;
        core->project(colaOpts, vpsc::XDIM);
        core->project(colaOpts, vpsc::YDIM);
    }

    // Destress with overlap prevention including aligned edges.
    // At this time we also prepare for the next step, which involves connector routing.
    // To ensure the routing is possible, we ensure there is some gap between all nodes.
    // We do this by adding padding, destressing, and then removing this padding.
    colaOpts.preventOverlaps = true;
    colaOpts.solidifyAlignedEdges = true;
    nli(ln);
    double preRoutingGapIELScalar = 0.125;
    double preRoutingGap = preRoutingGapIELScalar*IEL;
    core->padAllNodes(preRoutingGap, preRoutingGap);
    core->destress(colaOpts);
    core->padAllNodes(-preRoutingGap, -preRoutingGap);
    if (holaOpts.useACAforLinks) {
        log(*core, string_format("%02d_core_link_config_ACA", ln++));
    } else {
        log(*core, string_format("%02d_core_link_config_Chains", ln++));
    }

    // Next is the phase in which we planarise the core.
    // However, we want a 4-planar orthogonal layout with no leaves for this phase, so we first
    // perform a special orthogonal connector routing, which ensures that no nodes will become
    // leaves in the planarisation. (It does this by ensuring that connectors are routed to at
    // least two distinct sides of each node.)
    LeaflessOrthoRouter lor(core, holaOpts);
    nli(ln);
    lor.route(logger);
    ++ln;

    log(*core, string_format("%02d_core_leafless_ortho_route", ln++));

    OrthoPlanariser op(core);
    Graph_SP P = op.planarise();

    log(*P, string_format("%02d_planar_graph_P", ln++));

    // Set extra gap for boundary constraints.
    P->getSepMatrix().setExtraBdryGap(IEL/2.0);
    // Destress the new planar graph P, aiming to regain possible natural symmetries.
    // But use overlap prevention so that the structure cannot change.
    // (Note that now /all/ edges are aligned, so we have total edge-node overlap prevention.)
    colaOpts.preventOverlaps = true;
    colaOpts.solidifyAlignedEdges = true;
    nli(ln);
    P->destress(colaOpts);

    log(*P, string_format("%02d_P_EOP_destress", ln++));

    // Now we want to reattach the trees, choosing faces of the planarised core in which to
    // place them.
    // First the trees need their own symmetric layout.
    unsigned lns = 0;  // initialise logging sub-index
    for (Tree_SP tree : trees) {
        tree->symmetricLayout(
            holaOpts.defaultTreeGrowthDir,
            holaOpts.treeLayoutScalar_nodeSep*IEL,
            holaOpts.treeLayoutScalar_rankSep*IEL,
            holaOpts.preferConvexTrees
        );
        log(*(tree->underlyingGraph()), string_format("%02d_%02d_symm_tree", ln, lns++));
    }

    ++ln;
    nli(ln);
    // Now we can choose faces and reattach them.
    FaceSet_SP faceSet = reattachTrees(P, trees, holaOpts, logger);
    ++ln;
    // We will need the vector of chosen tree placements.
    TreePlacements tps = faceSet->getAllTreePlacements();

    // Next we insert the actual trees back into the planar graph.
    // The trees come with buffer nodes. We build a record of those, so they can be
    // ignored where necessary.
    NodesById bufferNodes;
    EdgesById treeEdges;
    std::vector<NodesById> clustersSansBufferNodes;
    for (auto tp : tps) {
        tp->applyGeometryToTree();
        NodesById treeNodes;
        NodesById buffNodes;
        tp->insertTreeIntoGraph(*P, treeNodes, buffNodes, treeEdges);
        clustersSansBufferNodes.push_back(treeNodes);
        treeNodes.insert(buffNodes.begin(), buffNodes.end());
        bufferNodes.insert(buffNodes.begin(), buffNodes.end());
        colaOpts.nodeClusters.push_back(treeNodes);
    }

    log(*P, string_format("%02d_P_with_trees", ln++));

    // We don't need solid edges within the trees; moreover, this would cause constraint
    // conflicts since the tree nodes now belong to clusters to which their solid edges
    // would not belong.
    colaOpts.solidEdgeExemptions = treeEdges;
    // Destress using neighbour stress, in order to compactify.
    colaOpts.useNeighbourStress = true;
    // Now that we are using clusters to keep the tree nodes together, we make sure
    // we do not use majorization, since ConstrainedMajorizationLayout does not work
    // with RectangularClusters.
    colaOpts.useMajorization = false;

    nli(ln);
    P->destress(colaOpts);

    log(*P, string_format("%02d_P_nbr_destress", ln++));

    // Do near alignments.
    if (holaOpts.do_near_align) {
        AlignmentTable atab(*P, bufferNodes);
        for (size_t i = 0; i < holaOpts.align_reps; ++i) {
            doNearAlignments(*P, atab, bufferNodes, holaOpts);
            // After each attempt to add alignment constraints, destress, again using
            // neighbour stress and majorization.
            nli(ln);
            P->destress(colaOpts);
            log(*P, string_format("%02d_P_near_alignments", ln++));
        }
    }

    // Delete buffer nodes.
    P->removeNodes(bufferNodes);
    colaOpts.nodeClusters = clustersSansBufferNodes;

    // Rotate if desired.
    if (holaOpts.preferredAspectRatio != AspectRatioClass::NONE) {
        BoundingBox b = P->getBoundingBox(bufferNodes);
        double w = b.w(),
               h = b.h();
        bool scaleBySize = true;
        unsigned quarterTurnsCW = 0;
        auto counts = faceSet->getNumTreesByGrowthDir(scaleBySize);
        if ((w < h && holaOpts.preferredAspectRatio == AspectRatioClass::LANDSCAPE) ||
            (h < w && holaOpts.preferredAspectRatio == AspectRatioClass::PORTRAIT)) {
            // Need to rotate 90 degrees to get preferred aspect ratio.
            // There are two ways to do this (clockwise and anticlockwise).
            // In order to choose one, consult the preferred tree growth direction.
            // Determine how many trees grow in the two directions 90 degrees away from this one.
            CardinalDir q = holaOpts.preferredTreeGrowthDir,
                        p = Compass::cardRotateAcw90(q),
                        r = Compass::cardRotateCw90(q);
            size_t np = counts[p],
                   nr = counts[r];
            nli(ln);
            if (np >= nr) {
                // In this case rotating clockwise will put more trees in the preferred
                // growth direction.
                P->rotate90cw(&colaOpts);
                // We need to rotate the constraints in the core too, since later we're
                // going to write those into the original graph.
                core->getSepMatrix().transform(SepTransform::ROTATE90CW);
                quarterTurnsCW = 1;
            } else {
                // In this case rotating anticlockwise is preferred.
                P->rotate90acw(&colaOpts);
                core->getSepMatrix().transform(SepTransform::ROTATE90ACW);
                quarterTurnsCW = 3;
            }
            ln += 2;
        } else {
            // In this case we may rotate 180 degrees if that would put more trees in the preferred
            // growth direction.
            CardinalDir q = holaOpts.preferredTreeGrowthDir,
                        s = Compass::cardFlip(q);
            size_t nq = counts[q],
                   ns = counts[s];
            if (ns > nq) {
                P->rotate180();
                core->getSepMatrix().transform(SepTransform::ROTATE180);
                quarterTurnsCW = 2;
            }
        }
        // Update Tree growth directions as needed.
        if (quarterTurnsCW != 0) {
            for (Tree_SP tree : trees) tree->rotateGrowthDirCW(quarterTurnsCW);
        }
    }

    log(*P, string_format("%02d_P_rotation", ln++));

    // Translate if desired.
    if (holaOpts.putUlcAtOrigin) {
        NodesById ignore; // leave empty; don't ignore any nodes
        bool includeBends = true; // we want the edge routes included
        BoundingBox b = P->getBoundingBox(ignore, includeBends);
        double dx = -b.x,
               dy = -b.y;
        P->translate(dx, dy);
    }

    log(*P, string_format("%02d_P_translation", ln++));

    // At this point, we can ask the planar graph P to set node positions in the original graph G.
    // This is because it is now true that for every node u in G, there is a node v in P with v.ID == u.ID.
    // Initially, P had a GhostNode representing each Node in the core of G. But now we have also added
    // the Trees into P (by calling insertTreeIntoGraph on each TreePlacement). P may have additional nodes
    // that do not correspond to any nodes in G, but this does not matter.
    P->setPosesInCorrespNodes(G);
    // Similarly, P now holds the full set of constraints that we want to keep, so we can ask it to set
    // those into G as well.
    G.clearAllConstraints();
    core->setCorrespondingConstraints(G);
    P->setCorrespondingConstraints(G);
    // Set extra gap for boundary constraints.
    G.getSepMatrix().setExtraBdryGap(IEL/2.0);

    // Final connector routing.
    G.clearAllRoutes();

    if (chains.size() > 0) {
        // If we used Chains, then there may be AestheticBends that were set as route points for certain
        // connectors. For these we made Nodes and added them to the core graph. We then made GhostNodes of
        // these, in the planar graph P. In subsequent layout of P, the positions of these GhostNodes were updated.
        // However, the Chains themselves still retain pointers not to these GhostNodes, but to the original
        // AestheticBend nodes that were added to the core graph. Therefore before we can ask the Chains to add
        // these route points into the Edges of the original Graph G, we must ask P to update their positions.
        P->setPosesInCorrespNodes(*core);
        for (Chain_SP ch : chains) ch->addAestheticBendsToEdges();
        G.buildRoutes();
    }
    // Set up a routing adapter.
    RoutingAdapter ra(Avoid::OrthogonalRouting);
    ra.router.setRoutingOption(Avoid::nudgeOrthogonalSegmentsConnectedToShapes, true);
    ra.router.setRoutingOption(Avoid::nudgeSharedPathsWithCommonEndPoint, true);
    ra.router.setRoutingParameter(Avoid::crossingPenalty, 2*IEL);
    ra.router.setRoutingParameter(Avoid::segmentPenalty, IEL/2.0);
    ra.router.setRoutingParameter(Avoid::idealNudgingDistance, holaOpts.routingAbs_nudgingDistance);
    // Ask the core graph to add its nodes, and just those edges that do not have any bend nodes.
    // After asking G to clear all routes (remember G has the same Edges as core), these will be all and only
    // those Edges for which no Chain set any aesthetic bend.
    
    // Remove part of the node padding now, to ensure open channels for connector routing.
    double nodePaddingLayer1 = 2*preRoutingGapIELScalar*nodePadding;
    double nodePaddingLayer2 = nodePadding - nodePaddingLayer1;
    core->padAllNodes(-nodePaddingLayer1, -nodePaddingLayer1);
    
    core->addBendlessSubnetworkToRoutingAdapter(ra);
    // Ask each Tree to add its network to the router.
    for (Tree_SP tree : trees) {
        tree->underlyingGraph()->padAllNodes(-nodePaddingLayer1, -nodePaddingLayer1);
        tree->padCorrespNonRootNodes(G, -nodePaddingLayer1, -nodePaddingLayer1);
        tree->addNetworkToRoutingAdapter(ra, holaOpts.peeledTreeRouting, core);
    }
    // Do the routing.
    ra.route();
    // Again, since the Edges of core also belong to G, those routes are already set in the original graph G.
    // However the Edges of the Trees are new ones that were never in G. So we need to set those routes.
    for (Tree_SP tree : trees) {
        tree->underlyingGraph()->setRoutesInCorrespEdges(G);
    }

    // Remove remaining node padding.
    G.padAllNodes(-nodePaddingLayer2, -nodePaddingLayer2);
}
