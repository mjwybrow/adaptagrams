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

#ifndef COLA_CC_CLUSTERCONTAINMENTCONSTRAINTS_H
#define COLA_CC_CLUSTERCONTAINMENTCONSTRAINTS_H

#include <vector>
#include "libcola/compound_constraints.h"

namespace vpsc {
class Rectangle;
}

namespace cola {

class ClusterContainmentConstraints : public CompoundConstraint
{
    public:
        ClusterContainmentConstraints(Cluster *cluster, unsigned int priority,
                std::vector<vpsc::Rectangle*>& boundingBoxes);
        SubConstraintAlternatives getCurrSubConstraintAlternatives(
                vpsc::Variables vs[]);
        
        void generateVariables(const vpsc::Dim dim, vpsc::Variables& vars);
        void generateSeparationConstraints(const vpsc::Dim dim, 
                vpsc::Variables& vars, vpsc::Constraints& cs,
                std::vector<vpsc::Rectangle*>& bbs);
        std::string toString(void) const;
};


} // namespace cola
#endif // COLA_CC_CLUSTERCONTAINMENTCONSTRAINTS
