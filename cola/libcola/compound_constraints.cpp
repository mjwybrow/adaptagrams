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

#include <algorithm>

#include <libvpsc/variable.h>
#include <libvpsc/constraint.h>
#include <libvpsc/assertions.h>

#include "cola.h"
#include "compound_constraints.h"
#include "exceptions.h"

using std::vector;
using vpsc::XDIM;
using vpsc::YDIM;

namespace cola {

static const double freeWeight = 0.0001;

static const unsigned int PRIORITY_NONOVERLAP = 
        DEFAULT_CONSTRAINT_PRIORITY - 50;

//-----------------------------------------------------------------------------
// BoundaryConstraint code
//-----------------------------------------------------------------------------

class Offset : public SubConstraintInfo
{
    public:
        Offset(unsigned ind, double offset) :
            SubConstraintInfo(ind),
            distOffset(offset)
        {
        }
        double distOffset;
};


BoundaryConstraint::BoundaryConstraint(const vpsc::Dim dim) 
    : CompoundConstraint(dim),
      position(0), 
      variable(NULL) 
{
}


void BoundaryConstraint::updatePosition(const vpsc::Dim dim) 
{
    if (dim == _primaryDim)
    {
        position = variable->finalPosition;
    }
}


void BoundaryConstraint::addShape(const unsigned int index, 
        const double offset)
{
    _subConstraintInfo.push_back(new Offset(index, offset));
}


void BoundaryConstraint::generateVariables(const vpsc::Dim dim, 
        vpsc::Variables& vars) 
{
    if (dim == _primaryDim)
    {
        // Just one variable is generated, associated with the position
        // of the boundary.  This variable can float freely.
        variable = new vpsc::Variable(vars.size(), position, freeWeight);
        vars.push_back(variable);
    }
}


void BoundaryConstraint::generateSeparationConstraints(const vpsc::Dim dim,
        vpsc::Variables& vars, vpsc::Constraints& cs) 
{
    if (dim == _primaryDim)
    {
        COLA_ASSERT(variable != NULL);
        for (SubConstraintInfoList::iterator o = _subConstraintInfo.begin();
                o != _subConstraintInfo.end(); ++o) 
        {
            Offset *info = static_cast<Offset *> (*o);
            assertValidVariableIndex(vars, info->varIndex);
            if (info->distOffset < 0)
            {
                // Constrain the objects with negative offsets to be 
                // to the left of the boundary.
                cs.push_back(new vpsc::Constraint(
                        vars[info->varIndex], variable, -info->distOffset));
            }
            else
            {
                // Constrain the objects with positive offsets to be 
                // to the right of the boundary.
                cs.push_back(new vpsc::Constraint(
                        variable, vars[info->varIndex], info->distOffset));
            }
        }
    }
}


SubConstraintAlternatives 
BoundaryConstraint::getCurrSubConstraintAlternatives(vpsc::Variables vs[])
{
    SubConstraintAlternatives alternatives;

    Offset *info = static_cast<Offset *> 
            (_subConstraintInfo[_currSubConstraintIndex]);
    
    assertValidVariableIndex(vs[_primaryDim], info->varIndex);
    if (info->distOffset < 0)
    {
        // Constrain the objects with negative offsets to be 
        // to the left of the boundary.
        vpsc::Constraint constraint = vpsc::Constraint(
                vs[_primaryDim][info->varIndex], variable, -info->distOffset);
        alternatives.push_back(SubConstraint(_primaryDim, constraint));
    }
    else
    {
        // Constrain the objects with positive offsets to be 
        // to the right of the boundary.
        vpsc::Constraint constraint = vpsc::Constraint(
                variable, vs[_primaryDim][info->varIndex], info->distOffset);
        alternatives.push_back(SubConstraint(_primaryDim, constraint));
    }

    //fprintf(stderr, "===== BOUNDARYLINE ALTERNATIVES -======\n");
    return alternatives;
}



//-----------------------------------------------------------------------------
// AlignmentConstraint code
//-----------------------------------------------------------------------------

AlignmentConstraint::AlignmentConstraint(const vpsc::Dim dim, double position)
    : CompoundConstraint(dim),
      variable(NULL),
      _position(position), 
      _isFixed(false)
{
}


void AlignmentConstraint::addShape(const unsigned int index, 
        const double offset)
{
    _subConstraintInfo.push_back(new Offset(index, offset));
}


void AlignmentConstraint::updatePosition(const vpsc::Dim dim) 
{
    if (dim == _primaryDim)
    {
        _position = variable->finalPosition;
    }
}


void AlignmentConstraint::fixPos(double pos) 
{
    _position = pos;
    _isFixed = true;
}


void AlignmentConstraint::unfixPos() 
{
    _isFixed = false;
}


double AlignmentConstraint::position(void) const 
{
    return _position;
}

bool AlignmentConstraint::isFixed(void) const
{
    return _isFixed;
}

void AlignmentConstraint::generateVariables(const vpsc::Dim dim, 
        vpsc::Variables& vars) 
{
    if (dim == _primaryDim)
    {
        // Variable representing the position of a guideline.
        variable = new vpsc::Variable(vars.size(), _position, freeWeight);
        if (_isFixed) 
        {
            variable->fixedDesiredPosition = true;
            variable->weight = 100000;
        }
        vars.push_back(variable);
    }
}


void AlignmentConstraint::generateSeparationConstraints(const vpsc::Dim dim,
        vpsc::Variables& vars, vpsc::Constraints& cs) 
{
    if (dim == _primaryDim)
    {
        COLA_ASSERT(variable != NULL);
        // Constrain each object to be offset from the guideline by
        // some exact amount.
        for (SubConstraintInfoList::iterator o = _subConstraintInfo.begin();
                o != _subConstraintInfo.end(); ++o) 
        {
            Offset *info = static_cast<Offset *> (*o);
            assertValidVariableIndex(vars, info->varIndex);
            cs.push_back(new vpsc::Constraint(
                        variable, vars[info->varIndex], info->distOffset, true));
        }
    }
}


void AlignmentConstraint::printCreationCode(FILE *fp) const
{
    fprintf(fp, "    AlignmentConstraint alignment%lu(%u, %g);\n",
            (unsigned long) this, _primaryDim, _position);
    for (SubConstraintInfoList::const_iterator o = _subConstraintInfo.begin();
            o != _subConstraintInfo.end(); ++o) 
    {
        Offset *info = static_cast<Offset *> (*o);
        fprintf(fp, "    alignment%lu.addShape(%u, %g);\n",
                (unsigned long) this, info->varIndex, info->distOffset);
    }
    fprintf(fp, "    ccs.push_back(alignment%lu);\n",
            (unsigned long) this);
}


SubConstraintAlternatives 
AlignmentConstraint::getCurrSubConstraintAlternatives(vpsc::Variables vs[])
{
    SubConstraintAlternatives alternatives;

    Offset *info = static_cast<Offset *> 
            (_subConstraintInfo[_currSubConstraintIndex]);
    
    assertValidVariableIndex(vs[_primaryDim], info->varIndex);
    vpsc::Constraint constraint(variable, vs[_primaryDim][info->varIndex], 
            info->distOffset, true);
    alternatives.push_back(SubConstraint(_primaryDim, constraint));

    //fprintf(stderr, "===== ALIGN ALTERNATIVES -======\n");
    return alternatives;
}


//-----------------------------------------------------------------------------
// SeparationConstraint code
//-----------------------------------------------------------------------------


class VarIndexPair : public SubConstraintInfo
{
    public:
        VarIndexPair(unsigned ind1, unsigned ind2) 
            : SubConstraintInfo(ind1),
              lConstraint(NULL),
              rConstraint(NULL),
              varIndex2(ind2)
        {
        }
        VarIndexPair(AlignmentConstraint *l, AlignmentConstraint *r) 
            : SubConstraintInfo(0),
              lConstraint(l),
              rConstraint(r),
              varIndex2(0)
        {
        }
        unsigned indexL(void) const
        {
            return (lConstraint) ? lConstraint->variable->id : varIndex;
        }
        unsigned indexR(void) const
        {
            return (rConstraint) ? rConstraint->variable->id : varIndex2;
        }
    private:
        AlignmentConstraint *lConstraint;
        AlignmentConstraint *rConstraint;
        unsigned varIndex2;
};


SeparationConstraint::SeparationConstraint(const vpsc::Dim dim, 
        unsigned l, unsigned r, double g, bool equality)
    : CompoundConstraint(dim),
      gap(g), 
      equality(equality),
      vpscConstraint(NULL)
{
    _subConstraintInfo.push_back(new VarIndexPair(l, r));
}


SeparationConstraint::SeparationConstraint(const vpsc::Dim dim, 
        AlignmentConstraint *l, AlignmentConstraint *r, double g, 
        bool equality) 
    : CompoundConstraint(dim),
      gap(g),
      equality(equality)
{
    COLA_ASSERT(l);
    COLA_ASSERT(r);
    
    _subConstraintInfo.push_back(new VarIndexPair(l, r));
}


void SeparationConstraint::generateVariables(const vpsc::Dim dim, 
        vpsc::Variables& vars) 
{
    // No additional variables are required!
}


SubConstraintAlternatives 
SeparationConstraint::getCurrSubConstraintAlternatives(vpsc::Variables vs[])
{
    SubConstraintAlternatives alternatives;

    VarIndexPair *info = static_cast<VarIndexPair *> 
            (_subConstraintInfo[_currSubConstraintIndex]);
    
    assertValidVariableIndex(vs[_primaryDim], info->indexL());
    assertValidVariableIndex(vs[_primaryDim], info->indexR());
    vpsc::Constraint constraint(vs[_primaryDim][info->indexL()],
            vs[_primaryDim][info->indexR()], gap, equality);
    alternatives.push_back(SubConstraint(_primaryDim, constraint));

    //fprintf(stderr, "===== SEPARATION ALTERNATIVES -======\n");
    return alternatives;
}


void SeparationConstraint::generateSeparationConstraints(const vpsc::Dim dim,
            vpsc::Variables& vs, vpsc::Constraints& cs) 
{
    if (dim == _primaryDim)
    {
        VarIndexPair *info = 
                static_cast<VarIndexPair *> (_subConstraintInfo.front());
        
        unsigned left = info->indexL();
        unsigned right = info->indexR();
        assertValidVariableIndex(vs, left);
        assertValidVariableIndex(vs, right);
        vpscConstraint = 
                new vpsc::Constraint(vs[left], vs[right], gap, equality);
        cs.push_back(vpscConstraint);
    }
}


unsigned SeparationConstraint::left(void) const
{
    VarIndexPair *info =
            static_cast<VarIndexPair *> (_subConstraintInfo.front());

    return info->indexL();
}


unsigned SeparationConstraint::right(void) const
{
    VarIndexPair *info =
            static_cast<VarIndexPair *> (_subConstraintInfo.front());

    return info->indexR();
}


void SeparationConstraint::setSeparation(double gap) 
{
    this->gap = gap;
    if (vpscConstraint != NULL) 
    {
        vpscConstraint->gap = gap;
    }
}


//-----------------------------------------------------------------------------
// OrthogonalEdgeConstraint code
//-----------------------------------------------------------------------------

OrthogonalEdgeConstraint::OrthogonalEdgeConstraint(const vpsc::Dim dim, 
        unsigned l, unsigned r)
    : CompoundConstraint(dim),
      left(l), 
      right(r), 
      vpscConstraint(NULL)
{
}


void OrthogonalEdgeConstraint::generateVariables(const vpsc::Dim dim, 
        vpsc::Variables& vars) 
{
    // No additional variables are required!
}


SubConstraintAlternatives 
OrthogonalEdgeConstraint::getCurrSubConstraintAlternatives(
        vpsc::Variables vs[])
{
    // XXX: What to do here?
    return SubConstraintAlternatives();
}


void OrthogonalEdgeConstraint::generateSeparationConstraints(
        const vpsc::Dim dim, vpsc::Variables& vs, vpsc::Constraints& cs) 
{
    if (dim == _primaryDim)
    {
        assertValidVariableIndex(vs, left);
        assertValidVariableIndex(vs, right);
        vpscConstraint = new vpsc::Constraint(vs[left], vs[right], 0, true);
        cs.push_back(vpscConstraint);
    }
}


void OrthogonalEdgeConstraint::generateTopologyConstraints(const vpsc::Dim k, 
        const vpsc::Rectangles& rs, vector<vpsc::Variable*> const & vars, 
        vector<vpsc::Constraint*> & cs) 
{
    assertValidVariableIndex(vars, left);
    assertValidVariableIndex(vars, right);
    double lBound, rBound, pos;
    if (k == vpsc::HORIZONTAL) 
    {
        lBound = rs[left]->getCentreY();
        rBound = rs[right]->getCentreY();
        pos = rs[left]->getCentreX();
    } 
    else 
    {
        lBound = rs[left]->getCentreX();
        rBound = rs[right]->getCentreX();
        pos = rs[left]->getCentreY();
    }
    double minBound = std::min(lBound, rBound);
    double maxBound = std::max(lBound, rBound);
    for (unsigned i = 0; i < rs.size(); ++i)
    {
        if (i==left || i==right) continue;
        vpsc::Rectangle *r = rs[i];
        if (r->allowOverlap()) continue;
        double l, rMin, rMax, rCentre;
        rectBounds(k, r, rMin, rMax, rCentre, l);
        if ((rMin >= minBound) && (rMin <= maxBound) || 
                (rMax >= minBound) && (rMax <= maxBound)) 
        {
            double g = l / 2;
            if (rCentre < pos) 
            {
                cs.push_back(new vpsc::Constraint(vars[i], vars[left], g));
            } 
            else 
            {
                cs.push_back(new vpsc::Constraint(vars[left], vars[i], g));
            }
        }
    }
}


void OrthogonalEdgeConstraint::rectBounds(const vpsc::Dim k, 
        vpsc::Rectangle const *r, double& cmin, double& cmax, 
        double& centre, double & l) const
{
	if (k == vpsc::HORIZONTAL)
    {
        cmin = r->getMinY();
        cmax = r->getMaxY();
        centre = r->getCentreX();
        l = r->width();
    } 
    else 
    {
        cmin = r->getMinX();
        cmax = r->getMaxX();
        centre = r->getCentreY();
        l = r->height();
    }
}


//-----------------------------------------------------------------------------
// MultiSeparationConstraint code
//-----------------------------------------------------------------------------

class AlignmentPair : public SubConstraintInfo
{
    public:
        AlignmentPair(AlignmentConstraint *ac1, AlignmentConstraint *ac2) 
            : SubConstraintInfo(0),
              alignment1(ac1),
              alignment2(ac2)
        {
        }
        AlignmentConstraint *alignment1;
        AlignmentConstraint *alignment2;
};


MultiSeparationConstraint::MultiSeparationConstraint(const vpsc::Dim dim, 
        double minSep, bool equality)
    : CompoundConstraint(dim),
      sep(minSep), 
      equality(equality)
{ 
}


void MultiSeparationConstraint::addAlignmentPair(AlignmentConstraint *ac1,
        AlignmentConstraint *ac2)
{
    _subConstraintInfo.push_back(new AlignmentPair(ac1, ac2));
}


void MultiSeparationConstraint::generateVariables(const vpsc::Dim dim, 
        vpsc::Variables& vars)
{
    // No additional variables are required!
}


SubConstraintAlternatives 
MultiSeparationConstraint::getCurrSubConstraintAlternatives(
        vpsc::Variables vs[])
{
    SubConstraintAlternatives alternatives;

    AlignmentPair *info = static_cast<AlignmentPair *> 
            (_subConstraintInfo[_currSubConstraintIndex]);
    
    AlignmentConstraint *c1 = info->alignment1;
    AlignmentConstraint *c2 = info->alignment2;
    if (!c1->variable || !c2->variable)
    {
        throw InvalidConstraint(this);
    }
    vpsc::Constraint constraint(c1->variable, c2->variable, sep, equality);
    alternatives.push_back(SubConstraint(_primaryDim, constraint));

    //fprintf(stderr, "===== MULTI SEPARATION ALTERNATIVES -======\n");
    return alternatives;
}


void MultiSeparationConstraint::setSeparation(double sep) 
{ 
    this->sep = sep;
}


void MultiSeparationConstraint::generateSeparationConstraints(
        const vpsc::Dim dim, vpsc::Variables& vs, vpsc::Constraints& gcs) 
{
    if (dim == _primaryDim)
    {
        for (SubConstraintInfoList::iterator o = _subConstraintInfo.begin();
                o != _subConstraintInfo.end(); ++o) 
        {
            AlignmentPair *info = static_cast<AlignmentPair *> (*o);
            AlignmentConstraint *c1 = info->alignment1;
            AlignmentConstraint *c2 = info->alignment2;
            if (!c1->variable || !c2->variable)
            {
                throw InvalidConstraint(this);
            }
            vpsc::Constraint *c = new vpsc::Constraint(
                c1->variable, c2->variable, sep, equality);
            gcs.push_back(c);
            cs.push_back(c);
        }
    }
}


//-----------------------------------------------------------------------------
// DistributionConstraint code
//-----------------------------------------------------------------------------

DistributionConstraint::DistributionConstraint(const vpsc::Dim dim)
    : CompoundConstraint(dim)
{
}


void DistributionConstraint::addAlignmentPair(AlignmentConstraint *ac1,
        AlignmentConstraint *ac2)
{
    _subConstraintInfo.push_back(new AlignmentPair(ac1, ac2));
}


void DistributionConstraint::generateVariables(const vpsc::Dim dim, 
        vpsc::Variables& vars)
{
    // No additional variables are required!
}


void DistributionConstraint::setSeparation(double sep) 
{
    this->sep = sep;
}


SubConstraintAlternatives 
DistributionConstraint::getCurrSubConstraintAlternatives(vpsc::Variables vs[])
{
    SubConstraintAlternatives alternatives;

    AlignmentPair *info = static_cast<AlignmentPair *>
            (_subConstraintInfo[_currSubConstraintIndex]);
    AlignmentConstraint *c1 = info->alignment1;
    AlignmentConstraint *c2 = info->alignment2;
    if (!c1->variable || !c2->variable)
    {
        throw InvalidConstraint(this);
    }
    vpsc::Constraint constraint(c1->variable, c2->variable, sep, true);
    alternatives.push_back(SubConstraint(_primaryDim, constraint));

    //fprintf(stderr, "===== DISTRIBUTION ALTERNATIVES -======\n");
    return alternatives;
}


void DistributionConstraint::generateSeparationConstraints(
        const vpsc::Dim dim, vpsc::Variables& vars, vpsc::Constraints& gcs) 
{
    if (dim == _primaryDim)
    {
        cs.clear();
        for (SubConstraintInfoList::iterator o = _subConstraintInfo.begin();
                o != _subConstraintInfo.end(); ++o) 
        {
            AlignmentPair *info = static_cast<AlignmentPair *> (*o);
            AlignmentConstraint *c1 = info->alignment1;
            AlignmentConstraint *c2 = info->alignment2;
            if (!c1->variable || !c2->variable)
            {
                throw InvalidConstraint(this);
            }
            vpsc::Constraint *c=new vpsc::Constraint(
                    c1->variable, c2->variable, sep, true);
            gcs.push_back(c);
            cs.push_back(c);
#if 0
            // The following was an experiment to allow variable distributions 
            // solved by optimisation rather than satisfying constraints
            if(isVariable) {
                // set second derivatives of:
                // (u + g - v)^2 = g^2 + 2gu + u^2 - 2gv - 2uv + v^2
                (*Q)[make_pair(c1->variable->id,c1->variable->id)]+=w;
                (*Q)[make_pair(c2->variable->id,c2->variable->id)]+=w;
                (*Q)[make_pair(variable->id,variable->id)]+=w;
                (*Q)[make_pair(c1->variable->id,c2->variable->id)]-=w;
                (*Q)[make_pair(c2->variable->id,c1->variable->id)]-=w;
                (*Q)[make_pair(c1->variable->id,variable->id)]+=w;
                (*Q)[make_pair(variable->id,c1->variable->id)]+=w;
                (*Q)[make_pair(c2->variable->id,variable->id)]-=w;
                (*Q)[make_pair(variable->id,c2->variable->id)]-=w;
            }
#endif
        }
    }
}


//-----------------------------------------------------------------------------
// PageBoundaryConstraint code
//-----------------------------------------------------------------------------

class ShapeOffsets : public SubConstraintInfo 
{
    public:
        ShapeOffsets(unsigned ind, double xOffset, double yOffset) :
            SubConstraintInfo(ind)
        {
            halfDim[0] = xOffset;
            halfDim[1] = yOffset;
        }
        double halfDim[2];  // half width and height values;
};


PageBoundaryConstraints::PageBoundaryConstraints(double lBoundary, 
        double rBoundary, double bBoundary, double tBoundary, double w)
    : CompoundConstraint(vpsc::HORIZONTAL)
{
    leftMargin[vpsc::XDIM] = lBoundary;
    rightMargin[vpsc::XDIM] = rBoundary;
    leftMargin[vpsc::YDIM] = bBoundary;
    rightMargin[vpsc::YDIM] = tBoundary;

    for (unsigned i = 0; i < 2; ++i)
    {
        actualLeftMargin[i] = leftMargin[i];
        actualRightMargin[i] = rightMargin[i];
        leftWeight[i] = w; 
        rightWeight[i] = w;
        vl[i] = NULL;
        vr[i] = NULL;
    }
}


SubConstraintAlternatives 
PageBoundaryConstraints::getCurrSubConstraintAlternatives(vpsc::Variables vs[])
{
    // Page boundary constraints do not need to be evaluated at the
    // time of makeFeasible, so we return an empty list here.
    return SubConstraintAlternatives();
}


void PageBoundaryConstraints::addShape(unsigned id, double halfW, double halfH)
{
    _subConstraintInfo.push_back(new ShapeOffsets(id, halfW, halfH));
}


void PageBoundaryConstraints::updatePosition(const vpsc::Dim dim)
{
    if (vl[dim]) 
    {
        actualLeftMargin[dim] = vl[dim]->finalPosition;
    }

    if (vr[dim])
    {
        actualRightMargin[dim] = vr[dim]->finalPosition;
    }
}


double PageBoundaryConstraints::getActualLeftMargin(const vpsc::Dim dim)
{
    return actualLeftMargin[dim];
}


double PageBoundaryConstraints::getActualRightMargin(const vpsc::Dim dim)
{
    return actualRightMargin[dim];
}


void PageBoundaryConstraints::generateVariables(const vpsc::Dim dim,
        vpsc::Variables& vars) 
{ 
    // create 2 dummy vars, based on the dimension we are in
    if (leftWeight[dim]) 
    {
        vars.push_back(vl[dim] = new vpsc::Variable(vars.size(), 
                    leftMargin[dim], leftWeight[dim]));
        vl[dim]->fixedDesiredPosition = true;
    }
    
    if (rightWeight[dim]) 
    {
        vars.push_back(vr[dim] = new vpsc::Variable(vars.size(), 
                    rightMargin[dim], rightWeight[dim]));
        vr[dim]->fixedDesiredPosition = true;
    }
}


void PageBoundaryConstraints::generateSeparationConstraints(
        const vpsc::Dim dim, vpsc::Variables& vs, vpsc::Constraints& cs) 
{
    // For each of the "real" variables, create a constraint that puts 
    // that var between our two new dummy vars, depending on the dimension.
    for (SubConstraintInfoList::iterator o = _subConstraintInfo.begin();
            o != _subConstraintInfo.end(); ++o) 
    {
        ShapeOffsets *info = static_cast<ShapeOffsets *> (*o);
        assertValidVariableIndex(vs, info->varIndex);
        if (vl[dim])
        {
            cs.push_back(new vpsc::Constraint(vl[dim], 
                    vs[info->varIndex], info->halfDim[dim]));
        }
        
        if (vr[dim])
        {
            cs.push_back(new vpsc::Constraint(vs[info->varIndex], 
                    vr[dim], info->halfDim[dim]));
        }
    }
}


//-----------------------------------------------------------------------------
// NonOverlapConstraint code
//-----------------------------------------------------------------------------

class ShapePairInfo 
{
    public:
        ShapePairInfo(unsigned ind1, unsigned ind2) 
            : satisfied(false),
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
            int processedInt = processed ? 1 : 0;
            int rhsProcessedInt = rhs.processed ? 1 : 0;
            // Make sure the processed ones are at the end after sorting.
            if (processedInt != rhsProcessedInt)
            {
                return processedInt < rhsProcessedInt;
            }
            return overlapMax > rhs.overlapMax;
        }
        unsigned varIndex1;
        unsigned varIndex2;
        bool satisfied;
        bool processed;
        double costL;
        double costR;
        double costB;
        double costA;
        double overlapMin;
        double overlapMax;
};


