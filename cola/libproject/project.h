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

#ifndef _LIBPROJECT_FEASIBLE_PROJECT_ALGORITHM_H
#define _LIBPROJECT_FEASIBLE_PROJECT_ALGORITHM_H

#include <vector>
#include <set>
#include <list>

namespace project {

using namespace std;

class Constraint;
typedef vector<Constraint*> Constraints;
class Block;

/**
 * Data for a Variable in a projection problem.
 */
struct Variable {
    Variable(double x, double d, double w=1.0) : x(x),d(d),w(w) {}
    /// recompute the current position based on offset and block position
    void updatePosition();
    /// compute derivative of goal function
    double dfdv() const { return 2.0 * w * (x-d); }
    /// weighted displacement from ideal position for block position
    double displacement() const { return w * (d-b); }
    /// compute cost of goal function at current position
    double cost() const { double dx=x-d; return w * dx*dx; }
    double x; ///< current position
    const double d; ///< desired position
    double w; ///< weight of variable's contribution to goal function
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
    void resetLM() { lm = 0; }
};

typedef list<Block*> Blocks;
/**
 * A block is a set of variables spanned by a tree of active constraints.
 */
class Block {
public:
    /**
     * create a block starting with a single variable.
     * @param v the starting variable
     */
    Block(Variable* v);
    /**
     * create a block by traversing a tree of active constraints.
     * @param v the variable from which to start the traversal
     * @param c don't traverse back over this constraint (v should be
     * either the left- or right-hand side of c)
     */
    Block(Variable* v, Constraint* c);
    /**
     * Compute the optimal position for this block based on the ideal positions of
     * its constituent variables.  
     */
    double optimalPosition() const;
    /**
     * compute the lagrangian multipliers of all the active constraints in this block.
     */
    void computeLagrangians();
    Variables V; ///< member vars
    double w; ///< total weight of constituent vars
    Constraints C; ///< active constraints
    double X; ///< position of reference var
    double XI; ///< initial position
    Blocks::iterator listIndex; ///< for easy removal from the blocks list
private:
    /**
     * Populate a new block that is created as the result of a splitting an existing block
     * by traversing its tree of active constraints.
     * @param v the start point of the traversal, assumes v->block still points to the old block
     * @param last don't backtrack over this constraint
     */
    void populateSplitBlock(Variable* v, Constraint const* last);
};

struct MaxSafeMove;

/**
 * A solver that projects onto separation constraints, starting from a
 * feasible solution, and maintaining feasibility at all times.
 */
class Project {
public:
    Project(std::vector<Variable*> const &vs, std::vector<Constraint *> const &cs);
    ~Project();
    /** 
     * attempts to solve a least-squares
     * problem subject to a set of separation constraints.
     * @return false if an unsatisfiable constraint is found
     */
    bool solve();
    /**
     * computes cost of the goal function over all variables
     */
    double cost() const;
private:
    Variables const &vs;
    Constraints const &cs;
    Blocks blocks;
    set<Constraint*> inactive;
    /**
     * Put each variable in its own block
     */
    void initBlocks();
    /**
     * Repeatedly search along the line from current to desired positions for the
     * first constraint that would be violated if we moved any further, and make
     * that constraint active.  We finish when all blocks can be moved to their
     * desired positions without violating any further constraints.
     */
    void makeOptimal();
    /**
     * Move all blocks by alpha * (X-XI) and make the specified constraint
     * active by setting to equality and merging the two blocks that it
     * spans into one new block (actually we merge the right hand side into the
     * left)
     * @param c the constraint with the maximum alpha over which it is
     * safe (meaning does not violate any other constraints) to merge.
     * @param alpha the fraction of the distance from the current to the
     * optimal position by which to move XI (the "initial" position) of 
     * each block
     */
    void makeActive(Constraint *c, double alpha);
    /**
     * Make a given active constraint inactive, therefore cutting the tree of active
     * constraints in the block to which it belongs, and creating two new blocks.
     */
    Blocks::iterator makeInactive(Constraint *c);
    /**
     * Check each block to see if splitting it allows the two new blocks to be moved
     * closer to their desired positions.  Returns true if no further splits are required
     * and therefore an optimal solution has been found.
     */
    bool splitBlocks(); 
    /// invariant: blocks.size() = |vs| - merges + splits
    int merges, splits;
};

} // namespace project
#endif // _LIBPROJECT_FEASIBLE_PROJECT_ALGORITHM_H
/*
 * vim: set cindent 
 * vim: ts=4 sw=4 et tw=0 wm=0
 */
