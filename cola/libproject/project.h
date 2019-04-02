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
 * See the file LICENSE.LGPL distributed with the library.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * Author(s):  Tim Dwyer
*/

#ifndef _LIBPROJECT_FEASIBLE_PROJECT_ALGORITHM_H
#define _LIBPROJECT_FEASIBLE_PROJECT_ALGORITHM_H

#include <vector>
#include <set>
#include <list>
#include <sstream>
#include <cstdio>

#include "libproject/util.h"

/**
 * project namespace delineates classes belonging to the libproject solver library.
 */
namespace project {

using namespace std;

class Constraint;
typedef vector<Constraint*> Constraints;
class Block;

/// type for Initial position for Variable ctor
struct Initial {
    explicit Initial(double pos) : pos(pos) {}
    double pos;
};
/// type for Desired position for Variable ctor
struct Desired {
    explicit Desired(double pos) : pos(pos) {}
    double pos;
};
/// type for Weight of Variable ctor
struct Weight {
    explicit Weight(double w) : w(w) {}
    double w;
};
/**
 * Data for a Variable in a projection problem.
 */
class Variable {
public:
    Variable(const unsigned id, const Initial& i) 
        : id(id), x(i.pos), d(i.pos), w(1.0)
        , block(nullptr), b(0), in(), out() {}
    /**
     * Arguments are passed in with explicit types to ensure they're in
     * the right order.
     * @param x the initial value of the variable
     * @param d the desired value
     * @param w weight, default is 1.0
     */
    Variable(const unsigned id, const Initial& i, const Desired& d,
            const Weight& w=Weight(1.0)) 
        : id(id), x(i.pos),d(d.pos),w(w.w),block(nullptr),b(0)
        , in(),out() {}
    Variable(const Initial& i, const Desired& d,
            const Weight& w=Weight(1.0)) 
        : id(idCtr++), x(i.pos),d(d.pos),w(w.w),block(nullptr),b(0)
        , in(),out() {}
    /** 
     * It may be necessary for the user to change the desired position after
     * the Variable is created.
     */
    void setPosition(const Desired& des) { d=des.pos; }
    double getDesiredPosition() const { return d; }
    /**
     * Similarly weight may need to be adjusted after initialisation
     */
    void setWeight(const Weight& weight) { w=weight.w; }
    double getWeight() const { return w; }
    /// get the current position (x)
    double getPosition() const { return x; }
    void setPosition(const Initial& p) { x=p.pos; }
    /// compute derivative of goal function
    double dfdv() const { return 2.0 * w * (x-d); }
    /// weighted displacement from ideal position for block position
    double displacement() const { return w * (d-b); }
    /// compute cost of goal function at current position
    double cost() const { double dx=x-d; return w * dx*dx; }
    /// @return block->XI + b
    double relativeInitialPos() const;
    /// @return block->X + b
    double relativeDesiredPos() const;
    /// recompute the current position based on offset and block position
    void updatePosition() { x = relativeDesiredPos(); }
    /** move x by alpha along the line from the relative initial position
     *  to the relative desired position
     */
    void moveBy(double alpha) { 
        double i=relativeInitialPos(), d=relativeDesiredPos();
        x = i + alpha*(d-i);
    }
    void clearConstraints() {
        in.clear();
        out.clear();
    }
    const unsigned id;
private:
    double x; ///< current position
    double d; ///< desired position
    double w; ///< weight of variable's contribution to goal function
    Block* block; ///< container block
    double b; ///< offset from block reference variable
    Constraints in, out; ///< defines constraint DAG
    static unsigned idCtr;
friend class Constraint;
friend class Block;
friend class Project;
friend double compute_dfdv(Variable const*, Constraint const*);
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
    double initialSlack() const;
    /** 
     * @return true if the constraint can be satisfied at the relative desired
     * positions for l and r.
     */
    bool feasibleAtDesired() const {
        return l->relativeDesiredPos() + g <= r->relativeDesiredPos();
    }
    /** 
     * @return the maximum move we can make along the line from initial to
     * desired positions without violating this constraint
     */
    double maxSafeAlpha() const;
    void setActive(bool val) {
        active=val;
    }
    bool isActive() const { return active; }
    void resetLM() { lm = 0; }
    bool wantsToMoveApart() const {
        return lm<-epsilon;
    }
    string toString() {
        std::stringstream s;
        s<<"C->g="<<g<<", lm="<<lm;
        return s.str();
    }
private:
    bool active; ///< if set at equality
    double lm; ///< lagrange multiplier
friend class Block;
friend double compute_dfdv(Variable const* v, Constraint const* last);
friend bool cmpLagrangians(Constraint* a,Constraint* b);
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
    /// vector to desired position
    double toDesired() const { return X - XI; }
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

/**
 * Override this functor and pass in a pointer to the implementation using 
 * Project::setExternalAlphaCheck in order to apply extra checks in each 
 * invocation of findSafeMove.
 */
struct ExternalAlphaCheck {
    virtual void operator()(double)=0;
    virtual ~ExternalAlphaCheck() {};
};

struct MaxSafeMove;

/**
 * A solver that projects onto separation constraints, starting from a
 * feasible solution, and maintaining feasibility at all times.
 */
class Project : private Uncopyable {
public:
    Project(
            std::vector<Variable*> const &vs, 
            std::vector<Constraint *> const &cs);

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
    /**
     * Set a pointer to a function that checks to make sure a given alpha does
     * not violate some external condition.
     */
    void setExternalAlphaCheck(ExternalAlphaCheck* check) {
        externalAlphaCheck=check;
    }
private:
    Variables const &vs;
    Constraints const &cs;
    Blocks blocks;
    set<Constraint*> inactive;
    /**
     * Put each variable in its own block
     */
    void initBlocksAndConstraints();
    /**
     * Find the largest move (alpha) we can make along the line from 
     * current positions to desired positions without violating a constraint.
     * If the function pointer externalAlphaCheck is set, then the function it points to
     * is called after an alpha is found for the internal constraints.  This gives
     * the application the chance to interrupt the solver early (for example,
     * to satisfy some other type of constraint and recompute the desired
     * positions).
     * @param c will be set to the constraint which becomes active at alpha
     * @return the largest move (alpha) that we can make without violating a constraint
     */
    double findSafeMove(Constraint* &c);
    /**
     * Repeatedly search along the line from current to desired positions for the
     * first constraint that would be violated if we moved any further, and make
     * that constraint active.  We finish when all blocks can be moved to their
     * desired positions without violating any further constraints.
     */
    void makeOptimal();
    /**
     * Make the specified constraint active by setting to equality and merging the
     * two blocks that it spans into one new block (actually we merge the right
     * hand side into the left).
     * The desired position of the merged block is recomputed and a "virtual" initial position
     * for the new block is computed by projection along the line from the new desired
     * position and the point XI+alpha*(X-XI), where XI and X are the initial and desired
     * positions of the old left block.
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
    /// a callback function checked in findSafeMove
    ExternalAlphaCheck* externalAlphaCheck;
}; // class Project

/**
 * Critical failure: either something went wrong with the solver or (more likely) there
 * was infeasible input.
 */
struct CriticalFailure {
    CriticalFailure(const char *expr, 
            const char *file, 
            int line, 
            const char *function)
        : expr(expr), file(file), line(line), function(function)
    {}
    void print() {
        fprintf(stderr,"ERROR: Critical sanity check failed in libproject!\n"
                "  expression: %s\n  at line %d of %s\n"
                "  in: %s\n", expr, line, file, function);
    }
    const char *expr;
    const char *file;
    int line;
    const char *function;
};
/**
 * Solve an instance of a variable placement with separation constraint (VPSC)
 * problem, that is a projection of variables onto separation constraints,
 * by a method that maintains feasibility at all times.
 * @return false if an unsatisfiable constraint is found.
 */
bool solve(Variables& vs, Constraints& cs);

} // namespace project
#endif // _LIBPROJECT_FEASIBLE_PROJECT_ALGORITHM_H
