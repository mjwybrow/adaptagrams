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

#include "libcola/compound_constraints.h"
#include "libcola/commondefs.h"

namespace cola {


// A cluster defines a hierarchical partitioning over the nodes
// which should be kept disjoint by the layout somehow
class Cluster
{
    public:
        Cluster();
        virtual ~Cluster();
        virtual void computeBoundary(const vpsc::Rectangles& rs) = 0;
        virtual void computeBoundingRect(const vpsc::Rectangles& rs);
        vpsc::Rectangle bounds;
        void setDesiredBounds(const vpsc::Rectangle bounds);
        void unsetDesiredBounds();
        void createVars(const vpsc::Dim dim, const vpsc::Rectangles& rs,
                vpsc::Variables& vars);
        void setRectBuffers(const double buffer);
        virtual void printCreationCode(FILE *fp) const = 0;
        vpsc::Variable *vXMin, *vXMax, *vYMin, *vYMax;
        virtual bool containsShape(unsigned index) const;
        void addChildNode(unsigned index);
        void addChildCluster(Cluster *cluster);
        virtual bool clusterIsFromFixedRectangle(void) const;
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

        double varWeight;
        double internalEdgeWeightFactor;
        std::vector<unsigned> nodes;
        std::vector<Cluster*> clusters;
        std::valarray<double> hullX, hullY;
        
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
        
        //! @brief  A rectangular cluster with floating sides that contains
        //!         its children.
        RectangularCluster();
        
        //! @brief  A fixed size rectangular cluster based on a particular 
        //!         rectangle.  This rectangle might be constrained in the 
        //!         other ways like other rectangles.
        //!
        //! @param[in]  rectIndex  The index of the rectangle that this cluster
        //!                        represents.
        RectangularCluster(unsigned rectIndex);
        virtual ~RectangularCluster();
        void computeBoundary(const vpsc::Rectangles& rs);
        virtual bool containsShape(unsigned index) const;
        virtual void printCreationCode(FILE *fp) const;
        virtual void computeBoundingRect(const vpsc::Rectangles& rs);
        inline vpsc::Rectangle *getMinEdgeRect(const vpsc::Dim dim)
        {
            if (minEdgeRect[dim])
            {
                delete minEdgeRect[dim];
            }
            minEdgeRect[dim] = new vpsc::Rectangle(bounds);

            // Set the Min and Max positions to be the min minus an offset.
            double edgePosition = minEdgeRect[dim]->getMinD(dim);
            minEdgeRect[dim]->setMinD(dim, edgePosition - rectBuffer);
            minEdgeRect[dim]->setMaxD(dim, edgePosition);

            return minEdgeRect[dim];
        }

        inline vpsc::Rectangle *getMaxEdgeRect(const vpsc::Dim dim)
        {
            if (maxEdgeRect[dim])
            {
                delete maxEdgeRect[dim];
            }
            maxEdgeRect[dim] = new vpsc::Rectangle(bounds);

            // Set the Min and Max positions to be the max plus an offset.
            double edgePosition = maxEdgeRect[dim]->getMaxD(dim);
            maxEdgeRect[dim]->setMaxD(dim, edgePosition + rectBuffer);
            maxEdgeRect[dim]->setMinD(dim, edgePosition);

            return maxEdgeRect[dim];
        }
        virtual bool clusterIsFromFixedRectangle(void) const;
        int rectangleIndex(void) const;
    
        // For fixed sized clusters based on a rectangle, this method 
        // generates the constraints that attach the cluster edges to the
        // centre position of the relevant rectangle.
        void generateFixedRectangleConstraints(
                cola::CompoundConstraints& idleConstraints,
                vpsc::Rectangles& rc, vpsc::Variables (&vars)[2]) const;
    
    private:
        vpsc::Rectangle *minEdgeRect[2];
        vpsc::Rectangle *maxEdgeRect[2];
        int m_rectangle_index;
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:textwidth=99 :
