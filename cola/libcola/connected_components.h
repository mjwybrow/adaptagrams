/*
 * vim: ts=4 sw=4 et tw=0 wm=0
 *
 * libcola - A library providing force-directed network layout using the 
 *           stress-majorization method subject to separation constraints.
 *
 * Copyright (C) 2006-2008  Monash University
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
*/

#ifndef CONNECTED_COMPONENTS_H
#define CONNECTED_COMPONENTS_H
#include "libcola/cola.h"
#include <vector>

namespace cola {
// a graph component with a list of node_ids giving indices for some larger list of nodes
// for the nodes in this component, 
// and a list of edges - node indices relative to this component
class Component {
public:
    std::vector<unsigned> node_ids;
    std::vector<vpsc::Rectangle*> rects;
    std::vector<cola::Edge> edges;
    //CompoundConstraints cx, cy;
    ~Component();
    void moveRectangles(double x, double y);
    vpsc::Rectangle* getBoundingBox();
};
// for a graph of n nodes, return connected components
void connectedComponents(
    const std::vector<vpsc::Rectangle*> &rs,
    const std::vector<cola::Edge> &es,
    //const CompoundConstraints &cx,
    //const CompoundConstraints &cy, 
    std::vector<Component*> &components);

// move the contents of each component so that the components do not
// overlap.
void separateComponents(const std::vector<Component*> &components);

} // namespace cola

#endif // CONNECTED_COMPONENTS_H
