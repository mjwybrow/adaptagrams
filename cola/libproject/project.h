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
    Variable(double x, double d) : x(x),d(d) {}
    /// recompute the current position based on offset and block position
    void updatePosition();
    /// displacement from ideal position
    double displacement() { return d - b; }
    void setBlock(Block* b) { block = b; }
    double x; ///< current position
    const double d; ///< desired position
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
    Constraints C; ///< active constraints
    double X; ///< position of reference var
    double XI; ///< initial position
    Blocks::iterator listIndex; ///< for easy removal from the blocks list
private:
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
private:
    Variables const &vs;
    Constraints const &cs;
    Blocks blocks;
    set<Constraint*> inactive;
    void initBlocks();
    void makeOptimal();
    void makeActive(Constraint *c, double alpha);
    Blocks::iterator makeInactive(Constraint *c);
    bool splitBlocks(); 
#ifndef NDEBUG
    void assertNoneViolated();
#endif
};

} // namespace project
#endif // _LIBPROJECT_FEASIBLE_PROJECT_ALGORITHM_H
/*
 * vim: set cindent 
 * vim: ts=4 sw=4 et tw=0 wm=0
 */
