/*
 * vim: ts=4 sw=4 et tw=0 wm=0
 *
 * libcola - A library providing force-directed network layout using the 
 *           stress-majorization method subject to separation constraints.
 *
 * Copyright (C) 2006-2010  Monash University
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
 * Author(s):  Tim Dwyer
 *             Michael Wybrow
*/

#ifndef COLA_CLUSTER_H
#define COLA_CLUSTER_H

#include "libvpsc/rectangle.h"
#include "libvpsc/variable.h"

#include "commondefs.h"

namespace cola {


// A cluster defines a hierarchical partitioning over the nodes
// which should be kept disjoint by the layout somehow
class Cluster
{
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
        void createVars(const vpsc::Dim dim, const vpsc::Rectangles& rs,
                vpsc::Variables& vars);
        void setRectBuffers(const double buffer);
        virtual void printCreationCode(FILE *fp) const = 0;
        vpsc::Variable *vXMin, *vXMax, *vYMin, *vYMax;
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
        
        // This will be the id of the left/bottom boundary, 
        // and the right/top will be clusterVarId + 1.
        unsigned clusterVarId; 

        // This is padding used for rectangular non-overlap during 
        // makeFeasible and for rectangular clusters during optimisation.
        double rectBuffer;

    private:
        bool desiredBoundsSet;
        vpsc::Rectangle desiredBounds;

        vpsc::Variable *vMin, *vMax;
        double length;
};
typedef std::vector<Cluster*> Clusters;

class RootCluster : public Cluster 
{
    public:
        void computeBoundary(const vpsc::Rectangles& rs);
        // There are just shapes at the top level, so
        // effectively no clusters in the diagram scene.
        bool flat(void) const
        {
            return clusters.empty();
        }
        virtual void printCreationCode(FILE *fp) const;
};

class RectangularCluster : public Cluster
{
    public:
        RectangularCluster();
        ~RectangularCluster();
        void computeBoundary(const vpsc::Rectangles& rs);
        virtual void printCreationCode(FILE *fp) const;
        vpsc::Rectangle *getMinEdgeRect(const vpsc::Dim dim);
        vpsc::Rectangle *getMaxEdgeRect(const vpsc::Dim dim);

        vpsc::Rectangle *minEdgeRect[2];
        vpsc::Rectangle *maxEdgeRect[2];
};

class ConvexCluster : public Cluster
{
    public:
        void computeBoundary(const vpsc::Rectangles& rs);
        virtual void printCreationCode(FILE *fp) const;

        std::valarray<unsigned> hullRIDs;
        std::valarray<unsigned char> hullCorners;
};


} // namespace cola

#endif // COLA_CLUSTER_H
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
