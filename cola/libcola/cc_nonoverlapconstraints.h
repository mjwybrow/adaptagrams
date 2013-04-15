/*
 * vim: ts=4 sw=4 et tw=0 wm=0
 *
 * libcola - A library providing force-directed network layout using the 
 *           stress-majorization method subject to separation constraints.
 *
 * Copyright (C) 2010  Monash University
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
#include "libcola/compound_constraints.h"

namespace vpsc {
class Rectangle;
}

namespace cola {

class OverlapShapeOffsets;
class ShapePairInfo;
class Cluster;


// Non-overlap constraints prevent a set of given shapes from overlapping.
class NonOverlapConstraints : public CompoundConstraint {
    public:
        NonOverlapConstraints(unsigned int priority = PRIORITY_NONOVERLAP);
        // Group is used to determine which objects should be made not to 
        // overlap with this one -- only objects in the same group.
        // This is useful for clusters.
        void addShape(unsigned id, double halfW, double halfH, 
                unsigned int group = 1);
        void addCluster(Cluster *cluster, unsigned int group);
        void computeAndSortOverlap(vpsc::Variables vs[]);
        void markCurrSubConstraintAsActive(const bool satisfiable);
        void markAllSubConstraintsAsInactive(void);
        bool subConstraintsRemaining(void) const;
        SubConstraintAlternatives getCurrSubConstraintAlternatives(
                vpsc::Variables vs[]);
        
        void generateVariables(const vpsc::Dim dim, vpsc::Variables& vars);
        void generateSeparationConstraints(const vpsc::Dim dim, 
                vpsc::Variables& vars, vpsc::Constraints& gcs);
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
};

} // namespace cola
#endif // COLA_CC_NONOVERLAPCONSTRAINTS
