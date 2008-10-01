/*
 * vim: ts=4 sw=4 et tw=0 wm=0
 *
 * libproject - Solves an instance of the Variable Placement with Separation
 *              Constraints problem, that is a projection onto separation 
 *              constraints, whilst always maintaining feasibility.
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
 * \file testutil.h
 * utilities for testing libproject
 */
#include <map>
#include <cassert>
/**
 * Solve a project problem using a conventional quadratic programming solver.
 * We use Luca Di Gaspero's GPLed QuadProg++ program 
 * http://www.diegm.uniud.it/digaspero/
 *
 * The problem is converted to the form:
 *
 * \f$\min_x \frac{1}{2} x^T G x + g^T x \f$ 
 * subject to \f$C^T x + c \ge 0 \f$
 *
 * @param vs variables
 * @param cs constraints
 * @param result the solution vector
 */
void qps(project::Variables &vs, project::Constraints &cs, 
        std::vector<double> &result);

/**
 * Checks that each constraint is satisfied, throwing an assert error otherwise.
 */
bool feasible(project::Constraints &cs);

/**
 * used to identify variable indices from pointers to Variable
 */
typedef std::map<const project::Variable*,unsigned> VMap;

/**
 * Print out the problem instance (as code that can easily be pasted into
 * a test case)
 */
void printProblem(project::Variables &vs, 
		std::vector<double> &XI, 
		project::Constraints &cs, 
		VMap &vmap);

/**
 * float comparisons (i.e. \f$a=b\$f) in our tests are actually \$f|a-b|<epsilon\$f.
 */
extern const double testEpsilon;
/**
 * float comparisons (i.e. \f$a=b\$f) in our tests are actually \$f|a-b|<epsilon\$f.
 * Check approximate equality of two doubles.
 * @return true if the above condition is met
 */
bool approxEquals(double a, double b);
/**
 * get random number in the range [0,max]
 */
double getRand(const double max);
/**
 * Attempt to use libproject to solve a given projection problem instance.
 * @param t pointer to a function that will populate a list of variables and a list
 * of constraints.
 */
void test(void (*t)(std::string&, project::Variables&, project::Constraints&),
		bool silentPass = false);
