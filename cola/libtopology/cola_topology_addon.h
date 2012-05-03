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
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library in the file LICENSE; if not, 
 * write to the Free Software Foundation, Inc., 59 Temple Place, 
 * Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Michael Wybrow
*/

#ifndef LAYOUT_TOPOLOGY_CONNECTOR_H
#define LAYOUT_TOPOLOGY_CONNECTOR_H

#include <vector>

#include "libcola/cola.h"
#include "libtopology/topology_graph.h"

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
        ColaTopologyAddon();
        ColaTopologyAddon(std::vector<topology::Node*>& tnodes, 
                std::vector<topology::Edge*>& routes);
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
        std::vector<topology::Node*> topologyNodes;
        
        //! Topology Information: edges routes.
        std::vector<topology::Edge*> topologyRoutes;
};

}
#endif
