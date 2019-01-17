/*
 * vim: ts=4 sw=4 et tw=0 wm=0
 *
 * libcola - A library providing force-directed network layout using the 
 *           stress-majorization method subject to separation constraints.
 *
 * Copyright (C) 2010-2014  Monash University
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
 * Author(s):  Michael Wybrow
 *
*/

#ifndef COLA_CC_NONOVERLAPCONSTRAINTS_H
#define COLA_CC_NONOVERLAPCONSTRAINTS_H

#include <vector>

#include "libcola/cola.h"
#include "libcola/compound_constraints.h"
#include "libcola/shapepair.h"

namespace vpsc {
class Rectangle;
}

namespace cola {

class Cluster;

class OverlapShapeOffsets : public SubConstraintInfo
{
    public:
        OverlapShapeOffsets(unsigned ind, double xOffset, double yOffset,
                unsigned int group)
            : SubConstraintInfo(ind),
              cluster(nullptr),
              rectPadding(0),
              group(group)
        {
            halfDim[0] = xOffset;
            halfDim[1] = yOffset;
        }
        OverlapShapeOffsets(unsigned ind, Cluster *cluster, unsigned int group)
            : SubConstraintInfo(ind),
              cluster(cluster),
              rectPadding(cluster->margin()),
              group(group)
        {
            halfDim[0] = 0;
            halfDim[1] = 0;
        }
        OverlapShapeOffsets()
            : SubConstraintInfo(1000000),
              cluster(nullptr),
              rectPadding(0)
        {
        }
        bool usesClusterBounds(void) const
        {
            return (cluster && !cluster->clusterIsFromFixedRectangle());
        }
        void resize(double xOffset, double yOffset)
        {
            halfDim[0] = xOffset;
            halfDim[1] = yOffset;
        }
        Cluster *cluster;
        double halfDim[2];   // Half width and height values.
        Box rectPadding;  // Used for cluster padding.
        unsigned int group;
};


class ShapePairInfo
{
    public:
        ShapePairInfo(unsigned ind1, unsigned ind2, unsigned ord = 1)
            : order(ord),
              satisfied(false),
              processed(false)
        {
            COLA_ASSERT(ind1 != ind2);
            // Assign the lesser value to varIndex1.
            varIndex1 = (ind1 < ind2) ? ind1 : ind2;
            // Assign the greater value to varIndex2.
            varIndex2 = (ind1 > ind2) ? ind1 : ind2;
        }
        bool operator<(const ShapePairInfo& rhs) const
        {
            // Make sure the processed ones are at the end after sorting.
            int processedInt = processed ? 1 : 0;
            int rhsProcessedInt = rhs.processed ? 1 : 0;
            if (processedInt != rhsProcessedInt)
            {
                return processedInt < rhsProcessedInt;
            }
            // Use cluster ordering for primary sorting.
            if (order != rhs.order)
            {
                return order < rhs.order;
            }
            return overlapMax > rhs.overlapMax;
        }
        unsigned short order;
        unsigned short varIndex1;
        unsigned short varIndex2;
        bool satisfied;
        bool processed;
        double overlapMax;
};


// Stores IDs of all rectangles exempt from non-overlap constraints.
class NonOverlapConstraintExemptions {
    public:
        NonOverlapConstraintExemptions();
        void addExemptGroupOfNodes(ListOfNodeIndexes listOfNodeGroups);
        bool shapePairIsExempt(ShapePair shapePair) const;
        std::set<ShapePair> getExemptPairs(void) {return m_exempt_pairs;}

    private:
        std::set<ShapePair> m_exempt_pairs;
};

// Non-overlap constraints prevent a set of given shapes from overlapping.
class NonOverlapConstraints : public CompoundConstraint {
    public:
        NonOverlapConstraints(NonOverlapConstraintExemptions *exemptions,
                unsigned int priority = PRIORITY_NONOVERLAP);
        //! @brief Use this method to add all the shapes between which you want
        //!        to prevent overlaps.
        //! @param id     This will be used as index into both the vars and
        //!               boundingBoxes vectors when you call the
        //!               generateSeparationConstraints method.
        //! @param halfW  If you add two shapes with half-widths hwi and hwj, then
        //!               if a horizontal separation constraint is generated between
        //!               them its gap will be hwi + hwj.
        //! @param halfH  Similar to halfW.
        //! @param group  Assign a group number to this shape. A separation constraint
        //!               will be generated between two shapes only if they belong to
        //!               the same group. This is useful for clusters.
        //! @param exemptions  Optional set of IDs of shapes to be skipped, i.e. such that
        //!                    a separation constraint should /not/ be generated between
        //!                    that shape and the one being added.
        void addShape(unsigned id, double halfW, double halfH, 
                unsigned int group = 1, std::set<unsigned> exemptions = std::set<unsigned>());
        void resizeShape(unsigned id, double halfW, double halfH);
        void removeShape(unsigned id);
        void addCluster(Cluster *cluster, unsigned int group);
        void computeAndSortOverlap(vpsc::Variables vs[]);
        void markCurrSubConstraintAsActive(const bool satisfiable);
        void markAllSubConstraintsAsInactive(void);
        bool subConstraintsRemaining(void) const;
        SubConstraintAlternatives getCurrSubConstraintAlternatives(
                vpsc::Variables vs[]);
        std::string toString(void) const;
        void setClusterClusterExemptions(std::set<ShapePair> exemptions);

        void generateVariables(const vpsc::Dim dim, vpsc::Variables& vars);
        void generateSeparationConstraints(const vpsc::Dim dim, 
                vpsc::Variables& vars, vpsc::Constraints& gcs);
        //! @brief Generate separation constraints in one dimension, to ensure
        //!        nonoverlap between all shapes in that dimension.
        //! @param dim   The dimension for which to generate constraints.
        //! @param vars  The variables between which the constraints will hold.
        //! @param gcs   The generated constraints will be added to this vector.
        //! @param boundingBoxes  For those id's corresponding to added shapes
        //!      (not clusters), the rectangle boundingBox[id] will be consulted
        //!      in order to determine:
        //!      (1) whether a separation constraint is called for, i.e. whether the
        //!          shapes in question span any of the same coods in the dimension
        //!          opposite to dim;
        //!      (2) which var is left and which is right in the separation constraint.
        //!      Note however that the dimensions of the boundingBoxes do NOT determine
        //!      the gaps of the separation constraints, which are instead based on the
        //!      half-width and half-height passed when adding the shapes.
        void generateSeparationConstraints(const vpsc::Dim dim, 
                vpsc::Variables& vars, vpsc::Constraints& gcs,
                std::vector<vpsc::Rectangle*>& boundingBoxes);

    private:
        void computeOverlapForShapePairInfo(ShapePairInfo& info,
                vpsc::Variables vs[]);
        
        std::list<ShapePairInfo> pairInfoList;
        std::map<unsigned, OverlapShapeOffsets> shapeOffsets;
        bool pairInfoListSorted;
        bool initialSortCompleted;

        // Cluster variables
        size_t clusterVarStartIndex;
        size_t currClusterIndex;
        size_t clusterMode;

        NonOverlapConstraintExemptions *m_exemptions;
        std::set<ShapePair> m_cluster_cluster_exemptions;
};

} // namespace cola
#endif // COLA_CC_NONOVERLAPCONSTRAINTS
