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


#ifndef VPSC_CONSTRAINT_H
#define VPSC_CONSTRAINT_H

// cmath needs ::strcpy_s under MinGW so include cstring.
#include <cstring>

#include <cfloat>
#include <iostream>
#include <vector>
#include <sstream>

#include "libvpsc/variable.h"

namespace vpsc {

class Variable;
typedef std::vector<Variable *> Variables;

//! @brief A constraint determines a minimum or exact spacing required between
//!        two Variable objects.
//!
class Constraint
{
	friend std::ostream& operator <<(std::ostream &os,const Constraint &c);
public:
    //! @brief Constructs a minimum or exact spacing constraint between two
    //!        Variable objects.
    //!
    //! (left + gap < right)  or (left + gap == right)
    //!
    //! @param[in] left      The left Variable.
    //! @param[in] right     The right Variable.
    //! @param[in] gap       The minimum or exact distance to separate the 
    //!                      variables by.
    //! @param[in] equality  Whether the separation is an exact distance or
    //!                      not.  The default is false.
	Constraint(Variable *left, Variable *right, double gap, 
            bool equality = false);
	~Constraint();

    /**
     *  @brief Returns a textual description of the constraint.
     *
     *  @return     A string describing the constraint.
     */
    std::string toString(void) const
    {
        std::stringstream stream;
        stream << "Constraint: var(" << left->id << ") ";
        if (gap < 0)
        {
            stream << "- " << -gap << " ";
        }
        else
        {
            stream << "+ " << gap << " ";
        }
        stream << ((equality) ? "==" : "<=");
        stream << " var(" << right->id << ") ";
        return stream.str();
    }

    inline double slack(void) const 
    { 
        if (unsatisfiable)
        {
            return DBL_MAX;
        }
        if (needsScaling)
        {
            return right->scale * right->position() - gap - 
                    left->scale * left->position();
        }
        COLA_ASSERT(left->scale == 1);
        COLA_ASSERT(right->scale == 1);
        return right->unscaledPosition() - gap - left->unscaledPosition(); 
    }

    //! @brief The left Variable.
    Variable *left;
    //! @brief The right Variable.
	Variable *right;
    //! @brief The minimum or exact distance to separate the variables by.
	double gap;
	double lm;
	long timeStamp;
	bool active;
    //! @brief Whether the separation is an exact distance or not.
	const bool equality;
    //! @brief Denote whether this constraint was unsatisifable (once the VPSC 
    //!        instance has been solved or satisfied).
	bool unsatisfiable;
    bool needsScaling;
    void *creator;
};

class CompareConstraints {
public:
	bool operator() (Constraint *const &l, Constraint *const &r) const;
};

//! @brief A vector of pointers to Constraint objects.
typedef std::vector<Constraint*> Constraints;

/** @brief Given a set of variables and constraints, returns a modified set
 *         of constraints with all redundant equality constraints removed.
 *
 * VPSC doesn't work well with redundant equality constraints, usually showing
 * them as unsatisfiable.  This function looks for cycles of equality 
 * constraints and removes the redundant ones.
 */
extern Constraints constraintsRemovingRedundantEqualities(
        const Variables& vars, const Constraints& constraints);

}

#endif // VPSC_CONSTRAINT_H
