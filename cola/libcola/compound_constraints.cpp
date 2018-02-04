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
 * See the file LICENSE.LGPL distributed with the library.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  
 *
 * Author(s):  Tim Dwyer
 *             Michael Wybrow
*/

#include <algorithm>
#include <climits>
#include <sstream>

#include <libvpsc/variable.h>
#include <libvpsc/constraint.h>
#include <libvpsc/assertions.h>

#include "libcola/cola.h"
#include "libcola/compound_constraints.h"
#include "libcola/exceptions.h"

using std::vector;
using vpsc::XDIM;
using vpsc::YDIM;

namespace cola {

void SubConstraintInfo::updateVarIDsWithMapping(
        const VariableIDMap& idMap, bool forward)
{
    varIndex = idMap.mappingForVariable(varIndex, forward);
}

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
      variable(nullptr) 
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


void BoundaryConstraint::printCreationCode(FILE *fp) const
{
    fprintf(fp, "    BoundaryConstraint *boundary%llu = "
            "new BoundaryConstraint(vpsc::%cDIM);\n",
            (unsigned long long) this, (_primaryDim == 0) ? 'X' : 'Y');
    for (SubConstraintInfoList::const_iterator o = _subConstraintInfo.begin();
            o != _subConstraintInfo.end(); ++o) 
    {
        Offset *info = static_cast<Offset *> (*o);
        fprintf(fp, "    boundary%llu->addShape(%u, %g);\n",
                (unsigned long long) this, info->varIndex, info->distOffset);
    }
    fprintf(fp, "    ccs.push_back(boundary%llu);\n\n",
            (unsigned long long) this);
}


std::string BoundaryConstraint::toString(void) const
{
    std::ostringstream stream;
    stream << "BoundaryConstraint(";
    stream << "dim: " << ((_primaryDim == 0) ? 'X' : 'Y');
    stream << "): {";
    bool first = true;
    for (SubConstraintInfoList::const_iterator o = _subConstraintInfo.begin();
            o != _subConstraintInfo.end(); ++o)
    {
        Offset *info = static_cast<Offset *> (*o);
        if (!first)
        {
            stream << ", ";
        }
        stream << "(" << "rect: " << info->varIndex << ", offset: " << info->distOffset << ")";
        first = false;
    }
    stream << "}";
    return stream.str();
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
        vpsc::Variables& vars, vpsc::Constraints& cs,
        vpsc::Rectangles& bbs) 
{
    COLA_UNUSED(bbs);
    if (dim == _primaryDim)
    {
        COLA_ASSERT(variable != nullptr);
        for (SubConstraintInfoList::iterator o = _subConstraintInfo.begin();
                o != _subConstraintInfo.end(); ++o) 
        {
            Offset *info = static_cast<Offset *> (*o);
            assertValidVariableIndex(vars, info->varIndex);
            vpsc::Constraint *constraint = nullptr;
            if (info->distOffset < 0)
            {
                // Constrain the objects with negative offsets to be 
                // to the left of the boundary.
                constraint = new vpsc::Constraint(vars[info->varIndex],
                        variable, -info->distOffset);
            }
            else
            {
                // Constrain the objects with positive offsets to be 
                // to the right of the boundary.
                constraint = new vpsc::Constraint(variable,
                        vars[info->varIndex], info->distOffset);
            }
            constraint->creator = this;
            cs.push_back(constraint);
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
      indicator(nullptr),
      variable(nullptr),
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
        vpsc::Variables& vars, vpsc::Constraints& cs,
        vpsc::Rectangles& bbs) 
{
    COLA_UNUSED(bbs);
    if (dim == _primaryDim)
    {
        COLA_ASSERT(variable != nullptr);
        // Constrain each object to be offset from the guideline by
        // some exact amount.
        for (SubConstraintInfoList::iterator o = _subConstraintInfo.begin();
                o != _subConstraintInfo.end(); ++o) 
        {
            Offset *info = static_cast<Offset *> (*o);
            assertValidVariableIndex(vars, info->varIndex);
            vpsc::Constraint *constraint = new vpsc::Constraint(
                        variable, vars[info->varIndex], info->distOffset, true);
            constraint->creator = this;
            cs.push_back(constraint);
        }
    }
}

void AlignmentConstraint::updateShapeOffsetsForDifferentCentres(
        const std::vector<double>& offsets, bool forward)
{
    for (SubConstraintInfoList::iterator o = _subConstraintInfo.begin();
            o != _subConstraintInfo.end(); ++o) 
    {
        Offset *info = static_cast<Offset *> (*o);
        if (offsets[info->varIndex] == 0)
        {
            continue;
        }

        if (forward)
        {
            info->distOffset -= offsets[info->varIndex];
        }
        else
        {
            info->distOffset += offsets[info->varIndex];
        }
    }
}

void AlignmentConstraint::printCreationCode(FILE *fp) const
{
    fprintf(fp, "    AlignmentConstraint *alignment%llu = "
            "new AlignmentConstraint(vpsc::%cDIM, %g);\n",
            (unsigned long long) this, (_primaryDim == 0) ? 'X' : 'Y', 
            _position);
    if (_isFixed)
    {
        fprintf(fp, "    alignment%llu->fixPos(%g);\n",
                (unsigned long long) this, _position);
    }
    for (SubConstraintInfoList::const_iterator o = _subConstraintInfo.begin();
            o != _subConstraintInfo.end(); ++o) 
    {
        Offset *info = static_cast<Offset *> (*o);
        fprintf(fp, "    alignment%llu->addShape(%u, %g);\n",
                (unsigned long long) this, info->varIndex, info->distOffset);
    }
    fprintf(fp, "    ccs.push_back(alignment%llu);\n\n",
            (unsigned long long) this);
}


std::string AlignmentConstraint::toString(void) const
{
    std::ostringstream stream;
    stream << "AlignmentConstraint(";
    stream << "dim: " << ((_primaryDim == 0) ? 'X' : 'Y');
    stream << ", pos: " << _position;
    if (_isFixed)
    {
        stream << ", fixed: true";
    }
#if 0
    // TODO: buggy.  Not the right value.
    if (variable)
    {
        stream << ", varInd: " << variable->id;
    }
#endif
    stream << "): {";
    bool first = true;
    for (SubConstraintInfoList::const_iterator o = _subConstraintInfo.begin();
            o != _subConstraintInfo.end(); ++o)
    {
        Offset *info = static_cast<Offset *> (*o);
        if (!first)
        {
            stream << ", ";
        }
        stream << "(" << "rect: " << info->varIndex << ", offset: " << info->distOffset << ")";
        first = false;
    }
    stream << "}";
    return stream.str();
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
              lConstraint(nullptr),
              rConstraint(nullptr),
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
            return (lConstraint) ? 
                    (unsigned) lConstraint->variable->id : varIndex;
        }
        unsigned indexR(void) const
        {
            return (rConstraint) ? 
                    (unsigned) rConstraint->variable->id : varIndex2;
        }
        void updateVarIDsWithMapping(const VariableIDMap& idMap, bool forward)
        {
            varIndex = idMap.mappingForVariable(varIndex, forward);
            varIndex2 = idMap.mappingForVariable(varIndex2, forward);
        }

        AlignmentConstraint *lConstraint;
        AlignmentConstraint *rConstraint;
        unsigned varIndex2;
};


SeparationConstraint::SeparationConstraint(const vpsc::Dim dim, 
        unsigned l, unsigned r, double g, bool equality)
    : CompoundConstraint(dim),
      gap(g), 
      equality(equality),
      vpscConstraint(nullptr)
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


void SeparationConstraint::printCreationCode(FILE *fp) const
{
    assert(_subConstraintInfo.size() == 1);
    VarIndexPair *varIndexPair = (VarIndexPair *) _subConstraintInfo.front();
    if (varIndexPair->lConstraint && varIndexPair->rConstraint)
    {
        fprintf(fp, "    SeparationConstraint *separation%llu = "
                "new SeparationConstraint(vpsc::%cDIM, alignment%llu, "
                "alignment%llu, %g, %s);\n",
                (unsigned long long) this, (_primaryDim == 0) ? 'X' : 'Y', 
                (unsigned long long) varIndexPair->lConstraint, 
                (unsigned long long) varIndexPair->rConstraint, gap, 
                (equality) ? "true" : "false");
    }
    else
    {
        fprintf(fp, "    SeparationConstraint *separation%llu = "
                "new SeparationConstraint(vpsc::%cDIM, %u, %u, %g, %s);\n",
                (unsigned long long) this, (_primaryDim == 0) ? 'X' : 'Y', 
                varIndexPair->indexL(), varIndexPair->indexR(), gap, 
                (equality) ? "true" : "false");
    }
    fprintf(fp, "    ccs.push_back(separation%llu);\n\n",
            (unsigned long long) this);
}


std::string SeparationConstraint::toString(void) const
{
    std::ostringstream stream;
    stream << "SeparationConstraint(";
    stream << "dim: " << ((_primaryDim == 0) ? 'X' : 'Y');
    stream << ", sep: " << gap;
    stream << ", equality: " << ((equality) ? "true" : "false");
    stream << "): {";
    VarIndexPair *varIndexPair = (VarIndexPair *) _subConstraintInfo.front();
    if (varIndexPair->lConstraint && varIndexPair->rConstraint)
    {
        stream << "(alignment: " << varIndexPair->indexL() << "), ";
        stream << "(alignment: " << varIndexPair->indexR() << "), ";
    }
    else
    {
        stream << "(rect: " << varIndexPair->indexL() << "), ";
        stream << "(rect: " << varIndexPair->indexR() << "), ";
    }
    stream << "}";
    return stream.str();
}


void SeparationConstraint::generateVariables(const vpsc::Dim dim, 
        vpsc::Variables& vars) 
{
    COLA_UNUSED(dim);
    COLA_UNUSED(vars);

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
            vpsc::Variables& vs, vpsc::Constraints& cs,
            vpsc::Rectangles& bbs) 
{
    COLA_UNUSED(bbs);
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
        vpscConstraint->creator = this;
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
    if (vpscConstraint != nullptr) 
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
      vpscConstraint(nullptr)
{
}


void OrthogonalEdgeConstraint::generateVariables(const vpsc::Dim dim, 
        vpsc::Variables& vars) 
{
    COLA_UNUSED(dim);
    COLA_UNUSED(vars);

    // No additional variables are required!
}


void OrthogonalEdgeConstraint::printCreationCode(FILE *fp) const
{
    fprintf(fp, "    /* OrthogonalEdgeConstraint *orthogonal%llu = nullptr; */\n\n",
            (unsigned long long) this);
}


std::string OrthogonalEdgeConstraint::toString(void) const
{
    std::ostringstream stream;
    stream << "OrthogonalEdgeConstraint()";
    return stream.str();
}



SubConstraintAlternatives 
OrthogonalEdgeConstraint::getCurrSubConstraintAlternatives(
        vpsc::Variables vs[])
{
    COLA_UNUSED(vs);

    // XXX: What to do here?
    _currSubConstraintIndex = _subConstraintInfo.size();
    return SubConstraintAlternatives();
}


void OrthogonalEdgeConstraint::generateSeparationConstraints(
        const vpsc::Dim dim, vpsc::Variables& vs, vpsc::Constraints& cs,
        vpsc::Rectangles& bbs) 
{
    COLA_UNUSED(bbs);
    if (dim == _primaryDim)
    {
        assertValidVariableIndex(vs, left);
        assertValidVariableIndex(vs, right);
        vpscConstraint = new vpsc::Constraint(vs[left], vs[right], 0, true);
        vpscConstraint->creator = this;
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
        if ( ((rMin >= minBound) && (rMin <= maxBound)) || 
             ((rMax >= minBound) && (rMax <= maxBound))) 
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
      indicator(nullptr),
      sep(minSep), 
      equality(equality)
{ 
}


void MultiSeparationConstraint::addAlignmentPair(AlignmentConstraint *ac1,
        AlignmentConstraint *ac2)
{
    _subConstraintInfo.push_back(new AlignmentPair(ac1, ac2));
}


void MultiSeparationConstraint::printCreationCode(FILE *fp) const
{
    fprintf(fp, "    MultiSeparationConstraint *multiSep%llu = "
            "new MultiSeparationConstraint(vpsc::%cDIM, %g, %s);\n",
            (unsigned long long) this, (_primaryDim == 0) ? 'X' : 'Y', sep,
            (equality) ? "true" : "false");
    for (SubConstraintInfoList::const_iterator o = _subConstraintInfo.begin();
            o != _subConstraintInfo.end(); ++o) 
    {
        AlignmentPair *pair = static_cast<AlignmentPair *> (*o);
        fprintf(fp, "    multiSep%llu->addAlignmentPair("
                "alignment%llu, alignment%llu);\n",
                (unsigned long long) this, 
                (unsigned long long) pair->alignment1, 
                (unsigned long long) pair->alignment2);
    }
    fprintf(fp, "    ccs.push_back(multiSep%llu);\n\n",
            (unsigned long long) this);
}


std::string MultiSeparationConstraint::toString(void) const
{
    std::ostringstream stream;
    stream << "MultiSeparationConstraint(";
    stream << "dim: " << ((_primaryDim == 0) ? 'X' : 'Y');
    stream << ", sep: " << sep;
    stream << ", equality: " << ((equality) ? "true" : "false");
    stream << "): {";
    bool first = true;
    for (SubConstraintInfoList::const_iterator o = _subConstraintInfo.begin();
            o != _subConstraintInfo.end(); ++o)
    {
        if (!first)
        {
            stream << ", ";
        }
        AlignmentPair *pair = static_cast<AlignmentPair *> (*o);
        stream << "(alignment: " << pair->alignment1->variable->id
               << ", alignment: " << pair->alignment2->variable->id << ")";
        first = false;
    }
    stream << "}";
    return stream.str();
}


void MultiSeparationConstraint::generateVariables(const vpsc::Dim dim, 
        vpsc::Variables& vars)
{
    COLA_UNUSED(dim);
    COLA_UNUSED(vars);

    // No additional variables are required!
}


SubConstraintAlternatives 
MultiSeparationConstraint::getCurrSubConstraintAlternatives(
        vpsc::Variables vs[])
{
    COLA_UNUSED(vs);

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
        const vpsc::Dim dim, vpsc::Variables& vs, vpsc::Constraints& gcs,
        vpsc::Rectangles& bbs) 
{
    COLA_UNUSED(vs);
    COLA_UNUSED(bbs);

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
            c->creator = this;
            gcs.push_back(c);
            cs.push_back(c);
        }
    }
}


//-----------------------------------------------------------------------------
// DistributionConstraint code
//-----------------------------------------------------------------------------

DistributionConstraint::DistributionConstraint(const vpsc::Dim dim)
    : CompoundConstraint(dim),
      indicator(nullptr)
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
    COLA_UNUSED(dim);
    COLA_UNUSED(vars);

    // No additional variables are required!
}


void DistributionConstraint::setSeparation(double sep) 
{
    this->sep = sep;
}


void DistributionConstraint::printCreationCode(FILE *fp) const
{
    fprintf(fp, "    DistributionConstraint *distribution%llu = "
            "new DistributionConstraint(vpsc::%cDIM);\n",
            (unsigned long long) this, (_primaryDim == 0) ? 'X' : 'Y');
    fprintf(fp, "    distribution%llu->setSeparation(%g);\n",
            (unsigned long long) this, sep);
    for (SubConstraintInfoList::const_iterator o = _subConstraintInfo.begin();
            o != _subConstraintInfo.end(); ++o) 
    {
        AlignmentPair *pair = static_cast<AlignmentPair *> (*o);
        fprintf(fp, "    distribution%llu->addAlignmentPair("
                "alignment%llu, alignment%llu);\n",
                (unsigned long long) this, 
                (unsigned long long) pair->alignment1, 
                (unsigned long long) pair->alignment2);
    }
    fprintf(fp, "    ccs.push_back(distribution%llu);\n\n",
            (unsigned long long) this);
}


std::string DistributionConstraint::toString(void) const
{
    std::ostringstream stream;
    stream << "DistributionConstraint(";
    stream << "dim: " << ((_primaryDim == 0) ? 'X' : 'Y');
    stream << ", sep: " << sep;
    stream << "): {";
    bool first = true;
    for (SubConstraintInfoList::const_iterator o = _subConstraintInfo.begin();
            o != _subConstraintInfo.end(); ++o)
    {
        if (!first)
        {
            stream << ", ";
        }
        AlignmentPair *pair = static_cast<AlignmentPair *> (*o);
        stream << "(alignment: " << pair->alignment1->variable->id
               << ", alignment: " << pair->alignment2->variable->id << ")";
        first = false;
    }
    stream << "}";
    return stream.str();
}


SubConstraintAlternatives 
DistributionConstraint::getCurrSubConstraintAlternatives(vpsc::Variables vs[])
{
    COLA_UNUSED(vs);

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
        const vpsc::Dim dim, vpsc::Variables& vars, vpsc::Constraints& gcs,
        vpsc::Rectangles& bbs)
{
    COLA_UNUSED(vars);
    COLA_UNUSED(bbs);

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
            c->creator = this;
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
// FixedRelativeConstraint code
//-----------------------------------------------------------------------------

class RelativeOffset : public SubConstraintInfo
{
    public:
        RelativeOffset(unsigned indL, unsigned indR, vpsc::Dim dim,
                double offset)
            : SubConstraintInfo(indL),
              varIndex2(indR),
              dim(dim),
              distOffset(offset)
        {
        }
        void updateVarIDsWithMapping(const VariableIDMap& idMap, bool forward)
        {
            varIndex = idMap.mappingForVariable(varIndex, forward);
            varIndex2 = idMap.mappingForVariable(varIndex2, forward);
        }

        unsigned varIndex2;
        vpsc::Dim dim;
        double distOffset;
};


FixedRelativeConstraint::FixedRelativeConstraint(const vpsc::Rectangles& rs,
        std::vector<unsigned> shapeIds, const bool fixedPosition)
    : CompoundConstraint(vpsc::XDIM),
      m_fixed_position(fixedPosition),
      m_shape_vars(shapeIds)
{
    _combineSubConstraints = true;

    // Make sure that m_shape_vars doesn't contain duplicates.
    std::sort(m_shape_vars.begin(), m_shape_vars.end());
    std::vector<unsigned>::iterator last = 
            std::unique(m_shape_vars.begin(), m_shape_vars.end());
    m_shape_vars.erase(last, m_shape_vars.end());

    unsigned firstId = UINT_MAX;
    COLA_ASSERT(m_shape_vars.size() >= 2);
    for (std::vector<unsigned>::iterator it = m_shape_vars.begin();
            it != m_shape_vars.end(); ++it)
    {
        COLA_ASSERT(*it < rs.size());

        if (it == m_shape_vars.begin())
        {
            firstId = *it;
        }
        else
        {
            unsigned thisId = *it;
            _subConstraintInfo.push_back(
                    new RelativeOffset(firstId, thisId, vpsc::XDIM,
                        rs[thisId]->getCentreX() - rs[firstId]->getCentreX()));
            _subConstraintInfo.push_back(
                    new RelativeOffset(firstId, thisId, vpsc::YDIM,
                        rs[thisId]->getCentreY() - rs[firstId]->getCentreY()));
        }
    }
}

void FixedRelativeConstraint::updateVarIDsWithMapping(
        const VariableIDMap& idMap, bool forward)
{
    CompoundConstraint::updateVarIDsWithMapping(idMap, forward);

    // We need to map variables in m_shape_vars too, 
    // since this is used in generateVariables().
    for (size_t i = 0; i < m_shape_vars.size(); ++i)
    {
        m_shape_vars[i] = idMap.mappingForVariable(m_shape_vars[i], forward);
    }
}



void FixedRelativeConstraint::generateVariables(const vpsc::Dim dim, 
        vpsc::Variables& vars)
{
    COLA_UNUSED(dim);
    COLA_UNUSED(vars);

    // No additional variables are required!
    
    // Fix shape positions if required.
    if (m_fixed_position)
    {
        for (std::vector<unsigned>::iterator it = m_shape_vars.begin();
                it != m_shape_vars.end(); ++it)
        {
            vars[*it]->fixedDesiredPosition = true;
            vars[*it]->weight = 100000;
        }
    }
}


void FixedRelativeConstraint::printCreationCode(FILE *fp) const
{
    fprintf(fp, "    std::vector<unsigned> fixedRelativeSet%llu;\n",
            (unsigned long long) this);
    for (std::vector<unsigned>::const_iterator it = m_shape_vars.begin();
            it != m_shape_vars.end(); ++it)
    {
        fprintf(fp, "    fixedRelativeSet%llu.push_back(%u);\n",
                (unsigned long long) this, *it);
    }
    fprintf(fp, "    FixedRelativeConstraint *fixedRelative%llu = "
            "new FixedRelativeConstraint(rs, fixedRelativeSet%llu, %s);\n",
            (unsigned long long) this, (unsigned long long) this,
            (m_fixed_position) ? "true" : "false");
    fprintf(fp, "    ccs.push_back(fixedRelative%llu);\n\n",
            (unsigned long long) this);
}


std::string FixedRelativeConstraint::toString(void) const
{
    std::ostringstream stream;
    stream << "FixedRelativeConstraint(";
    stream << "fixedPos: " << ((m_fixed_position) ? "true" : "false");
    stream << "): {";
    bool first = true;
    for (std::vector<unsigned>::const_iterator it = m_shape_vars.begin();
            it != m_shape_vars.end(); ++it)
    {
        if (!first)
        {
            stream << ", ";
        }
        stream << "(rect: " << *it << ")";
        first = false;
    }
    stream << "}";
    return stream.str();
}


SubConstraintAlternatives 
FixedRelativeConstraint::getCurrSubConstraintAlternatives(vpsc::Variables vs[])
{
    COLA_UNUSED(vs);

    SubConstraintAlternatives alternatives;

    RelativeOffset *offset = static_cast<RelativeOffset *> 
            (_subConstraintInfo[_currSubConstraintIndex]);
        
    vpsc::Dim dim = offset->dim;
    vpsc::Constraint constraint(vs[dim][offset->varIndex], 
            vs[dim][offset->varIndex2], offset->distOffset, true);
    alternatives.push_back(SubConstraint(dim, constraint));
    
    //fprintf(stderr, "===== FixedRelativeConstraint ALTERNATIVES -======\n");
    return alternatives;
}


void FixedRelativeConstraint::generateSeparationConstraints(
        const vpsc::Dim dim, vpsc::Variables& vars, vpsc::Constraints& ccs,
        vpsc::Rectangles& bbs)
{
    COLA_UNUSED(vars);
    COLA_UNUSED(bbs);

    for (SubConstraintInfoList::iterator o = _subConstraintInfo.begin();
            o != _subConstraintInfo.end(); ++o) 
    {
        RelativeOffset *offset = static_cast<RelativeOffset *> (*o);
        if (dim != offset->dim)
        {
            continue;
        }

        assertValidVariableIndex(vars, offset->varIndex);
        assertValidVariableIndex(vars, offset->varIndex2);
        vpsc::Constraint *c = 
                new vpsc::Constraint(vars[offset->varIndex], 
                        vars[offset->varIndex2], offset->distOffset, true);
        c->creator = this;
        ccs.push_back(c);
    }
}


//-----------------------------------------------------------------------------
// PageBoundaryConstraint code
//-----------------------------------------------------------------------------

class PageBoundaryShapeOffsets : public SubConstraintInfo
{
    public:
        PageBoundaryShapeOffsets(unsigned ind, double xOffset, double yOffset) :
            SubConstraintInfo(ind)
        {
            halfDim[0] = xOffset;
            halfDim[1] = yOffset;
        }
        double halfDim[2];  // half width and height values;
};


PageBoundaryConstraints::PageBoundaryConstraints(double xLow, double xHigh,
        double yLow, double yHigh, double weight)
    : CompoundConstraint(vpsc::HORIZONTAL)
{
    COLA_ASSERT(xLow < xHigh);
    COLA_ASSERT(yLow < yHigh);

    leftMargin[vpsc::XDIM]  = xLow;
    rightMargin[vpsc::XDIM] = xHigh;
    leftMargin[vpsc::YDIM]  = yLow;
    rightMargin[vpsc::YDIM] = yHigh;

    for (unsigned i = 0; i < 2; ++i)
    {
        actualLeftMargin[i]  = leftMargin[i];
        actualRightMargin[i] = rightMargin[i];
        leftWeight[i]  = weight; 
        rightWeight[i] = weight;
        vl[i] = nullptr;
        vr[i] = nullptr;
    }
}


void PageBoundaryConstraints::addShape(unsigned id, double halfW, double halfH)
{
    _subConstraintInfo.push_back(new PageBoundaryShapeOffsets(id, halfW, halfH));
}


void PageBoundaryConstraints::printCreationCode(FILE *fp) const
{
    fprintf(fp, "    PageBoundaryConstraints *pageBoundary%llu = "
            "new PageBoundaryConstraints(%g, %g, %g, %g, %g);\n",
            (unsigned long long) this, leftMargin[vpsc::XDIM], 
            rightMargin[vpsc::XDIM], leftMargin[vpsc::YDIM],
            rightMargin[vpsc::YDIM], leftWeight[0]);
    for (SubConstraintInfoList::const_iterator o = _subConstraintInfo.begin();
            o != _subConstraintInfo.end(); ++o) 
    {
        PageBoundaryShapeOffsets *offsets = 
                static_cast<PageBoundaryShapeOffsets *> (*o);
        fprintf(fp, "    pageBoundary%llu->addShape(%u, %g, %g);\n",
                (unsigned long long) this, offsets->varIndex,
                offsets->halfDim[XDIM], offsets->halfDim[YDIM]);
    }
    fprintf(fp, "    ccs.push_back(pageBoundary%llu);\n\n",
            (unsigned long long) this);
}


std::string PageBoundaryConstraints::toString(void) const
{
    std::ostringstream stream;
    stream << "PageBoundaryConstraints(";
    stream << "xLow: " << leftMargin[vpsc::XDIM];
    stream << ", xHigh: " << rightMargin[vpsc::XDIM];
    stream << ", yLow: " << leftMargin[vpsc::YDIM];
    stream << ", yHigh: " << rightMargin[vpsc::YDIM];
    stream << ", weight: " << leftWeight[0];
    stream << "): {";
    bool first = true;
    for (SubConstraintInfoList::const_iterator o = _subConstraintInfo.begin();
            o != _subConstraintInfo.end(); ++o)
    {
        if (!first)
        {
            stream << ", ";
        }
        PageBoundaryShapeOffsets *offsets =
                static_cast<PageBoundaryShapeOffsets *> (*o);
        stream << "(rect: " << offsets->varIndex;
        stream << ", halfWidth: " << offsets->halfDim[XDIM];
        stream << ", halfHeight: " << offsets->halfDim[YDIM];
        stream <<")";
        first = false;
    }
    stream << "}";
    return stream.str();
}

SubConstraintAlternatives 
PageBoundaryConstraints::getCurrSubConstraintAlternatives(vpsc::Variables vs[])
{
    COLA_UNUSED(vs);

    // Page boundary constraints do not need to be evaluated at the
    // time of makeFeasible, so we return an empty list here.
    _currSubConstraintIndex = _subConstraintInfo.size();
    return SubConstraintAlternatives();
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
        const vpsc::Dim dim, vpsc::Variables& vs, vpsc::Constraints& cs,
        vpsc::Rectangles& bbs) 
{
    COLA_UNUSED(bbs);
    // For each of the "real" variables, create a constraint that puts 
    // that var between our two new dummy vars, depending on the dimension.
    for (SubConstraintInfoList::iterator o = _subConstraintInfo.begin();
            o != _subConstraintInfo.end(); ++o) 
    {
        PageBoundaryShapeOffsets *info =
                static_cast<PageBoundaryShapeOffsets *> (*o);
        assertValidVariableIndex(vs, info->varIndex);
        if (vl[dim])
        {
            vpsc::Constraint *constraint = new vpsc::Constraint(vl[dim],
                    vs[info->varIndex], info->halfDim[dim]);
            constraint->creator = this;
            cs.push_back(constraint);
        }
        
        if (vr[dim])
        {
            vpsc::Constraint *constraint = new vpsc::Constraint(
                    vs[info->varIndex], vr[dim], info->halfDim[dim]);
            constraint->creator = this;
            cs.push_back(constraint);
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
            vpsc::Constraints& cs, vpsc::Rectangles& bbs) 
        : dim(dim),
          vars(vars), 
          cs(cs),
          bbs(bbs)
    {
    }
    void operator() (CompoundConstraint *c) 
    {
        c->generateSeparationConstraints(dim, vars, cs, bbs);
    }
    const vpsc::Dim dim;
    vpsc::Variables& vars;
    vpsc::Constraints& cs;
    vpsc::Rectangles& bbs;
};


void generateVariablesAndConstraints(CompoundConstraints& ccs, 
        const vpsc::Dim dim, vpsc::Variables& vars, vpsc::Constraints& cs,
        vpsc::Rectangles& bbs)
{
    for_each(ccs.begin(), ccs.end(), 
            GenerateVariables(dim, vars));
    for_each(ccs.begin(), ccs.end(), 
            GenerateSeparationConstraints(dim, vars, cs, bbs));
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
    _combineSubConstraints(false),
    _currSubConstraintIndex(0)
{
}


CompoundConstraint::~CompoundConstraint()
{
    // Free memory from the subConstraintInfo list.
    for_each(_subConstraintInfo.begin(), _subConstraintInfo.end(), delete_object());
    _subConstraintInfo.clear();
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
    COLA_UNUSED(fp);

    // Do nothing.  Subclasses can implement this.
}

void CompoundConstraint::updateVarIDsWithMapping(const VariableIDMap& idMap,
        bool forward)
{
    for (SubConstraintInfoList::iterator o = _subConstraintInfo.begin();
            o != _subConstraintInfo.end(); ++o) 
    {
        (*o)->updateVarIDsWithMapping(idMap, forward);
    }
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


bool CompoundConstraint::shouldCombineSubConstraints(void) const
{
    return _combineSubConstraints;
}


UnsatisfiableConstraintInfo::UnsatisfiableConstraintInfo(
        const vpsc::Constraint *c) 
    : leftVarIndex(c->left->id),
      rightVarIndex(c->right->id),
      separation(c->gap),
      equality(c->equality),
      cc((CompoundConstraint *)c->creator)
{
}

// Variable mapping.

VariableIDMap::VariableIDMap()
{
}

VariableIDMap::~VariableIDMap()
{
}

typedef std::pair<unsigned, unsigned> IDPair;
typedef std::list<IDPair> IDPairList;

bool VariableIDMap::addMappingForVariable(const unsigned from, 
        const unsigned to)
{
    bool found = false;
    for (IDPairList::iterator it = m_mapping.begin(); it != m_mapping.end();
            ++it)
    {
        IDPair& ids = *it;

        if (ids.first == from)
        {
            found = true;
            break;
        }
    }

    if (!found)
    {
        m_mapping.push_back(std::make_pair(from, to));
        return true;
    }
    //fprintf(stderr, "VariableIDMap: mapping already exists for var %u\n", from);
    return false;
}

void VariableIDMap::printCreationCode(FILE *fp) const
{
    fprintf(fp, "    cola::VariableIDMap idMap;\n");
    for (IDPairList::const_iterator it = m_mapping.begin();
            it != m_mapping.end(); ++it)
    {
        fprintf(fp, "    idMap.addMappingForVariable(%u, %u);\n", 
                it->first, it->second);
    }
    fprintf(fp, "    \n");
}

unsigned VariableIDMap::mappingForVariable(const unsigned var, 
        bool forward) const
{
    for (IDPairList::const_iterator it = m_mapping.begin(); 
            it != m_mapping.end(); ++it)
    {
        const IDPair& ids = *it;

        if (forward)
        {
            if (ids.first == var)
            {
                return ids.second;
            }
        }
        else // (!forward)
        {
            if (ids.second == var)
            {
                return ids.first;
            }
        }
    }

    //fprintf(stderr, "Warning: mapping not found for var %u\n", var);
    
    // Just return original variable index.
    return var;
}

void VariableIDMap::clear(void)
{
    m_mapping.clear();
}


} // namespace cola

