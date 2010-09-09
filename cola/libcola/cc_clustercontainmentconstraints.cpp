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

#include "cola.h"
#include "compound_constraints.h"
#include "cc_clustercontainmentconstraints.h"

using vpsc::XDIM;
using vpsc::YDIM;


namespace cola {

class ClusterShapeOffsets : public SubConstraintInfo
{
    public:
        ClusterShapeOffsets(unsigned ind, vpsc::Dim dim, double offset,
                unsigned int boundaryVar) :
            SubConstraintInfo(ind),
            offset(offset),
            dim(dim),
            boundaryVar(boundaryVar)
        {
        }
        double offset;    // half the width or height value
        vpsc::Dim dim;    // The dimension
        unsigned int boundaryVar; // The boundary var.
};


ClusterContainmentConstraints::ClusterContainmentConstraints(Cluster *cluster, 
        unsigned int priority, std::vector<vpsc::Rectangle*>& boundingBoxes)
    : CompoundConstraint(vpsc::HORIZONTAL, priority)
{
    _combineSubConstraints = true;
    for (std::vector<unsigned>::iterator curr = cluster->nodes.begin();
            curr != cluster->nodes.end(); ++curr)
    {
        unsigned id = *curr;
        double halfW = (boundingBoxes[id]->width() / 2.0) + cluster->rectBuffer;
        double halfH = (boundingBoxes[id]->height() / 2.0) + cluster->rectBuffer;
        _subConstraintInfo.push_back(new ClusterShapeOffsets(
                id, XDIM, halfW, cluster->clusterVarId));
        _subConstraintInfo.push_back(new ClusterShapeOffsets(
                id, XDIM, -halfW, cluster->clusterVarId + 1));
        _subConstraintInfo.push_back(new ClusterShapeOffsets(
                id, YDIM, halfH, cluster->clusterVarId));
        _subConstraintInfo.push_back(new ClusterShapeOffsets(
                id, YDIM, -halfH, cluster->clusterVarId + 1));
    }
}


void ClusterContainmentConstraints::generateVariables(const vpsc::Dim dim,
        vpsc::Variables& vars) 
{
    COLA_UNUSED(dim);
    COLA_UNUSED(vars);
}



SubConstraintAlternatives 
ClusterContainmentConstraints::getCurrSubConstraintAlternatives(vpsc::Variables vs[])
{
    SubConstraintAlternatives alternatives;

    ClusterShapeOffsets *info = static_cast<ClusterShapeOffsets *>
            (_subConstraintInfo[_currSubConstraintIndex]);
    
    assertValidVariableIndex(vs[_primaryDim], info->varIndex);
    if (info->offset < 0)
    {
        // Constrain the objects with negative offsets to be 
        // to the left of the boundary.
        vpsc::Constraint constraint = vpsc::Constraint( 
                vs[info->dim][info->varIndex], vs[info->dim][info->boundaryVar], 
                -info->offset);
        alternatives.push_back(SubConstraint(info->dim, constraint));
    }
    else
    {
        // Constrain the objects with positive offsets to be 
        // to the right of the boundary.
        vpsc::Constraint constraint = vpsc::Constraint(
                vs[info->dim][info->boundaryVar], vs[info->dim][info->varIndex], 
                info->offset);
        alternatives.push_back(SubConstraint(info->dim, constraint));
    }

    //fprintf(stderr, "===== BOUNDARYLINE ALTERNATIVES -======\n");
    return alternatives;
}

#if 0
bool ClusterContainmentConstraints::subConstraintsRemaining(void) const
{
    //printf(". %3d of %4d\n", _currSubConstraintIndex, pairInfoList.size());
    return _currSubConstraintIndex < pairInfoList.size();
}


void ClusterContainmentConstraints::markAllSubConstraintsAsInactive(void)
{
    for (std::list<ShapePairInfo>::iterator curr = pairInfoList.begin();
            curr != pairInfoList.end(); ++curr)
    {
        ShapePairInfo& info = (*curr);
        info.satisfied = false;
        info.processed = false;
    }
    _currSubConstraintIndex = 0;
}


void ClusterContainmentConstraints::markCurrSubConstraintAsActive(
        const bool satisfiable)
{
    ShapePairInfo& info = pairInfoList.front();
    
    info.processed = true;
    info.satisfied = satisfiable;

    _currSubConstraintIndex++;
}


#endif


void ClusterContainmentConstraints::generateSeparationConstraints(
        const vpsc::Dim dim, vpsc::Variables& vs, vpsc::Constraints& cs,
        std::vector<vpsc::Rectangle*>& bbs) 
{
    COLA_UNUSED(bbs);
    for (SubConstraintInfoList::iterator o = _subConstraintInfo.begin();
            o != _subConstraintInfo.end(); ++o) 
    {
        ClusterShapeOffsets *info = static_cast<ClusterShapeOffsets *> (*o);

        if (info->dim != dim)
        {
            continue;
        }

        if (info->offset < 0)
        {
            // Constrain the objects with negative offsets to be 
            // to the left of the boundary.
            vpsc::Constraint *constraint = new vpsc::Constraint( 
                    vs[info->varIndex], vs[info->boundaryVar], 
                    -info->offset);
            cs.push_back(constraint);
        }
        else
        {
            // Constrain the objects with positive offsets to be 
            // to the right of the boundary.
            vpsc::Constraint *constraint = new vpsc::Constraint(
                    vs[info->boundaryVar], vs[info->varIndex], 
                    info->offset);
            cs.push_back(constraint);
        }
    }
}


} // namespace cola

// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=4:softtabstop=4 :

