/*
 * vim: ts=4 sw=4 et tw=0 wm=0
 *
 * libtopology - Classes used in generating and managing topology constraints.
 *
 * Copyright (C) 2012  Monash University
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
 * Author: Michael Wybrow
*/

#ifndef LAYOUT_TOPOLOGY_CONNECTOR_H
#define LAYOUT_TOPOLOGY_CONNECTOR_H

#include <vector>

#include "libcola/cola.h"
#include "libtopology/topology_graph.h"

/**
 * @namespace topology
 * @brief libtopology: Extensions for topology preservation for libcola 
 *        and libavoid libraries.
*/
namespace topology {


/** 
 *  @brief  This class can be passed to libcola to replace some functionality
 *          to provide topology preserving layout.
 *
 *  You should instantiate this class with a given set of nodes and edges to
 *  preserve and pass it to cola::ConstrainedFDLayout::setTopology().
 */
class ColaTopologyAddon : public cola::TopologyAddonInterface
{
    public:
        /**
         * @brief Constructs an empty ColaTopologyAddon instance for 
         *        collecting topology information.
         *
         * Passing libcola an empty instance of this class will cause libcola
         * to populate it with current topology information for nodes and 
         * edges when cola::ConstrainedFDLayout::makeFeasible() is run.  This
         * information is then available in the #topologyNodes and 
         * #topologyRoutes member variables.
         */
        ColaTopologyAddon();
        /**
         * @brief Constructs a ColaTopologyAddon instance with a specified
         *        set of topology information.
         *
         * Passing libcola an instance of this class with given topology 
         * information will cause libcola to generate topology preserving
         * constraints to prevent nodes from crossing edges, and for the 
         * topology of the network to be preserved during layout.
         *
         * This topology information is usually not constructed from scratch
         * but rather extracted from an empty ColaTopologyAddon instance
         * after COLA has determined a feasible layout for it.
         *
         * @param[in] tnodes  Topology information for nodes.
         * @param[in] routes  Topology information for edges.
         */
        ColaTopologyAddon(topology::Nodes& tnodes, topology::Edges& routes);

        cola::TopologyAddonInterface *clone(void) const;

        void handleResizes(const cola::Resizes& resizeList, unsigned n,
                std::valarray<double>& X, std::valarray<double>& Y, 
                cola::CompoundConstraints& ccs, 
                vpsc::Rectangles& boundingBoxes,
                cola::RootCluster* clusterHierarchy);
        void computePathLengths(unsigned short** G);
        double computeStress(void) const;
        bool useTopologySolver(void) const;
        void makeFeasible(bool generateNonOverlapConstraints, 
                vpsc::Rectangles& boundingBoxes, 
                cola::RootCluster* clusterHierarchy);
        void moveTo(const vpsc::Dim dim, vpsc::Variables& vs, 
                vpsc::Constraints& cs, std::valarray<double> &coords, 
                cola::RootCluster* clusterHierarchy);
        double applyForcesAndConstraints(cola::ConstrainedFDLayout *layout, 
                const vpsc::Dim dim, std::valarray<double>& g, 
                vpsc::Variables& vs, vpsc::Constraints& cs, 
                std::valarray<double> &coords, 
                cola::DesiredPositionsInDim& des, double oldStress);
    
        //! Topology information: node positions and sizes.
        topology::Nodes topologyNodes;
        
        //! Topology information: edges routes.
        topology::Edges topologyRoutes;
};

}
#endif
