/*
 * vim: ts=4 sw=4 et tw=0 wm=0
 *
 * libvpsc - A solver for the problem of Variable Placement with 
 *           Separation Constraints.
 *
 * Copyright (C) 2005-2008  Monash University
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
*/


#include <map>
#include <list>
#include <sstream>
#include <cassert>
#include <cfloat>
#include <cmath>


#include "libvpsc/constraint.h"
#include "libvpsc/variable.h"

namespace vpsc {
Constraint::Constraint(Variable *left, Variable *right, double gap, bool equality)
: left(left),
  right(right),
  gap(gap),
  timeStamp(0),
  active(false),
  equality(equality),
  unsatisfiable(false),
  needsScaling(true),
  creator(nullptr)
{
    // In hindsight I think it's probably better to build the constraint DAG
    // (by creating variable in/out lists) when needed, rather than in advance
    //left->out.push_back(this);
    //right->in.push_back(this);
}
Constraint::~Constraint() {
    // see constructor: the following is just way too slow.  
    // Better to create a
    // new DAG on demand than maintain the lists dynamically.
    //Constraints::iterator i;
    //for(i=left->out.begin(); i!=left->out.end(); i++) {
        //if(*i==this) break;
    //}
    //left->out.erase(i);
    //for(i=right->in.begin(); i!=right->in.end(); i++) {
        //if(*i==this) break;
    //}
    //right->in.erase(i);
}
std::ostream& operator <<(std::ostream &os, const Constraint &c)
{
    const char *type = c.equality ? "=" : "<=";
    std::ostringstream lscale, rscale;
    if (c.left->scale != 1) 
    {
        lscale << c.left->scale << "*";
    }
    if (c.right->scale != 1)
    {
        rscale << c.right->scale << "*";
    }
    os << lscale.str() << *c.left << "+" << c.gap << type << 
          rscale.str() << *c.right;
    if (c.left->block && c.right->block)
    {
        os << "(" << c.slack() << ")" << (c.active ? "-active" : "") <<
              "(lm=" << c.lm << ")";
    }
    else
    {
        os << "(vars have no position)";
    }
    return os;
}
#include "libvpsc/block.h"
bool CompareConstraints::operator() (
    Constraint *const &l, Constraint *const &r
) const {
    double const sl = 
        l->left->block->timeStamp > l->timeStamp
        ||l->left->block==l->right->block
        ?-DBL_MAX:l->slack();
    double const sr = 
        r->left->block->timeStamp > r->timeStamp
        ||r->left->block==r->right->block
        ?-DBL_MAX:r->slack();
    if(sl==sr) {
        // arbitrary choice based on id
        if(l->left->id==r->left->id) {
            if(l->right->id<r->right->id) return true;
            return false;
        }
        if(l->left->id<r->left->id) return true;
        return false;
    }
    return sl < sr;
}


typedef std::list<std::map<Variable *, double> > VarOffsetMapList;

class EqualityConstraintSet
{
    public:
        EqualityConstraintSet(Variables vs)
        {
            for (size_t i = 0; i < vs.size(); ++i)
            {
                std::map<Variable *, double> varSet;
                varSet[vs[i]] = 0;
                variableGroups.push_back(varSet);
            }
        }
        bool isRedundant(Variable *lhs, Variable *rhs, double sep)
        {
            VarOffsetMapList::iterator lhsSet = setForVar(lhs);
            VarOffsetMapList::iterator rhsSet = setForVar(rhs);
            COLA_ASSERT(lhsSet != variableGroups.end());
            COLA_ASSERT(rhsSet != variableGroups.end());
            if (lhsSet == rhsSet)
            {
                // Check if this is a redundant constraint.
                if (fabs(((*lhsSet)[lhs] + sep) - (*rhsSet)[rhs]) < 0.0001)
                {
                    // If so, return true.
                    return true;
                }
            }
            return false;
        }
        void mergeSets(Variable *lhs, Variable *rhs, double sep)
        {
            VarOffsetMapList::iterator lhsSet = setForVar(lhs);
            VarOffsetMapList::iterator rhsSet = setForVar(rhs);
            if (lhsSet == rhsSet)
            {
                return;
            }

            double rhsOldOffset = (*rhsSet)[rhs];
            double rhsNewOffset = (*lhsSet)[lhs] + sep;
            double offset = rhsNewOffset - rhsOldOffset;

            // Update offsets
            for (std::map<Variable *, double>::iterator it = rhsSet->begin();
                    it != rhsSet->end(); ++it)
            {
                it->second += offset;
            }
            
            // Merge rhsSet into lhsSet
            lhsSet->insert(rhsSet->begin(), rhsSet->end());
            variableGroups.erase(rhsSet);
        }

    private:
        VarOffsetMapList::iterator setForVar(Variable *var)
        {
            for (VarOffsetMapList::iterator it = variableGroups.begin();
                    it != variableGroups.end(); ++it)
            {
                if (it->find(var) != it->end())
                {
                    return it;
                }
            }
            return variableGroups.end();
        }

    VarOffsetMapList variableGroups;
};

Constraints constraintsRemovingRedundantEqualities(const Variables& vars, 
        const Constraints& constraints)
{
    EqualityConstraintSet equalitySets(vars);
    Constraints cs = Constraints(constraints.size());
    int csSize = 0;

    for (unsigned i = 0; i < constraints.size(); ++i)
    {
        Constraint *c = constraints[i];
        if (c->equality)
        {
            if (!equalitySets.isRedundant(c->left, c->right, c->gap))
            {
                // Only add non-redundant equalities
                equalitySets.mergeSets(c->left, c->right, c->gap);
                cs[csSize++] = c;
            }
        }
        else
        {
            // Add all non-equalities
            cs[csSize++] = c;
        }
    }
    cs.resize(csSize);
    return cs;
}


}
