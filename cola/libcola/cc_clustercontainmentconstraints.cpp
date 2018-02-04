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

#include <sstream>

#include "libcola/cola.h"
#include "libcola/compound_constraints.h"
#include "libcola/cc_clustercontainmentconstraints.h"

using vpsc::XDIM;
using vpsc::YDIM;


namespace cola {

static const int AboveBoundary = 1;
static const int BelowBoundary = -1;

class ClusterShapeOffsets : public SubConstraintInfo
{
    public:
        ClusterShapeOffsets(unsigned ind, vpsc::Dim dim, double offset, int boundarySide,
                unsigned int boundaryVar) :
            SubConstraintInfo(ind),
            offset(offset),
            dim(dim),
            boundarySide(boundarySide),
            boundaryVar(boundaryVar)
        {
        }
        double offset;    // half the width or height value
        vpsc::Dim dim;    // The dimension
        int boundarySide; // The boundary side we are constraining on.
        unsigned int boundaryVar; // The boundary var.
};


ClusterContainmentConstraints::ClusterContainmentConstraints(Cluster *cluster, 
        unsigned int priority, std::vector<vpsc::Rectangle*>& boundingBoxes)
    : CompoundConstraint(vpsc::HORIZONTAL, priority)
{
    Box padding = cluster->padding();
    _combineSubConstraints = true;
    for (std::set<unsigned>::iterator curr = cluster->nodes.begin();
            curr != cluster->nodes.end(); ++curr)
    {
        unsigned id = *curr;
        double halfW = (boundingBoxes[id]->width() / 2.0);
        double halfH = (boundingBoxes[id]->height() / 2.0);
        _subConstraintInfo.push_back(new ClusterShapeOffsets(
                id, XDIM, halfW + padding.min(XDIM), AboveBoundary, 
                cluster->clusterVarId));
        _subConstraintInfo.push_back(new ClusterShapeOffsets(
                id, XDIM, halfW + padding.max(XDIM), BelowBoundary,
                cluster->clusterVarId + 1));
        _subConstraintInfo.push_back(new ClusterShapeOffsets(
                id, YDIM, halfH + padding.min(YDIM), AboveBoundary,
                cluster->clusterVarId));
        _subConstraintInfo.push_back(new ClusterShapeOffsets(
                id, YDIM, halfH + padding.max(YDIM), BelowBoundary,
                cluster->clusterVarId + 1));
    }
    for (std::vector<Cluster *>::iterator curr = cluster->clusters.begin();
            curr != cluster->clusters.end(); ++curr)
    {
        Cluster *childCluster = *curr;
        Box margin = childCluster->margin();
        _subConstraintInfo.push_back(new ClusterShapeOffsets(
                childCluster->clusterVarId, XDIM, 
                padding.min(XDIM) + margin.min(XDIM), AboveBoundary, 
                cluster->clusterVarId));
        _subConstraintInfo.push_back(new ClusterShapeOffsets(
                childCluster->clusterVarId + 1, XDIM,
                padding.max(XDIM) + margin.max(XDIM), BelowBoundary,
                cluster->clusterVarId + 1));
        _subConstraintInfo.push_back(new ClusterShapeOffsets(
                childCluster->clusterVarId, YDIM,
                padding.min(YDIM) + margin.min(YDIM), AboveBoundary,
                cluster->clusterVarId));
        _subConstraintInfo.push_back(new ClusterShapeOffsets(
                childCluster->clusterVarId + 1, YDIM,
                padding.max(YDIM) + margin.max(YDIM), BelowBoundary,
                cluster->clusterVarId + 1));
    }
}


std::string ClusterContainmentConstraints::toString(void) const
{
    std::ostringstream stream;
    stream << "ClusterContainmentConstraints()";
    return stream.str();
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
    if (info->boundarySide == BelowBoundary)
    {
        // Constrain the objects with negative offsets to be 
        // to the left of the boundary.
        vpsc::Constraint constraint = vpsc::Constraint( 
                vs[info->dim][info->varIndex], vs[info->dim][info->boundaryVar], 
                info->offset);
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

        vpsc::Constraint *constraint = nullptr;
        if (info->boundarySide == BelowBoundary)
        {
            // Constrain the objects with negative offsets to be 
            // to the left of the boundary.
            constraint = new vpsc::Constraint(vs[info->varIndex],
                    vs[info->boundaryVar], info->offset);
        }
        else
        {
            // Constrain the objects with positive offsets to be 
            // to the right of the boundary.
            constraint = new vpsc::Constraint(vs[info->boundaryVar],
                    vs[info->varIndex], info->offset);
        }
        constraint->creator = this;
        cs.push_back(constraint);
    }
}


} // namespace cola

