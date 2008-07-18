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
*/

/**
 * @file 
 * @brief Solve an instance of the "Variable Placement with Separation
 * Constraints" problem.
 *
 * Authors:
 *   Tim Dwyer <tgdwyer@gmail.com>
 */

//
// TODO: Really, we should have three classes: VPSC, IncrementalVPSC and
// StaticVPSC, where the latter two inherit from VPSC.  StaticVPSC would be
// the equivalent of what is currently VPSC.
// Also, a lot of the code specific to one or other of these concrete
// implementations should be moved from Block and Blocks: e.g. mergeLeft etc.
//
#ifndef SEEN_LIBVPSC_SOLVE_VPSC_H
#define SEEN_LIBVPSC_SOLVE_VPSC_H

#include <vector>
#include "exceptions.h"

/**
 * the vpsc namespace delineates the libvpsc Variable Placement with
 * Separation Constraints quadratic program solver library.
 */
namespace vpsc {
class Variable;
class Constraint;
class Blocks;
typedef std::vector<Constraint*> Constraints;

/**
 * Variable Placement with Separation Constraints problem instance
 */
class Solver {
public:
	// the following two methods both attempt to solve a least-squares
	// problem subject to a set of sepation constraints.  They return
	// true if any constraints are active, in both cases false means
	// an unconstrained optimum has been found.
	// satisfy returns an approximate solution subject to the constraints
	virtual bool satisfy();
	// solve returns an optimum solution subject to the constraints
	virtual bool solve();

	Solver(std::vector<Variable*> const &vs, std::vector<Constraint *> const &cs);
	virtual ~Solver();
	std::vector<Variable*> const & getVariables() { return vs; }
protected:
	Blocks *bs;
	unsigned m;
	std::vector<Constraint*> const &cs;
	unsigned n;
	std::vector<Variable*> const &vs;
	void printBlocks();
	void copyResult();
private:
	void refine();
	bool constraintGraphIsCyclic(const unsigned n, Variable* const vs[]);
	bool blockGraphIsCyclic();
};

class IncSolver : public Solver {
public:
	unsigned splitCnt;
	bool satisfy();
	bool solve();
	void moveBlocks();
	void splitBlocks();
	IncSolver(std::vector<Variable*> const &vs, std::vector<Constraint*> const &cs);
private:
	Constraints inactive;
	Constraints violated;
	Constraint* mostViolated(Constraints &l);
};
}
#endif // SEEN_LIBVPSC_SOLVE_VPSC_H
