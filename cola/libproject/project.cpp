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
 *
*/

#include <algorithm>
#include <numeric>
#include <cmath>
#include <cfloat>
#include <cassert>
#include <iterator>
#include "libproject/util.h"
#include "libproject/project.h"
#include "libproject/project_log.h"

#ifndef NDEBUG
static double lastCost;
/// invariant: blocks.size() = |vs| - merges + splits
static int merges;
static int splits;
#define DEBUG_CODE(code) { code; }
#define INIT_DEBUG(p) {      \
    lastCost = (p)->cost(); \
    merges = 0; \
    splits = 0; \
}
#define ASSERT_COST_DECREASE(p) {    \
    double c = (p)->cost();          \
    /*printf("lastcost=%f, currcost=%f\n",lastCost,c);*/ \
    LIBPROJECT_ASSERT(c<=lastCost+epsilon);  \
    lastCost = c;                    \
}
#define ASSERT_NONE_VIOLATED(p) { \
    for(Constraints::const_iterator i=(p)->cs.begin();i!=(p)->cs.end();i++) { \
        Constraint *c=*i;                                                     \
        double XIl = c->l->block->XI,                                         \
               XIr = c->r->block->XI,                                         \
               bl = c->l->b,                                                  \
               br = c->r->b;                                                  \
        double slack = XIr + br - XIl - bl - c->g;                            \
        LIBPROJECT_ASSERT(slack>=-epsilon);                                   \
    }                                                                         \
}
#else // not NDEBUG
#define ASSERT_COST_DECREASE(p)
#define ASSERT_NONE_VIOLATED(p)
#define INIT_DEBUG(p)
#define DEBUG_CODE(code)
#endif
namespace project {
/**
 * Solve an instance of a variable placement with separation constraint (VPSC)
 * problem, that is a projection of variables onto separation constraints,
 * by a method that maintains feasibility at all times.
 * @return false if an unsatisfiable constraint is found.
 */
bool solve(Variables& vs, Constraints& cs) {
    Project p(vs,cs);
    return p.solve();
}

unsigned Variable::idCtr=0;

double Variable::relativeInitialPos() const { return block->XI + b; }
double Variable::relativeDesiredPos() const { return block->X + b; }

Constraint::Constraint(Variable *l, Variable *r, const double g)
    : l(l), r(r), g(g)
    , active(false) 
    , lm(0)
{
    l->out.push_back(this);
    r->in.push_back(this);
}

double Constraint::initialSlack() const {
    return r->x - l->x - g;
}

Block::Block(Variable* v) 
    : w(v->w)
    , X(v->d)
    , XI(v->x)
{
    V.push_back(v);
    v->b=0;
    v->block=this;
}

/**
 * Compute the optimal position for this block based on the ideal positions of
 * its constituent variables.  
 * That is, for each variable in the block \f$v_i\in V\f$ with ideal
 * positions \f$d_i\f$ and offset relative to the block reference position
 * \f$b_i\f$ the ideal position for the block is
 * \f$\frac{1}{|V|}\sum_{v_i\in V} d_i - b_i\f$.
 */
double Block::optimalPosition() const {
    return sum_over(V.begin(),V.end(),0.0,mem_fun(&Variable::displacement)) / w;
}

/**
 * Used by computeLagrangians to recursively compute lagrangrians over the tree of
 * active constraints, from the partial derivatives of each variable.
 * @param v either the left or right side of last, used as the starting point of recursion
 * @param last don't backtrack over this constraint.
 */
double compute_dfdv(Variable const* v, Constraint const* last) {
    LIBPROJECT_ASSERT( last==nullptr || v==last->l || v==last->r );
    double dfdv = v->dfdv();
    for(Constraints::const_iterator i=v->out.begin();i!=v->out.end();++i) {
        Constraint *c=*i;
        if(c!=last && c->active) {
            c->lm = compute_dfdv(c->r,c);
            dfdv += c->lm;
        }
    }
    for(Constraints::const_iterator i=v->in.begin();i!=v->in.end();++i) {
        Constraint *c=*i;
        if(c!=last && c->active) {
            c->lm = -compute_dfdv(c->l,c);
            dfdv -= c->lm;
        }
    }
    return dfdv;
}
/**
 * Compute the lagrange multipliers for each active constraint in the block
 */
void Block::computeLagrangians() {
    for_each(C.begin(),C.end(),mem_fun(&Constraint::resetLM));
    compute_dfdv(V[0],nullptr);
}

Project::
Project(
        vector<Variable*> const &vs, 
        vector<Constraint *> const &cs) 
    : vs(vs)
    , cs(cs)
    , inactive(cs.begin(),cs.end())
    , externalAlphaCheck(nullptr)
{ 
    FILELog::ReportingLevel() = logERROR;
    //FILELog::ReportingLevel() = logDEBUG1;
}
Project::
~Project() {
    LIBPROJECT_ASSERT(blocks.size()==vs.size()-merges+splits);
    for_each(blocks.begin(),blocks.end(),delete_object());
}
/** 
 * attempts to solve a least-squares
 * problem subject to a set of separation constraints.
 */
bool Project::
solve() {
    initBlocksAndConstraints();
    INIT_DEBUG(this);
    bool optimal=true;
    do {
        makeOptimal();
        for_each(vs.begin(),vs.end(),mem_fun(&Variable::updatePosition));
        ASSERT_COST_DECREASE(this);
        optimal=splitBlocks();
    } while(!optimal);
    return true;
}
/**
 * Put each variable in its own block
 */
void Project::
initBlocksAndConstraints() {
    for(Variables::const_iterator i=vs.begin();i!=vs.end();++i) {
        Variable* v=*i;
        Block *b=new Block(v);
        b->listIndex=blocks.insert(blocks.end(),b);
    }
    for_each(cs.begin(),cs.end(),bind2nd(mem_fun(&Constraint::setActive),false));
}

/** 
 * @return the maximum move we can make along the line from initial to desired positions
 * without violating this constraint
 */
double Constraint::maxSafeAlpha() const {
    // maxSafeAlpha should only ever be applied to inactive constraints
    LIBPROJECT_ASSERT(!active);
    double a;
    if(feasibleAtDesired()) {
        // if constraint is satisfied at the desired positions
        // then we can move all the way
        // note: this should also include inactive constraints within
        // the same block, hence the assertion below that the two 
        // sides of _c be in a different block.
        a = 1;
    } else {
        LIBPROJECT_ASSERT(l->block!=r->block);
        double Al = l->relativeInitialPos(),
               Ar = r->relativeInitialPos(),
               Bl = l->block->toDesired(),
               Br = r->block->toDesired();
        double n = g + Al - Ar,
               d = Br - Bl;
        if(approx_equals(n,0)||approx_equals(d,0)) {
            a = 0;
        } else {
            a = n / d;
        }
        LIBPROJECT_ASSERT(0<=a && a<=1);
    }
    FILE_LOG(logDEBUG1)<<"C@"<<this<<" g="<<g<<", alpha="<<a;
    return a;
}
/**
 * Functor used for finding the largest move (alpha) we can make along the line from 
 * current positions to desired positions without violating a constraint.
 */
struct MaxSafeMove : unary_function<Constraint*,void> {
    MaxSafeMove(Constraint *&c, double &alpha) : c(c), alpha(alpha) { }
    /**
     * Compute the distance along the line from current to desired positions we would
     * need to move to make a given constraint tight.  If that distance is smaller than
     * the existing alpha then store it in alpha and note the constraint in c.
     * @param _c constraint to check against alpha
     */
    void operator()(Constraint *_c) {
        double a = _c->maxSafeAlpha();
        if(a < alpha) {
            c = _c;
            alpha = a;
        }
    }
    Constraint *&c; ///< the constraint with the smallest alpha to date
    double &alpha; ///< the distance required to move c in order to make it tight
};
/**
 * Find the largest move (alpha) we can make along the line from 
 * current positions to desired positions without violating a constraint.
 * If the function pointer externalAlphaCheck is set, then the function it points to
 * is called giving the application the chance to interrupt the solver early (for
 * example, to satisfy some other type of constraint and recompute the desired positions).
 * @param c will be set to the constraint which becomes active at alpha
 * @return the largest move (alpha) that we can make without violating a constraint
 */
double Project::
findSafeMove(Constraint* &c) {
    c=nullptr;
    double alpha=1.0;
    for_each(inactive.begin(),inactive.end(),MaxSafeMove(c,alpha));
    if(externalAlphaCheck) {
        (*externalAlphaCheck)(alpha);
    }
    return alpha;
} 
/**
 * Repeatedly search along the line from current to desired positions for the
 * first constraint that would be violated if we moved any further, and make
 * that constraint active.  We finish when all blocks can be moved to their
 * desired positions without violating any further constraints.
 */
void Project:: 
makeOptimal() {
    ASSERT_NONE_VIOLATED(this);
    Constraint *c=nullptr;
    double alpha;
    while((alpha = findSafeMove(c)) < 1) {
        makeActive(c,alpha);
        inactive.erase(c);
    }
    for(Blocks::iterator i=blocks.begin(); i!=blocks.end(); ++i) {
        Block* b=*i;
        b->XI = b->X;
    }
    ASSERT_NONE_VIOLATED(this);
}

ostream& operator <<(ostream &os, const Variable* &v) {
    os << "(" << v->getPosition()
       << "," << v->getDesiredPosition()
       << "," << v->getWeight()
       << ")";
    return os;
}
ostream& operator <<(ostream &os, const Block &b) {
    copy(b.V.begin(),b.V.end(),ostream_iterator<Variable*>(os,","));
    return os;
}
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
void Project:: 
makeActive(Constraint *c, double alpha) {
    Block *L = c->l->block;
    Block *R = c->r->block;
    double br = c->l->b - c->r->b + c->g;
    double prevOptPos = L->X;
    FILE_LOG(logDEBUG1)<<"mergeblock:";
    FILE_LOG(logDEBUG1)<<*L;
    FILE_LOG(logDEBUG1)<<" plus:";
    FILE_LOG(logDEBUG1)<<*R;
    for(Variables::iterator i=R->V.begin();i!=R->V.end();++i) {
        Variable *v=*i;
        v->b+=br;
        v->block=L;
    }
    c->setActive(true);
    L->V.insert(L->V.end(),R->V.begin(),R->V.end());
    L->w+=R->w;
    L->C.insert(L->C.end(),R->C.begin(),R->C.end());
    L->C.push_back(c);
    L->X = L->optimalPosition();
    L->XI = (L->XI - alpha * (L->XI - prevOptPos + L->X))
            / (1.0 - alpha);
    FILE_LOG(logDEBUG1)<<"  L': X="<<L->X<<", XI="<<L->XI;
    blocks.erase(R->listIndex);
    delete R;
    DEBUG_CODE(merges++);
}
bool cmpLagrangians(Constraint* a,Constraint* b) { return a->lm < b->lm; }
ostream& operator <<(ostream &os, const Constraints &cs) {
    Constraints::const_iterator c=cs.begin();
    os << (*(c++))->toString();
    for(;c!=cs.end();++c) {
        os<<","<<(*c)->toString();
    }
    copy(cs.begin(),cs.end(),ostream_iterator<Constraint*>(os,","));
    return os;
}
/**
 * Check each block to see if splitting it allows the two new blocks to be moved
 * closer to their desired positions.  Returns true if no further splits are required
 * and therefore an optimal solution has been found.
 */
bool Project:: 
splitBlocks() {
    bool optimal = true;
    for(Blocks::iterator i = blocks.begin(); i!=blocks.end(); ++i) {
        Block* b = *i;
        b->XI = b->X;
        if(b->C.empty()) continue;
        b->computeLagrangians();

        FILE_LOG(logDEBUG1)<<b->C;

        Constraint *sc
            = *min_element(b->C.begin(),b->C.end(),cmpLagrangians);

        FILE_LOG(logDEBUG1)<<"min: "<<sc;

        if(sc->wantsToMoveApart()) {
            optimal = false;
            i=makeInactive(sc);
        }
    }
    return optimal;
}
/**
 * Populate a new block that is created as the result of a splitting an existing block
 * by traversing its tree of active constraints.
 * @param v the start point of the traversal, assumes v->block still points to the old block
 * @param last don't backtrack over this constraint
 */
void Block::populateSplitBlock(Variable* v, Constraint const* last) {
    LIBPROJECT_ASSERT( v==last->l || v==last->r );
    LIBPROJECT_ASSERT( v->block!=this );
    V.push_back(v);
    v->block = this;
    w+=v->w;
    for(Constraints::const_iterator i=v->out.begin();i!=v->out.end();++i) {
        Constraint *c=*i;
        if(c!=last && c->active) {
            C.push_back(c);
            populateSplitBlock(c->r,c);
        }
    }
    for(Constraints::const_iterator i=v->in.begin();i!=v->in.end();++i) {
        Constraint *c=*i;
        if(c!=last && c->active) {
            C.push_back(c);
            populateSplitBlock(c->l,c);
        }
    }
}
/**
 * create a block by traversing a tree of active constraints.
 * @param v the variable from which to start the traversal
 * @param c don't traverse back over this constraint (v should be
 * either the left- or right-hand side of c)
 */
Block::Block(Variable* v, Constraint* c) : w(0), XI(v->block->XI) {
    LIBPROJECT_ASSERT(!c->active);
    LIBPROJECT_ASSERT( v==c->l || v==c->r );
    populateSplitBlock(v,c);
    X=optimalPosition();
}
/**
 * Make a given active constraint inactive, therefore cutting the tree of active
 * constraints in the block to which it belongs, and creating two new blocks.
 * @param c the constraint to make inactive and hence split the block across
 * @return position of the second of the two new blocks in the list of blocks
 */
Blocks::iterator Project:: 
makeInactive(Constraint *c) {
    FILE_LOG(logDEBUG) << "Project::makeInactive(Constraint *c)";
    LIBPROJECT_ASSERT(c->isActive());
    inactive.insert(c);
    c->setActive(false);
    Block* b=c->l->block;
    Block* lb=new Block(c->l,c);
    Block* rb=new Block(c->r,c);
    LIBPROJECT_ASSERT(b->V.size()==lb->V.size()+rb->V.size());
    LIBPROJECT_ASSERT(approx_equals(b->w,lb->w+rb->w));
    LIBPROJECT_ASSERT(lb->X<=b->X+epsilon);
    LIBPROJECT_ASSERT(rb->X>=b->X-epsilon);
    lb->listIndex=blocks.insert(b->listIndex,lb);
    rb->listIndex=blocks.insert(b->listIndex,rb);
    blocks.erase(b->listIndex);
    delete b;
    DEBUG_CODE(splits++);
    return rb->listIndex;
}

/**
 * computes cost of the goal function over all variables
 */
double Project::cost() const {
    return sum_over(vs.begin(),vs.end(),0.0,mem_fun(&Variable::cost));
}

} // namespace project
