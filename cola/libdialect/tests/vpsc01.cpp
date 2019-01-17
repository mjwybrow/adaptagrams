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

#include "libvpsc/rectangle.h"
#include "libcola/compound_constraints.h"

#include "libdialect/commontypes.h"
#include "libdialect/io.h"
#include "libdialect/graphs.h"
#include "libdialect/util.h"

using namespace dialect;

int main(void) {

    // Suppose we have two nodes, u0 and u1, and we want to constrain u1 so that its
    // right-hand boundary does not go to the right of u0's right-hand boundary.

    // For example, suppose u0 is 100 x 60, and u1 is 40 x 40.
    // Theoretically, what we are asking for is a separation constraint with a negative gap:
    //
    //          u1.x - 30 <= u0.x
    //
    // But it is another question how to implement this with VPSC.

    // Set up the two nodes.
    Graph G;
    Node_SP u0 = Node::allocate();
    u0->setDims(100, 60);
    u0->setCentre(0, 0);
    G.addNode(u0);
    Node_SP u1 = Node::allocate();
    u1->setDims(40, 40);
    u1->setCentre(70, 70);
    G.addNode(u1);
    writeStringToFile(G.writeTglf(), "output/" "vpsc01_" "00_initial_position.tglf");

    // Does VPSC support separations with negative gaps?
    cola::SeparationConstraint *sep1 = new cola::SeparationConstraint(vpsc::XDIM, 0, 1, -30);
    ColaOptions opts;
    opts.ccs.push_back(sep1);
    G.project(opts, vpsc::XDIM);
    writeStringToFile(G.writeTglf(), "output/" "vpsc01_" "01_negative_gap_sepco_proj.tglf");

    // Reset positions.
    u0->setCentre(0, 0);
    u1->setCentre(70, 70);
    writeStringToFile(G.writeTglf(), "output/" "vpsc01_" "02_reset_position.tglf");

    // Try again, using alignments and separation in combination.
    cola::AlignmentConstraint *aln0 = new cola::AlignmentConstraint(vpsc::XDIM, 50);
    aln0->addShape(0, -50);
    cola::AlignmentConstraint *aln1 = new cola::AlignmentConstraint(vpsc::XDIM, 90);
    aln1->addShape(1, -20);
    cola::SeparationConstraint *sep2 = new cola::SeparationConstraint(vpsc::XDIM, aln1, aln0, 0);
    ColaOptions opts2;
    opts2.ccs.push_back(aln0);
    opts2.ccs.push_back(aln1);
    opts2.ccs.push_back(sep2);
    G.project(opts2, vpsc::XDIM);
    writeStringToFile(G.writeTglf(), "output/" "vpsc01_" "03_separated_alignments_proj.tglf");

    // Clean up.
    delete sep1;
    delete aln0;
    delete aln1;
    delete sep2;

}
