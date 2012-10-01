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
 * Authors:
 *   Tim Dwyer <tgdwyer@gmail.com>
*/


#ifndef VPSC_CONSTRAINT_H
#define VPSC_CONSTRAINT_H

#include <iostream>
#include <vector>

namespace vpsc {

class Variable;
typedef std::vector<Variable *> Variables;

/** @brief A constraint determines a minimum or exact spacing required between
 *         two variables.
 */
class Constraint
{
	friend std::ostream& operator <<(std::ostream &os,const Constraint &c);
public:
	Variable *left;
	Variable *right;
	double gap;
	double lm;
	Constraint(Variable *left, Variable *right, double gap, bool equality=false);
	~Constraint();
	double slack() const;
	long timeStamp;
	bool active;
	const bool equality;
	bool unsatisfiable;
};
class CompareConstraints {
public:
	bool operator() (Constraint *const &l, Constraint *const &r) const;
};
typedef std::vector<Constraint*> Constraints;

/** @brief Given a set of variables and constraints, returns a modified set
 *         of constraints with all redundant equality constraints removed.
 *
 * VPSC doesn't work well with redundant equality constraints, usually showing
 * them as unsatisfiable.  This function looks for cycles of equality 
 * constraints and removes the redundant ones.
 */
extern Constraints constraintsRemovingRedundantEqualities(
        Variables const &vars, Constraints const &constraints);

}

#endif // VPSC_CONSTRAINT_H
