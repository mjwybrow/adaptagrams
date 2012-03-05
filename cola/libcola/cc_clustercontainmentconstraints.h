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
 * Author(s):  Michael Wybrow
 *
*/

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

};



} // namespace cola

// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=4:softtabstop=4 :

