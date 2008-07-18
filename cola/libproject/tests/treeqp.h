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

#include <vector>
/**
 * our goal function has terms:
 *   w_i(x_p+o_i-x_c)^2
 * where x_p is the position of the "parent", x_c is the "child" position,
 * w_i is a weight for the ith term and o_i is the offset required between
 * them.
 */
struct Term {
    unsigned p, c;
    double w, o; 
    Term(unsigned p, unsigned c, double w, double o) 
        : p(p), c(c), w(w), o(o) {
    }
};
typedef std::vector<Term> Terms;
/** 
 * we solve the goal subject to equality constraints of the form
 *   x_l+g=x_r or x_p=(x_l+x_r+g)/2
 */
struct EqualityConstraint {
    unsigned l, r, p;
    /// gap required between variables
    double g;
    /// true if this is an equality constraint, otherwise it's an inequality
    bool threeVars;
    EqualityConstraint(unsigned l, unsigned r, double g)
        : l(l), r(r), p(0), g(g), threeVars(false) {
    }
    EqualityConstraint(unsigned p, unsigned l, unsigned r, double g) 
        : l(l), r(r), p(p), g(g), threeVars(true) {
    }
};
typedef std::vector<EqualityConstraint> EqualityConstraints;
/**
 * we solve the goal subject to inequality constraint of the form 
 *   x_l+g<=x_r 
 */
struct InequalityConstraint {
    unsigned l, r;
    /// gap required between variables
    double g;
    InequalityConstraint(unsigned l, unsigned r, double g)
        : l(l), r(r), g(g) {
    }
};
typedef std::vector<InequalityConstraint> InequalityConstraints;
/**
 * Solve problem using a conventional quadratic programming solver.
 * We use Luca Di Gaspero's GPLed QuadProg++ program 
 * http://www.diegm.uniud.it/digaspero/
 *
 * The problem is converted to the form:
 *
 * \f$\min_x \frac{1}{2} x^T A x + b^T x \f$ 
 * subject to \f$C^T x + c \ge 0 \f$
 *
 * @param ts goal terms
 * @param cs constraints
 * @param result the solution vector
 */
void treeQPSolve(Terms &ts, EqualityConstraints &ecs, InequalityConstraints &ics, std::vector<double> &result);
/*
 * vim: set cindent 
 * vim: ts=4 sw=4 et tw=0 wm=0
 */