NonOverlapConstraints::NonOverlapConstraints()
    : CompoundConstraint(vpsc::HORIZONTAL, PRIORITY_NONOVERLAP)
{
}


void NonOverlapConstraints::addShape(unsigned id, double halfW, double halfH)
{
    COLA_ASSERT(id == shapeOffsets.size());
    shapeOffsets.push_back(ShapeOffsets(id, halfW, halfH));

    // Setup pairInfos for all other shapes. 
    for (size_t i = 0; i + 1 < shapeOffsets.size(); ++i)
    {
        pairInfoList.push_back(ShapePairInfo(i, id));
    }
}


void NonOverlapConstraints::computeAndSortOverlap(vpsc::Variables vs[])
{
    for (std::list<ShapePairInfo>::iterator curr = pairInfoList.begin();
            curr != pairInfoList.end(); ++curr)
    {
        ShapePairInfo& info = static_cast<ShapePairInfo&> (*curr);

        ShapeOffsets& shape1 = shapeOffsets[info.varIndex1];
        ShapeOffsets& shape2 = shapeOffsets[info.varIndex2];

        double xPos1 = vs[0][info.varIndex1]->finalPosition;
        double xPos2 = vs[0][info.varIndex2]->finalPosition;
        double yPos1 = vs[1][info.varIndex1]->finalPosition;
        double yPos2 = vs[1][info.varIndex2]->finalPosition;
        // If lr < 0, then left edge of shape1 is on the left 
        // of right edge of shape2.
        double spaceR = (xPos2 - shape2.halfDim[0]) - 
                (xPos1 + shape1.halfDim[0]);
        double spaceL = (xPos1 - shape1.halfDim[0]) - 
                (xPos2 + shape2.halfDim[0]);
        // Below
        double spaceA = (yPos2 - shape2.halfDim[1]) - 
                (yPos1 + shape1.halfDim[1]);
        // Above
        double spaceB = (yPos1 - shape1.halfDim[1]) - 
                (yPos2 + shape2.halfDim[1]);

        info.costL = info.costR = info.costB = info.costA = 0;
        info.overlapMax = 0;
        info.overlapMin = DBL_MAX;
        bool xOverlap = false;
        bool yOverlap = false;
        if ((spaceR < 0) && (spaceL < 0))
        {
            info.costL = std::max(-spaceL, 0.0);
            info.costR = std::max(-spaceR, 0.0);

            info.overlapMax = std::max(info.costL, info.costR);

            info.overlapMin = std::min(info.overlapMin, info.costL);
            info.overlapMin = std::min(info.overlapMin, info.costR);

            xOverlap = true;
        }
        if ((spaceB < 0) && (spaceA < 0))
        {
            info.costB = std::max(-spaceB, 0.0);
            info.costA = std::max(-spaceA, 0.0);

            info.overlapMax = std::max(info.overlapMax, info.costB);
            info.overlapMax = std::max(info.overlapMax, info.costA);

            info.overlapMin = std::min(info.overlapMin, info.costB);
            info.overlapMin = std::min(info.overlapMin, info.costA);

            yOverlap = true;
        }

        if (!xOverlap || !yOverlap)
        {
            // Overlap must occur in both dimensions.
            info.overlapMax = 0;
        }
        else
        {
            // There is overlap.
            //printf("[%02d][%02d] L %g, R %G, B %g, A %g\n", info.varIndex1, 
            //        info.varIndex2, spaceL, spaceR, spaceB, spaceA);
        }
    }
    pairInfoList.sort();
}


