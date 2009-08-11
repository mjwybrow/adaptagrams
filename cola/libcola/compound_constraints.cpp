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

#include <libvpsc/variable.h>
#include <libvpsc/constraint.h>
#include <libvpsc/assertions.h>

#include "cola.h"
#include "compound_constraints.h"
#include "exceptions.h"

using std::vector;

namespace cola {

static const double freeWeight = 0.0001;

//-----------------------------------------------------------------------------
// BoundaryConstraint code
//-----------------------------------------------------------------------------

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
        // Constrain the left objects to be to the left of the boundary.
        for (OffsetList::iterator o = leftOffsets.begin();
                o != leftOffsets.end(); ++o) 
        {
            assertValidVariableIndex(vars, o->first);
            cs.push_back(new vpsc::Constraint(
                    vars[o->first], variable, o->second));
        }
        // Constrain the right objects to be to the right of the boundary.
        for(OffsetList::iterator o = rightOffsets.begin();
                o != rightOffsets.end(); ++o) 
        {
            assertValidVariableIndex(vars, o->first);
            cs.push_back(new vpsc::Constraint(
                    variable, vars[o->first], o->second));
        }
    }
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
        for (OffsetList::iterator o = offsets.begin();
                o != offsets.end(); ++o) 
        {
            assertValidVariableIndex(vars, o->first);
            cs.push_back(new vpsc::Constraint(
                        variable, vars[o->first], o->second, true));
        }
    }
}


//-----------------------------------------------------------------------------
// SeparationConstraint code
//-----------------------------------------------------------------------------

SeparationConstraint::SeparationConstraint(const vpsc::Dim dim, 
        unsigned l, unsigned r, double g, bool equality)
    : CompoundConstraint(dim),
      left(l),
      right(r), 
      al(NULL), 
      ar(NULL), 
      gap(g), 
      equality(equality),
      vpscConstraint(NULL)
{
}


SeparationConstraint::SeparationConstraint(const vpsc::Dim dim, 
        AlignmentConstraint *l, AlignmentConstraint *r, double g, 
        bool equality) 
    : CompoundConstraint(dim),
      left(0),
      right(0),
      al(l),
      ar(r),
      gap(g),
      equality(equality)
{
}


void SeparationConstraint::generateVariables(const vpsc::Dim dim, 
        vpsc::Variables& vars) 
{
    // No additional variables are required!
}


void SeparationConstraint::generateSeparationConstraints(const vpsc::Dim dim,
            vpsc::Variables& vs, vpsc::Constraints& cs) 
{
    if (dim == _primaryDim)
    {
        if(al) 
        {
            left = al->variable->id;
        }
        if(ar) 
        {
            right = ar->variable->id;
        }
        assertValidVariableIndex(vs, left);
        assertValidVariableIndex(vs, right);
        vpscConstraint = 
                new vpsc::Constraint(vs[left], vs[right], gap, equality);
        cs.push_back(vpscConstraint);
    }
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

MultiSeparationConstraint::MultiSeparationConstraint(const vpsc::Dim dim, 
        double minSep, bool equality)
    : CompoundConstraint(dim),
      sep(minSep), 
      equality(equality)
{ 
}


void MultiSeparationConstraint::generateVariables(const vpsc::Dim dim, 
        vpsc::Variables& vars)
{
    // No additional variables are required!
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
        for (vector<std::pair<
                AlignmentConstraint*,AlignmentConstraint*> >::iterator iac
                = acs.begin(); iac != acs.end(); ++iac)
        {
            AlignmentConstraint *c1 = iac->first;
            AlignmentConstraint *c2 = iac->second;
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


void DistributionConstraint::generateVariables(const vpsc::Dim dim, 
        vpsc::Variables& vars)
{
    // No additional variables are required!
}


void DistributionConstraint::setSeparation(double sep) 
{
    this->sep = sep;
}


void DistributionConstraint::generateSeparationConstraints(
        const vpsc::Dim dim, vpsc::Variables& vars, vpsc::Constraints& gcs) 
{
    if (dim == _primaryDim)
    {
        cs.clear();
        for(vector<std::pair<
                AlignmentConstraint*,AlignmentConstraint*> >::iterator iac
                =acs.begin(); iac!=acs.end();++iac) {
            AlignmentConstraint *c1, *c2;
            c1=iac->first;
            c2=iac->second;
            if (!c1->variable || !c2->variable)
            {
                throw InvalidConstraint(this);
            }
            vpsc::Constraint *c=new vpsc::Constraint(
                    c1->variable, c2->variable, sep, true);
            gcs.push_back(c);
            cs.push_back(c);
        /*
            //The following was an experiment to allow variable distributions 
            //solved by optimisation rather than satisfying constraints
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
        */
        }
    }
}


//-----------------------------------------------------------------------------
// PageBoundaryConstraint code
//-----------------------------------------------------------------------------

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


void PageBoundaryConstraints::addContainedShape(unsigned id, 
        double halfW, double halfH)
{
    offsets[vpsc::XDIM].push_back(std::make_pair(id, halfW));
    offsets[vpsc::YDIM].push_back(std::make_pair(id, halfH));
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
    for (OffsetList::iterator o = offsets[dim].begin(); 
            o != offsets[dim].end(); ++o)
    {
        assertValidVariableIndex(vs, o->first);
        if (vl[dim])
        {
            cs.push_back(
                    new vpsc::Constraint(vl[dim], vs[o->first], o->second));
        }
        
        if (vr[dim])
        {
            cs.push_back(
                    new vpsc::Constraint(vs[o->first], vr[dim], o->second));
        }
    }
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


CompoundConstraint::CompoundConstraint(vpsc::Dim primaryDim)
    : _primaryDim(primaryDim),
      _secondaryDim((vpsc::Dim) ((primaryDim + 1) % 2))
{
}


vpsc::Dim CompoundConstraint::dimension(void) const
{
    return _primaryDim;
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

