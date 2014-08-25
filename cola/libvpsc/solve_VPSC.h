/*
 * vim: ts=4 sw=4 et tw=0 wm=0
 *
 * libvpsc - A solver for the problem of Variable Placement with 
 *           Separation Constraints.
 *
 * Copyright (C) 2005-2013  Monash University
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

//
// TODO: Really, we should have three classes: VPSC, IncrementalVPSC and
// StaticVPSC, where the latter two inherit from VPSC.  StaticVPSC would be
// the equivalent of what is currently VPSC.
// Also, a lot of the code specific to one or other of these concrete
// implementations should be moved from Block and Blocks: e.g. mergeLeft etc.
//
#ifndef VPSC_SOLVE_VPSC_H
#define VPSC_SOLVE_VPSC_H

#include <vector>

/**
 * @namespace vpsc
 * @brief libvpsc: Variable Placement with Separation Constraints
 *        quadratic program solver library.
 *
 * You should use VPSC via an instance of the IncSolver or Solver classes.
 */
namespace vpsc {
class Variable;
typedef std::vector<Variable*> Variables;
class Constraint;
class Blocks;
typedef std::vector<Constraint*> Constraints;

/**
 * @brief Static solver for Variable Placement with Separation Constraints 
 *        problem instance
 *
 * This class attempts to solve a least-squares problem subject to a set 
 * of separation constraints.  The solve() and satisfy() methods return true 
 * if any constraints are active, in both cases false means an unconstrained 
 * optimum has been found.
 *
 * @sa IncSolver
 */
class Solver {
public:
	//! @brief  Results in an approximate solution subject to the constraints.
    //! @return true if any constraints are active, or false if an unconstrained 
    //!         optimum has been found.
	virtual bool satisfy();
	//! @brief  Results in an optimum solution subject to the constraints
    //! @return true if any constraints are active, or false if an unconstrained 
    //!         optimum has been found.
	virtual bool solve();

	Solver(Variables const &vs, Constraints const &cs);
	virtual ~Solver();
    //! @brief   Returns the Variables in this problem instance.
    //! @returns A vector of Variable objects.
    Variables const & getVariables() { return vs; }
protected:
	Blocks *bs;
	size_t m;
	std::vector<Constraint*> const &cs;
	size_t n;
	std::vector<Variable*> const &vs;
    bool needsScaling;

	void printBlocks();
	void copyResult();
private:
	void refine();
	bool constraintGraphIsCyclic(const unsigned n, Variable* const vs[]);
	bool blockGraphIsCyclic();
};

/**
 * @brief Incremental solver for Variable Placement with Separation Constraints 
 *        problem instance
 *
 * This class attempts to solve a least-squares problem subject to a set 
 * of sepation constraints.  The solve() and satisfy() methods return true 
 * if any constraints are active, in both cases false means an unconstrained 
 * optimum has been found.  This is an incremental version of that allows 
 * refinement after blocks are moved.  This version is preferred if you are 
 * using VPSC in an interactive context.
 *
 * @sa Solver
 */
class IncSolver : public Solver {
public:
	IncSolver(Variables const &vs, Constraints const &cs);
	//! @brief  Results in an approximate solution subject to the constraints.
    //! @return true if any constraints are active, or false if an unconstrained 
	bool satisfy();
	//! @brief  Results in an optimum solution subject to the constraints
    //! @return true if any constraints are active, or false if an unconstrained 
    //!         optimum has been found.
	bool solve();
   	//! @brief  Adds a constraint to the existing VPSC solver.
    //!
    //! @param constraint The new additional constraint to add. 
    void addConstraint(Constraint *constraint);
private:
	void moveBlocks();
	void splitBlocks();

	unsigned splitCnt;
	Constraints inactive;
	Constraints violated;
	Constraint* mostViolated(Constraints &l);
};

}
#endif // VPSC_SOLVE_VPSC_H
