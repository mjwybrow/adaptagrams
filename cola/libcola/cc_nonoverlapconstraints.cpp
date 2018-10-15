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

#include <sstream>

#include "libcola/cola.h"
#include "libcola/compound_constraints.h"
#include "libcola/cc_nonoverlapconstraints.h"

using vpsc::XDIM;
using vpsc::YDIM;


namespace cola {

//-----------------------------------------------------------------------------
// NonOverlapConstraintExemptions code
//-----------------------------------------------------------------------------

NonOverlapConstraintExemptions::NonOverlapConstraintExemptions()
{
}

void NonOverlapConstraintExemptions::addExemptGroupOfNodes(
        ListOfNodeIndexes listOfNodeGroups)
{
    m_exempt_pairs.clear();

    const size_t listSize = listOfNodeGroups.size();
    for (size_t l = 0; l < listSize; ++l)
    {
        NodeIndexes ids(listOfNodeGroups[l]);

        // Sort and remove duplicate IDs for this group.
        std::sort(ids.begin(), ids.end());
        NodeIndexes::iterator last = std::unique(ids.begin(), ids.end());
        ids.erase(last, ids.end());

        // Add all combinations of pairs of IDs for this group to m_exempt_pairs
        const size_t idsSize = ids.size();
        for (size_t i = 0; i < idsSize; ++i)
        {
            for (size_t j = i + 1; j < idsSize; ++j)
            {
                m_exempt_pairs.insert(ShapePair(ids[i], ids[j]));
            }
        }

    }
}

bool NonOverlapConstraintExemptions::shapePairIsExempt(
        ShapePair shapePair) const
{
    return (m_exempt_pairs.count(shapePair) == 1);
}

//-----------------------------------------------------------------------------
// NonOverlapConstraints code
//-----------------------------------------------------------------------------

NonOverlapConstraints::NonOverlapConstraints(
        NonOverlapConstraintExemptions *exemptions, unsigned int priority)
    : CompoundConstraint(vpsc::HORIZONTAL, priority),
      pairInfoListSorted(false),
      initialSortCompleted(false),
      m_exemptions(exemptions)
{
    // All work is done by repeated addShape() calls.
}

void NonOverlapConstraints::addShape(unsigned id, double halfW, double halfH,
        unsigned int group, std::set<unsigned> exemptions)
{
    // Setup pairInfos for all other shapes. 
    for (std::map<unsigned, OverlapShapeOffsets>::iterator curr =
            shapeOffsets.begin(); curr != shapeOffsets.end(); ++curr)
    {
        unsigned otherId = curr->first;
        if ((shapeOffsets[otherId].group == group) && (id != otherId) && exemptions.count(otherId)==0)
        {
            if (m_exemptions &&
                    m_exemptions->shapePairIsExempt(ShapePair(otherId, id)))
            {
                continue;
            }

            // Apply non-overlap only to objects in the same group (cluster).
            pairInfoList.push_back(ShapePairInfo(otherId, id));
        }
    }

    shapeOffsets[id] = OverlapShapeOffsets(id, halfW, halfH, group);
}

void NonOverlapConstraints::resizeShape(unsigned id, double halfW, double halfH)
{
    OverlapShapeOffsets oso = shapeOffsets[id];
    oso.resize(halfW, halfH);
}

void NonOverlapConstraints::removeShape(unsigned id)
{
    // Remove the OverlapShapeOffsets object for this id.
    shapeOffsets.erase(id);
    // Remove all ShapePairInfo objects having this id as one of their two indices.
    std::list<ShapePairInfo>::iterator it = pairInfoList.begin();
    while (it != pairInfoList.end()) {
        ShapePairInfo spi = *it;
        if (spi.varIndex1==id || spi.varIndex2==id) {
            it = pairInfoList.erase(it); // now it points to next list element after one erased
        } else {
            it++;
        }
    }
}

// This is expected to be called after all addNode calls.
void NonOverlapConstraints::addCluster(Cluster *cluster, unsigned int group)
{
    unsigned id = cluster->clusterVarId;
    // Setup pairInfos for all other shapes. 
    for (std::map<unsigned, OverlapShapeOffsets>::iterator curr =
            shapeOffsets.begin(); curr != shapeOffsets.end(); ++curr)
    {
        unsigned otherId = curr->first;
        if (shapeOffsets[otherId].group != group)
        {
            // Don't apply if not in same group.
            continue;
        }
        if (cluster->nodes.count(otherId) > 0)
        {
            // Don't apply non-overlap to child nodes.
            continue;
        }
        if (m_cluster_cluster_exemptions.count(ShapePair(id, otherId)) > 0)
        {
            // Don't apply  if exempt due to non-strict cluster hierarchy.
            continue;
        }
        // Apply non-overlap only to objects in the same group (cluster).
        pairInfoList.push_back(ShapePairInfo(otherId, id));
    }
    
    shapeOffsets[id] = OverlapShapeOffsets(id, cluster, group);
}


void NonOverlapConstraints::setClusterClusterExemptions(
        std::set<ShapePair> exemptions)
{
    m_cluster_cluster_exemptions = exemptions;
}

void NonOverlapConstraints::generateVariables(const vpsc::Dim dim,
        vpsc::Variables& vars) 
{
    COLA_UNUSED(dim);
    COLA_UNUSED(vars);
}

void NonOverlapConstraints::computeOverlapForShapePairInfo(ShapePairInfo& info,
        vpsc::Variables vs[])
{
    OverlapShapeOffsets& shape1 = shapeOffsets[info.varIndex1];
    OverlapShapeOffsets& shape2 = shapeOffsets[info.varIndex2];

    double xPos1 = vs[0][info.varIndex1]->finalPosition;
    double xPos2 = vs[0][info.varIndex2]->finalPosition;
    double yPos1 = vs[1][info.varIndex1]->finalPosition;
    double yPos2 = vs[1][info.varIndex2]->finalPosition;

    double left1   = xPos1 - shape1.halfDim[0];
    double right1  = xPos1 + shape1.halfDim[0];
    double bottom1 = yPos1 - shape1.halfDim[1];
    double top1    = yPos1 + shape1.halfDim[1];

    if (shape1.cluster)
    {
        COLA_ASSERT(shape1.halfDim[0] == 0);
        COLA_ASSERT(shape1.halfDim[1] == 0);
        COLA_ASSERT(info.varIndex1 + 1 < vs[0].size());
        right1 = vs[0][info.varIndex1 + 1]->finalPosition;
        COLA_ASSERT(info.varIndex1 + 1 < vs[1].size());
        top1    = vs[1][info.varIndex1 + 1]->finalPosition;
        left1 -= shape1.rectPadding.min(XDIM);
        bottom1 -= shape1.rectPadding.min(YDIM);
        right1 += shape1.rectPadding.max(XDIM);
        top1 += shape1.rectPadding.max(YDIM);
    }

    double left2   = xPos2 - shape2.halfDim[0];
    double right2  = xPos2 + shape2.halfDim[0];
    double bottom2 = yPos2 - shape2.halfDim[1];
    double top2    = yPos2 + shape2.halfDim[1];

    if (shape2.cluster)
    {
        COLA_ASSERT(shape2.halfDim[0] == 0);
        COLA_ASSERT(shape2.halfDim[1] == 0);
        COLA_ASSERT(info.varIndex2 + 1 < vs[0].size());
        right2 = vs[0][info.varIndex2 + 1]->finalPosition;
        COLA_ASSERT(info.varIndex2 + 1 < vs[1].size());
        top2   = vs[1][info.varIndex2 + 1]->finalPosition;
        left2 -= shape2.rectPadding.min(XDIM);
        bottom2 -= shape2.rectPadding.min(YDIM);
        right2 += shape2.rectPadding.max(XDIM);
        top2 += shape2.rectPadding.max(YDIM);
    }

    // If lr < 0, then left edge of shape1 is on the left 
    // of right edge of shape2.
    double spaceR = left2 - right1;
    double spaceL = left1 - right2;
    // Below
    double spaceA = bottom2 - top1;
    // Above
    double spaceB = bottom1 - top2;

    double costL = 0;
    double costR = 0;
    double costB = 0;
    double costA = 0;
    info.overlapMax = 0;
    bool xOverlap = false;
    bool yOverlap = false;
    if ((spaceR < 0) && (spaceL < 0))
    {
        costL = std::max(-spaceL, 0.0);
        costR = std::max(-spaceR, 0.0);

        info.overlapMax = std::max(costL, costR);

        xOverlap = true;
    }
    if ((spaceB < 0) && (spaceA < 0))
    {
        costB = std::max(-spaceB, 0.0);
        costA = std::max(-spaceA, 0.0);

        info.overlapMax = std::max(info.overlapMax, costB);
        info.overlapMax = std::max(info.overlapMax, costA);

        yOverlap = true;
    }

    if (!xOverlap || !yOverlap)
    {
        // Overlap must occur in both dimensions.
        info.overlapMax = 0;
    }
    else
    {
        // Increase the overlap value for overlap where one shape is fully
        // within the other.  This results in these situations being
        // resolved first and will sometimes prevent us from committing to
        // bad non-overlap choices and getting stuck later.
        // XXX Another alternative would be to look at previously added
        //     non-overlap constraints that become unsatified and then 
        //     allow them to be rechosen maybe just a single time.
        double penalty = 100000;
        if ( (left1 >= left2) && (right1 <= right2) &&
             (bottom1 >= bottom2) && (top1 <= top2) )
        {
            // Shape 1 is inside shape 2.
            double smallShapeArea = (right1 - left1) * (top1 - bottom1);
            info.overlapMax = penalty + smallShapeArea;
        }
        else if ( (left2 >= left1) && (right2 <= right1) &&
             (bottom2 >= bottom1) && (top2 <= top1) )
        {
            // Shape 2 is inside shape 1.
            double smallShapeArea = (right2 - left2) * (top2 - bottom2);
            info.overlapMax = penalty + smallShapeArea;
        }
        // There is overlap.
        //printf("[%02d][%02d] L %g, R %G, B %g, A %g\n", info.varIndex1, 
        //        info.varIndex2, spaceL, spaceR, spaceB, spaceA);
    }
}

std::string NonOverlapConstraints::toString(void) const
{
    std::ostringstream stream;
    stream << "NonOverlapConstraints()";
    return stream.str();
}

void NonOverlapConstraints::computeAndSortOverlap(vpsc::Variables vs[])
{
    for (std::list<ShapePairInfo>::iterator curr = pairInfoList.begin();
            curr != pairInfoList.end(); ++curr)
    {
        ShapePairInfo& info = static_cast<ShapePairInfo&> (*curr);
        
        if (info.processed)
        {
            // Once we reach the processed nodes, which will always be at 
            // the back, we can some computing overlap since this no longer
            // matters.
            break;
        }
        computeOverlapForShapePairInfo(info, vs);
    }
    pairInfoList.sort();
}


void NonOverlapConstraints::markCurrSubConstraintAsActive(const bool satisfiable)
{
    ShapePairInfo info = pairInfoList.front();
    pairInfoList.pop_front();

    info.processed = true;
    info.satisfied = satisfiable;
    info.overlapMax = 0;
    
    pairInfoList.push_back(info);
    pairInfoListSorted = false;
}



SubConstraintAlternatives 
NonOverlapConstraints::getCurrSubConstraintAlternatives(vpsc::Variables vs[])
{
    SubConstraintAlternatives alternatives;
    if (initialSortCompleted == false)
    {
        // Initally sort the shape pair info objects.
        computeAndSortOverlap(vs);
        pairInfoListSorted = true;
        initialSortCompleted = true;
    }

    // Take the first in the list.
    ShapePairInfo& info = pairInfoList.front();
    if (pairInfoListSorted == false)
    {
        // Only need to compute if not sorted.
        computeOverlapForShapePairInfo(info, vs);
    }

    if (info.overlapMax == 0)
    {
        if (pairInfoListSorted)
        {
            // Seeing no overlap in the sorted list means we have solved
            // all non-overlap.  Nothing more to do.
            _currSubConstraintIndex = pairInfoList.size();
            return alternatives;
        }
        computeAndSortOverlap(vs);
        pairInfoListSorted = true;
        return alternatives;
    }
    OverlapShapeOffsets& shape1 = shapeOffsets[info.varIndex1];
    OverlapShapeOffsets& shape2 = shapeOffsets[info.varIndex2];

    double xSepL = shape1.halfDim[0] + shape2.halfDim[0];
    double ySepB = shape1.halfDim[1] + shape2.halfDim[1];
    double xSepR = xSepL;
    double ySepT = ySepB;

    unsigned varIndexL1 = info.varIndex1;
    unsigned varIndexL2 = info.varIndex2;
    // Clusters have left and right variables, instead of centre variables.
    unsigned varIndexR1 = 
            (shape1.cluster) ? (info.varIndex1 + 1) : info.varIndex1;
    unsigned varIndexR2 = 
            (shape2.cluster) ? (info.varIndex2 + 1) : info.varIndex2;

    assertValidVariableIndex(vs[XDIM], varIndexL1);
    assertValidVariableIndex(vs[YDIM], varIndexL1);
    assertValidVariableIndex(vs[XDIM], varIndexR1);
    assertValidVariableIndex(vs[YDIM], varIndexR1);
    assertValidVariableIndex(vs[XDIM], varIndexL2);
    assertValidVariableIndex(vs[YDIM], varIndexL2);
    assertValidVariableIndex(vs[XDIM], varIndexR2);
    assertValidVariableIndex(vs[YDIM], varIndexR2);

    // 
    double xSepCostL = xSepL;
    double xSepCostR = xSepR;
    double ySepCostB = ySepB;
    double ySepCostT = ySepT;

    double desiredX1 = vs[XDIM][info.varIndex1]->desiredPosition;
    double desiredY1 = vs[YDIM][info.varIndex1]->desiredPosition;
    double desiredX2 = vs[XDIM][info.varIndex2]->desiredPosition;
    double desiredY2 = vs[YDIM][info.varIndex2]->desiredPosition;
    
    // Clusters have two variables instead of a centre variable -- one for
    // each boundary side, so we need to remap the desired positions and the
    // separations values for the purposes of cost sorting.
    if (shape1.cluster)
    {
        double width = vs[XDIM][info.varIndex1 + 1]->finalPosition -
                vs[XDIM][info.varIndex1]->finalPosition;
        double height = vs[YDIM][info.varIndex1 + 1]->finalPosition -
                vs[YDIM][info.varIndex1]->finalPosition;
        desiredX1 += width / 2;
        desiredY1 += height / 2;
        xSepCostL += width / 2;
        xSepCostR += width / 2;
        ySepCostB += height / 2;
        ySepCostT += height / 2;
        xSepL += shape1.rectPadding.min(XDIM);
        xSepR += shape1.rectPadding.max(XDIM);
        ySepB += shape1.rectPadding.min(YDIM);
        ySepT += shape1.rectPadding.max(YDIM);
    }
    if (shape2.cluster)
    {
        double width = vs[XDIM][info.varIndex2 + 1]->finalPosition -
                vs[XDIM][info.varIndex2]->finalPosition;
        double height = vs[YDIM][info.varIndex2 + 1]->finalPosition -
                vs[YDIM][info.varIndex2]->finalPosition;
        desiredX2 += width / 2;
        desiredY2 += height / 2;
        xSepCostL += width / 2;
        xSepCostR += width / 2;
        ySepCostB += height / 2;
        ySepCostT += height / 2;
        xSepL += shape2.rectPadding.min(XDIM);
        xSepR += shape2.rectPadding.max(XDIM);
        ySepB += shape2.rectPadding.min(YDIM);
        ySepT += shape2.rectPadding.max(YDIM);
    }

    // We get problems with numerical inaccuracy in the topology constraint
    // generation, so make sure the rectagles are separated by at least a 
    // tiny distance.
    xSepL += 10e-10;
    xSepR += 10e-10;
    ySepB += 10e-10;
    ySepT += 10e-10;

    // Compute the cost to move in each direction based on the 
    // desired positions for the two objects.
    double costR = xSepCostR - (desiredX2 - desiredX1);
    double costL = xSepCostL - (desiredX1 - desiredX2);
    
    double costT = ySepCostT - (desiredY2 - desiredY1);
    double costB = ySepCostB - (desiredY1 - desiredY2);

    vpsc::Constraint constraintL(vs[XDIM][varIndexR2], 
            vs[XDIM][varIndexL1], xSepL);
    alternatives.push_back(SubConstraint(XDIM, constraintL, costL));

    vpsc::Constraint constraintR(vs[XDIM][varIndexR1], 
            vs[XDIM][varIndexL2], xSepR);
    alternatives.push_back(SubConstraint(XDIM, constraintR, costR));

    vpsc::Constraint constraintB(vs[YDIM][varIndexR2], 
            vs[YDIM][varIndexL1], ySepB);
    alternatives.push_back(SubConstraint(YDIM, constraintB, costB));

    vpsc::Constraint constraintT(vs[YDIM][varIndexR1], 
            vs[YDIM][varIndexL2], ySepT);
    alternatives.push_back(SubConstraint(YDIM, constraintT, costT));
    
    //fprintf(stderr, "===== NONOVERLAP ALTERNATIVES -====== \n");

    return alternatives;
}


bool NonOverlapConstraints::subConstraintsRemaining(void) const
{
    //printf(". %3d of %4d\n", _currSubConstraintIndex, pairInfoList.size());
    return _currSubConstraintIndex < pairInfoList.size();
}


void NonOverlapConstraints::markAllSubConstraintsAsInactive(void)
{
    for (std::list<ShapePairInfo>::iterator curr = pairInfoList.begin();
            curr != pairInfoList.end(); ++curr)
    {
        ShapePairInfo& info = (*curr);
        info.satisfied = false;
        info.processed = false;
    }
    _currSubConstraintIndex = 0;
    initialSortCompleted = false;
}


void NonOverlapConstraints::generateSeparationConstraints(
        const vpsc::Dim dim, vpsc::Variables& vs, vpsc::Constraints& cs,
        std::vector<vpsc::Rectangle*>& boundingBoxes) 
{
    for (std::list<ShapePairInfo>::iterator info = pairInfoList.begin();
            info != pairInfoList.end(); ++info)
    {
        assertValidVariableIndex(vs, info->varIndex1);
        assertValidVariableIndex(vs, info->varIndex2);
        
        OverlapShapeOffsets& shape1 = shapeOffsets[info->varIndex1];
        OverlapShapeOffsets& shape2 = shapeOffsets[info->varIndex2];
        
        vpsc::Rectangle rect1 = (shape1.cluster) ?
                shape1.cluster->bounds : *boundingBoxes[info->varIndex1];
        vpsc::Rectangle rect2 = (shape2.cluster) ?
                shape2.cluster->bounds : *boundingBoxes[info->varIndex2];

        double pos1 = rect1.getCentreD(dim);
        double pos2 = rect2.getCentreD(dim);

        double below1 = shape1.halfDim[dim];
        double above1 = shape1.halfDim[dim];
        double below2 = shape2.halfDim[dim];
        double above2 = shape2.halfDim[dim];

        vpsc::Variable *varLeft1 = nullptr;
        vpsc::Variable *varLeft2 = nullptr;
        vpsc::Variable *varRight1 = nullptr;
        vpsc::Variable *varRight2 = nullptr;
        if (shape1.cluster)
        {
            // Must constraint to cluster boundary variables.
            varLeft1 = vs[shape1.cluster->clusterVarId];
            varRight1 = vs[shape1.cluster->clusterVarId + 1];
            rect1 = shape1.cluster->margin().rectangleByApplyingBox(rect1);
            below1 = shape1.cluster->margin().min(dim);
            above1 = shape1.cluster->margin().max(dim);
        }
        else
        {
            // Must constrain to rectangle centre postion variable.
            varLeft1 = varRight1 = vs[info->varIndex1];
        }

        if (shape2.cluster)
        {
            // Must constraint to cluster boundary variables.
            varLeft2 = vs[shape2.cluster->clusterVarId];
            varRight2 = vs[shape2.cluster->clusterVarId + 1];
            rect2 = shape2.cluster->margin().rectangleByApplyingBox(rect2);
            below2 = shape2.cluster->margin().min(dim);
            above2 = shape2.cluster->margin().max(dim);
        }
        else
        {
            // Must constrain to rectangle centre postion variable.
            varLeft2 = varRight2 = vs[info->varIndex2];
        }

        if (rect1.overlapD(!dim, &rect2) > 0.0005)
        {
            vpsc::Constraint *constraint = nullptr;
            if (pos1 < pos2)
            {
                constraint = new vpsc::Constraint(varRight1, varLeft2,
                             above1 + below2);
            }
            else
            {
                constraint = new vpsc::Constraint(varRight2, varLeft1,
                        below1 + above2);
            }
            constraint->creator = this;
            cs.push_back(constraint);
        }
    }
}


} // namespace cola
