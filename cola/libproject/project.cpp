/**
 * @file project.cpp
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

#include <numeric>
#include <cmath>
#include <cfloat>
#include <cassert>
#include "util.h"
#include "project.h"

namespace project {

void Variable::updatePosition() { x = block->X + b; }

Constraint::Constraint(Variable *l, Variable *r, const double g)
    : l(l), r(r), g(g)
    , active(false) 
    , lm(0)
{
    l->out.push_back(this);
    r->in.push_back(this);
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
    LIBPROJECT_ASSERT( last==NULL || v==last->l || v==last->r );
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
    compute_dfdv(V[0],NULL);
}

Project::
Project(
        std::vector<Variable*> const &vs, 
        std::vector<Constraint *> const &cs) 
    : vs(vs)
    , cs(cs)
    , inactive(cs.begin(),cs.end()) 
{ 
}
Project::
~Project() {
    for_each(blocks.begin(),blocks.end(),delete_object());
}
/** 
 * attempts to solve a least-squares
 * problem subject to a set of separation constraints.
 * @return false if an unsatisfiable constraint is found
 */
bool Project::
solve() {
    initBlocks();
    bool optimal=true;
    do {
        makeOptimal();
        for_each(vs.begin(),vs.end(),mem_fun(&Variable::updatePosition));
        optimal=splitBlocks();
    } while(!optimal);
    return true;
}
/**
 * Put each variable in its own block
 */
void Project::
initBlocks() {
    for(Variables::const_iterator i=vs.begin();i!=vs.end();++i) {
        Block *b=new Block(*i);
        b->listIndex=blocks.insert(blocks.end(),b);
    }
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
        // MaxSafeMove should only ever be applied to inactive constraints
        LIBPROJECT_ASSERT(!_c->active);
        double a = 0;
        double Xl = _c->l->block->X,
               Xr = _c->r->block->X,
               bl = _c->l->b,
               br = _c->r->b;
        if(Xl + bl + _c->g <= Xr + br) {
            // if constraint is satisfied at the desired positions
            // then we can move all the way
            // note: this should also include inactive constraints within
            // the same block, hence the assertion below that the two 
            // sides of _c be in a different block.
            a = 1;
        } else {
            LIBPROJECT_ASSERT(_c->l->block!=_c->r->block);
            double XIl = _c->l->block->XI,
                   XIr = _c->r->block->XI;
            double Al = XIl + bl,
                   Ar = XIr + br,
                   Bl = Xl - XIl,
                   Br = Xr - XIr;
            a = (_c->g + Al - Ar) / (Br - Bl);
            LIBPROJECT_ASSERT(0<=a && a<=1);
        }
        LIBPROJECT_LOG(("C->g=%f, alpha=%f\n",_c->g,a));
        if(a < alpha) {
            c = _c;
            alpha = a;
        }
    }
    Constraint *&c; ///< the constraint with the smallest alpha to date
    double &alpha; ///< the distance required to move c in order to make it tight
};
#ifndef NDEBUG
const double epsilon = 1e-10;
void Project::
assertNoneViolated() {
    for(Constraints::const_iterator i=cs.begin();i!=cs.end();i++) {
        Constraint *c=*i;
        double XIl = c->l->block->XI,
               XIr = c->r->block->XI,
               bl = c->l->b,
               br = c->r->b;
        double slack = XIr + br - XIl - bl - c->g;
        LIBPROJECT_ASSERT(slack>=-epsilon);
    }
}
#endif
/**
 * Repeatedly search along the line from current to desired positions for the
 * first constraint that would be violated if we moved any further, and make
 * that constraint active.  We finish when all blocks can be moved to their
 * desired positions without violating any further constraints.
 */
void Project:: 
makeOptimal() {
#ifndef NDEBUG
    assertNoneViolated();
#endif
    Constraint *c=NULL;
    double alpha=DBL_MAX;
    for_each(inactive.begin(),inactive.end(),MaxSafeMove(c,alpha));
    while(alpha < 1) {
        makeActive(c,alpha);
        inactive.erase(c);
        for_each(inactive.begin(),inactive.end(),MaxSafeMove(c,alpha=DBL_MAX));
    }
    for(Blocks::iterator i=blocks.begin(); i!=blocks.end(); ++i) {
        Block* b=*i;
        b->XI = b->X;
    }
#ifndef NDEBUG
    assertNoneViolated();
#endif
}
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
void Project:: 
makeActive(Constraint *c, double alpha) {
    Block *L = c->l->block;
    Block *R = c->r->block;
    double br = c->l->b - c->r->b + c->g;
    double prevOptPos = L->X;
    LIBPROJECT_LOG(("mergeblock:\n"));
#ifdef LOGGING
    for(Variables::iterator i=L->V.begin();i!=L->V.end();++i) {
        Variable *v=*i;
        LIBPROJECT_LOG(("  v[%p]->b=%f\n",v,v->b));
    }
#endif
    LIBPROJECT_LOG((" plus:\n"));
    for(Variables::iterator i=R->V.begin();i!=R->V.end();++i) {
        Variable *v=*i;
        v->b+=br;
        v->block=L;
        LIBPROJECT_LOG(("  v[%p]->b=%f\n",v,v->b));
    }
    c->active=true;
    L->V.insert(L->V.end(),R->V.begin(),R->V.end());
    L->w+=R->w;
    L->C.insert(L->C.end(),R->C.begin(),R->C.end());
    L->C.push_back(c);
    L->X = L->optimalPosition();
    L->XI = (L->XI - alpha * (L->XI - prevOptPos + L->X))
            / (1.0 - alpha);
    LIBPROJECT_LOG(("   X=%f, XI=%f\n",L->X, L->XI));
    blocks.erase(R->listIndex);
}
bool cmpLagrangians(Constraint* a,Constraint* b) { return a->lm < b->lm; }
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
#ifdef LOGGING
        for(Constraints::iterator j=b->C.begin();j!=b->C.end();j++) {
            Constraint* c=*j;
            LIBPROJECT_LOG(("C->g=%f, lm=%f\n",c->g,c->lm));
        }
#endif
        Constraint *sc
            = *min_element(b->C.begin(),b->C.end(),cmpLagrangians);
        LIBPROJECT_LOG(("min: C->g=%f, lm=%f\n",sc->g,sc->lm));
        if(sc->lm < 0) {
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
    populateSplitBlock(v,c);
    X=optimalPosition();
}
/**
 * Make a given active constraint inactive, therefore cutting the tree of active
 * constraints in the block to which it belongs, and creating two new blocks.
 */
Blocks::iterator Project:: 
makeInactive(Constraint *c) {
    LIBPROJECT_LOG(("Project::makeInactive(Constraint *c)\n"));
    LIBPROJECT_ASSERT(c->active);
    inactive.insert(c);
    c->active=false;
    Block* b=c->l->block;
    Block* lb=new Block(c->l,c);
    Block* rb=new Block(c->r,c);
    lb->listIndex=blocks.insert(b->listIndex,lb);
    rb->listIndex=blocks.insert(b->listIndex,rb);
    blocks.erase(b->listIndex);
    delete b;
    return rb->listIndex;
}

} // namespace project
/*
 * vim: set cindent 
 * vim: ts=4 sw=4 et tw=0 wm=0
 */