void NonOverlapConstraints::markCurrSubConstraintAsActive(const bool satisfiable)
{
    ShapePairInfo& info = pairInfoList.front();
    
    info.processed = true;
    info.satisfied = satisfiable;

    _currSubConstraintIndex++;
}



SubConstraintAlternatives 
NonOverlapConstraints::getCurrSubConstraintAlternatives(vpsc::Variables vs[])
{
    SubConstraintAlternatives alternatives;
    computeAndSortOverlap(vs);

    ShapePairInfo& info = pairInfoList.front();
    
    if (info.overlapMax == 0)
    {
        //fprintf(stderr, "===== EMPTY ALTERNATIVES -======\n");
        // There is no overlap here.
        // Mark as finished at this point.
        _currSubConstraintIndex = pairInfoList.size();
        return alternatives;
    }
    ShapeOffsets& shape1 = shapeOffsets[info.varIndex1];
    ShapeOffsets& shape2 = shapeOffsets[info.varIndex2];

    double xSep = shape1.halfDim[0] + shape2.halfDim[0];
    double ySep = shape1.halfDim[1] + shape2.halfDim[1];

    assertValidVariableIndex(vs[XDIM], info.varIndex1);
    assertValidVariableIndex(vs[XDIM], info.varIndex2);
    assertValidVariableIndex(vs[YDIM], info.varIndex1);
    assertValidVariableIndex(vs[YDIM], info.varIndex2);

    vpsc::Constraint constraintL(vs[XDIM][info.varIndex2], 
            vs[XDIM][info.varIndex1], xSep);
    alternatives.push_back(SubConstraint(XDIM, constraintL, info.costL));

    vpsc::Constraint constraintR(vs[XDIM][info.varIndex1], 
            vs[XDIM][info.varIndex2], xSep);
    alternatives.push_back(SubConstraint(XDIM, constraintR, info.costR));

    vpsc::Constraint constraintB(vs[YDIM][info.varIndex2], 
            vs[YDIM][info.varIndex1], ySep);
    alternatives.push_back(SubConstraint(YDIM, constraintB, info.costB));

    vpsc::Constraint constraintT(vs[YDIM][info.varIndex1], 
            vs[YDIM][info.varIndex2], ySep);
    alternatives.push_back(SubConstraint(YDIM, constraintT, info.costA));
    
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
}



