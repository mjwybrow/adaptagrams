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
*/

#ifndef COLA_CLUSTER_H
#define COLA_CLUSTER_H

#include <libvpsc/rectangle.h>
#include <libvpsc/variable.h>
#include "commondefs.h"

namespace cola {


// A cluster defines a hierarchical partitioning over the nodes
// which should be kept disjoint by the layout somehow
class Cluster {
public:
    double varWeight;
    double internalEdgeWeightFactor;
    std::vector<unsigned> nodes;
    std::vector<Cluster*> clusters;
    std::valarray<double> hullX, hullY;
    Cluster();
    virtual ~Cluster() {}
    virtual void computeBoundary(const vpsc::Rectangles& rs) = 0;
    void computeBoundingRect(const vpsc::Rectangles& rs);
    vpsc::Rectangle bounds;
    void setDesiredBounds(const vpsc::Rectangle bounds);
    void unsetDesiredBounds();
    void createVars(const vpsc::Dim dim, const vpsc::Rectangles& rs, vpsc::Variables& vars);
    vpsc::Variable *vXMin, *vXMax, *vYMin, *vYMax;
    void generateNonOverlapConstraints(
            const vpsc::Dim dim,
            const NonOverlapConstraintsMode nonOverlapConstraints,
            const vpsc::Rectangles& rs,
            const vpsc::Variables& vars,
            vpsc::Constraints & cs);
    void clear();
    /**
     * @return the total area covered by contents of this cluster (not
     * including space between nodes/clusters)
     */
    double area(const vpsc::Rectangles& rs);
    /**
     * sets bounds based on the finalPositions of vMin and vMax.
     */
    void updateBounds(const vpsc::Dim dim);
private:
    bool desiredBoundsSet;
    vpsc::Rectangle desiredBounds;
    vpsc::Rectangle getMinRect( const vpsc::Dim dim, const vpsc::Rectangle& bounds);
    vpsc::Rectangle getMaxRect( const vpsc::Dim dim, const vpsc::Rectangle& bounds);
    vpsc::Variable *vMin, *vMax;
    double length;
    double border;
};
typedef std::vector<Cluster*> Clusters;
class RectangularCluster : public Cluster {
public:
    void computeBoundary(const vpsc::Rectangles& rs);
};
class RootCluster : public Cluster {
public:
    void computeBoundary(const vpsc::Rectangles& rs);
};
class ConvexCluster : public Cluster {
public:
    void computeBoundary(const vpsc::Rectangles& rs);
    std::valarray<unsigned> hullRIDs;
    std::valarray<unsigned char> hullCorners;
};


} // namespace cola

#endif // COLA_CLUSTER_H
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
