/**
 * @file project.h
 * @brief Solve an instance of the "Variable Placement with Separation
 * Constraints" problem, that is a projection onto separation constraints,
 * whilest always maintaining feasibility.
 *
 * Authors:
 *   Tim Dwyer <tgdwyer@gmail.com>
 *
 * Copyright (C) 2007 Authors
 *
 * Released under GNU LGPL.  Read the file 'COPYING' for more information.
 */

#ifndef _PROJECT_PROJECT_H
#define _PROJECT_PROJECT_H

#include <vector>
#include <map>
namespace algorithm {
    class Variable;
    class Constraint;
    class FeasibleProjectionAlgorithm;
    typedef std::vector<Variable*> Variables;
    typedef std::vector<Constraint*> Constraints;
} // namespace algorithm

namespace project {

using namespace std;
class Constraint;
typedef vector<Constraint*> Constraints;
class Block;
/**
 */
class Variable {
public:
    Variable(double x, double d);
    double x; ///< current position
    const double d; ///< desired position
};
typedef vector<Variable*> Variables;

/**
 * A separation constraint of the form l->x + g <= r->x
 */
class Constraint {
public:
    Constraint(Variable const *l, Variable const *r, const double g);
    Variable const *l; 
    Variable const *r;
    const double g; ///< minimum separation required between l and r
};

/**
 * A solver that projects onto separation constraints, starting from a
 * feasible solution, and maintaining feasibility at all times.
 */
class FeasibleProjection {
public:
    FeasibleProjection(Variables const &vs, Constraints const &cs);
    ~FeasibleProjection();
    /** 
     * attempts to solve a least-squares
     * problem subject to a set of separation constraints.
     * @return false if an unsatisfiable constraint is found
     */
    bool solve();
private:
    Variables const &vs;
    Constraints const &cs;
    algorithm::Variables avs;
    algorithm::Constraints acs;
    map<Variable const*,algorithm::Variable*> vMap;
    map<Constraint const*,algorithm::Constraint*> cMap;
    algorithm::FeasibleProjectionAlgorithm* alg;        
};

} // namespace project
#endif // _PROJECT_PROJECT_H
/*
 * vim: set cindent 
 * vim: ts=4 sw=4 et tw=0 wm=0
 */