void NonOverlapConstraints::generateVariables(const vpsc::Dim dim,
        vpsc::Variables& vars) 
{
}


void NonOverlapConstraints::generateSeparationConstraints(
        const vpsc::Dim dim, vpsc::Variables& vs, vpsc::Constraints& cs) 
{
}


//-----------------------------------------------------------------------------
// Support code
//-----------------------------------------------------------------------------


struct GenerateVariables 
{
    GenerateVariables(const vpsc::Dim dim, vpsc::Variables& vars) 
        : dim(dim),
          vars(vars) 
    {
    }
    void operator() (CompoundConstraint *c) 
    {
        c->generateVariables(dim, vars);
    }

    const vpsc::Dim dim;
    vpsc::Variables& vars;
};


struct GenerateSeparationConstraints 
{
    GenerateSeparationConstraints(const vpsc::Dim dim, vpsc::Variables& vars, 
            vpsc::Constraints& cs) 
        : dim(dim),
          vars(vars), 
          cs(cs) 
    {
    }
    void operator() (CompoundConstraint *c) 
    {
        c->generateSeparationConstraints(dim, vars, cs);
    }
    const vpsc::Dim dim;
    vpsc::Variables& vars;
    vpsc::Constraints& cs;
};


void generateVariablesAndConstraints(CompoundConstraints& ccs, 
        const vpsc::Dim dim, vpsc::Variables& vars, vpsc::Constraints& cs)
{
	for_each(ccs.begin(), ccs.end(), 
            GenerateVariables(dim, vars));
	for_each(ccs.begin(), ccs.end(), 
            GenerateSeparationConstraints(dim, vars, cs));
}


