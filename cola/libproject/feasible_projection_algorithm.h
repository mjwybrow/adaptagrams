/**
 * @file 
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

#ifndef _PROJECT_FEASIBLE_PROJECT_ALGORITHM_H
#define _PROJECT_FEASIBLE_PROJECT_ALGORITHM_H

#include <vector>
#include <set>
#include "project.h"

namespace algorithm {

using namespace std;

class Constraint;
typedef vector<Constraint*> Constraints;
class Block;

struct Variable : public project::Variable {
    Variable(project::Variable const &v)
        : project::Variable(v) {}
    Block* block; ///< container block
    double b; ///< offset from block reference variable
    Constraints in, out; ///< defines constraint DAG
};
typedef vector<Variable*> Variables;
/**
 * A separation constraint of the form \f$x_l + g \le x_r\f$
 */
class Constraint {
public:
    Constraint(Variable *l, Variable *r, const double g);
    Variable *l;
    Variable *r;
    const double g;
    double slack() const;
    bool active; ///< if set at equality
    double lm; ///< lagrange multiplier
};

/**
 * A block is a set of variables spanned by a tree of active constraints.
 */
class Block {
public:
    Block(Variable* v);
    /**
     * Compute the optimal position for this block based on the ideal positions of
     * its constituent variables.  
     */
    double optBlockPos() const;
    /**
     * compute the lagrangian multipliers of all the active constraints in this block.
     */
    void computeLagrangians();
    vector<Variable*> V; ///< member vars
    vector<Constraint*> C; ///< active constraints
    double X; ///< position of reference var
    double XI; ///< initial position
private:
    /**
     * used by compute_lagrangians to recursively compute lagrangrians from the partial derivatives
     * of each variable.
     */
    double compute_dfdv(Variable const* v, Constraint const* last);
};

/**
 * A solver that projects onto separation constraints, starting from a
 * feasible solution, and maintaining feasibility at all times.
 */
class FeasibleProjectionAlgorithm {
public:
    FeasibleProjectionAlgorithm(std::vector<Variable*> const &vs, std::vector<Constraint *> const &cs);
    ~FeasibleProjectionAlgorithm();
    /** 
     * attempts to solve a least-squares
     * problem subject to a set of separation constraints.
     * @return false if an unsatisfiable constraint is found
     */
    bool project();
private:
    vector<Variable*> const &vs;
    vector<Constraint*> const &cs;
    vector<Block*> blocks;
    set<Constraint*> inactive;
    void init_blocks();
    double maxSafeAlpha(Constraint const* c) const;
    void make_optimal();
    void make_active(Constraint *c, double alpha);
    void make_inactive(Constraint *c);
    void split_blocks(); 
};

} // namespace algorithm
#endif // _PROJECT_FEASIBLE_PROJECT_ALGORITHM_H
/*
 * vim: set cindent 
 * vim: ts=4 sw=4 et tw=0 wm=0
 */