void generateVariables(CompoundConstraints& ccs, const vpsc::Dim dim, 
        vpsc::Variables& vars)
{
	for_each(ccs.begin(), ccs.end(), 
            GenerateVariables(dim, vars));
}


CompoundConstraint::CompoundConstraint(vpsc::Dim primaryDim,
        unsigned int priority) : 
    _primaryDim(primaryDim),
    _secondaryDim((vpsc::Dim) ((primaryDim + 1) % 2)),
    _priority(priority),
    _currSubConstraintIndex(0)
{
}


CompoundConstraint::~CompoundConstraint()
{
    // Free memory from the subConstraintInfo list.
    while (!_subConstraintInfo.empty())
    {
        delete _subConstraintInfo.back();
        _subConstraintInfo.pop_back();
    }
}


vpsc::Dim CompoundConstraint::dimension(void) const
{
    return _primaryDim;
}


unsigned int CompoundConstraint::priority(void) const
{
    return _priority;
}


void CompoundConstraint::printCreationCode(FILE *fp) const
{
    // Do nothing.  Subclasses can implement this.
}


std::list<unsigned> CompoundConstraint::subConstraintObjIndexes(void) const
{
    std::list<unsigned> idList;
    for (size_t i = 0; i < _subConstraintInfo.size(); ++i)
    {
        idList.push_back(_subConstraintInfo[i]->varIndex);
    }
    return idList;
}


bool CompoundConstraint::subConstraintsRemaining(void) const
{
    return _currSubConstraintIndex < _subConstraintInfo.size();
}


void CompoundConstraint::markAllSubConstraintsAsInactive(void)
{
    for (size_t i = 0; i < _subConstraintInfo.size(); ++i)
    {
        _subConstraintInfo[i]->satisfied = false;
    }
    _currSubConstraintIndex = 0;
}


void CompoundConstraint::markCurrSubConstraintAsActive(const bool satisfiable)
{
    _subConstraintInfo[_currSubConstraintIndex]->satisfied = satisfiable;

    _currSubConstraintIndex++;
}


void CompoundConstraint::assertValidVariableIndex(const vpsc::Variables& vars, 
        const unsigned index)
{
    if (index >= vars.size())
    {
        throw InvalidVariableIndexException(this, index);
    }
}


UnsatisfiableConstraintInfo::UnsatisfiableConstraintInfo(
        const vpsc::Constraint *c) 
    : vlid(c->left->id), 
      vrid(c->right->id), 
      gap(c->gap) 
{
}


} // namespace cola

// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=4:softtabstop=4 :

