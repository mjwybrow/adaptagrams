/*
 * vim: ts=4 sw=4 et tw=0 wm=0
 *
 * libvpsc - A solver for the problem of Variable Placement with 
 *           Separation Constraints.
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

/*
 * @brief A block is a group of variables that must be moved together to improve
 * the goal function without violating already active constraints.
 * The variables in a block are spanned by a tree of active constraints.
 *
 */

#include "libvpsc/block.h"
#include "libvpsc/variable.h"
#include <cassert>
#include "libvpsc/pairing_heap.h"
#include "libvpsc/constraint.h"
#include "libvpsc/exceptions.h"
#include "libvpsc/blocks.h"

#ifdef LIBVPSC_LOGGING
#include <fstream>
using std::ios;
using std::ofstream;
using std::endl;
#endif

#define __NOTNAN(p) (p)==(p)

namespace vpsc {
void PositionStats::addVariable(Variable* v) {
    double ai=scale/v->scale;
    double bi=v->offset/v->scale;
    double wi=v->weight;
    AB+=wi*ai*bi;
    AD+=wi*ai*v->desiredPosition;
    A2+=wi*ai*ai;
    /*
#ifdef LIBVPSC_LOGGING
    ofstream f(LOGFILE,ios::app);
    f << "adding v[" << v->id << "], blockscale=" << scale << ", despos=" 
      << v->desiredPosition << ", ai=" << ai << ", bi=" << bi
      << ", AB=" << AB << ", AD=" << AD << ", A2=" << A2;
#endif
*/
}
void Block::addVariable(Variable* v) {
    v->block=this;
    vars->push_back(v);
    if(ps.A2==0) ps.scale=v->scale;
    //weight+= v->weight;
    //wposn += v->weight * (v->desiredPosition - v->offset);
    //posn=wposn/weight;
    ps.addVariable(v);
    posn=(ps.AD - ps.AB) / ps.A2;
    COLA_ASSERT(__NOTNAN(posn));
    /*
#ifdef LIBVPSC_LOGGING
    ofstream f(LOGFILE,ios::app);
    f << ", posn=" << posn << endl;
#endif
*/
}
Block::Block(Blocks *blocks, Variable* const v)
    : vars(new std::vector<Variable*>)
    , posn(0)
    //, weight(0)
    //, wposn(0)
    , deleted(false)
    , timeStamp(0)
    , in(nullptr)
    , out(nullptr)
    , blocks(blocks)
{
    if(v!=nullptr) {
        v->offset=0;
        addVariable(v);
    }
}

void Block::updateWeightedPosition() {
    //wposn=0;
    ps.AB=ps.AD=ps.A2=0;
    for (Vit v=vars->begin();v!=vars->end();++v) {
        //wposn += ((*v)->desiredPosition - (*v)->offset) * (*v)->weight;
        ps.addVariable(*v);
    }
    posn=(ps.AD - ps.AB) / ps.A2;
    COLA_ASSERT(__NOTNAN(posn));
#ifdef LIBVPSC_LOGGING
    ofstream f(LOGFILE,ios::app);
    f << ", posn=" << posn << endl;
#endif
}
Block::~Block(void)
{
    delete vars;
    delete in;
    delete out;
}
void Block::setUpInConstraints() {
    setUpConstraintHeap(in,true);
}
void Block::setUpOutConstraints() {
    setUpConstraintHeap(out,false);
}
void Block::setUpConstraintHeap(PairingHeap<Constraint*,CompareConstraints>* &h,bool in) {
    delete h;
    h = new PairingHeap<Constraint*,CompareConstraints>();
    for (Vit i=vars->begin();i!=vars->end();++i) {
        Variable *v=*i;
        std::vector<Constraint*> *cs=in?&(v->in):&(v->out);
        for (Cit j=cs->begin();j!=cs->end();++j) {
            Constraint *c=*j;
            c->timeStamp=blocks->blockTimeCtr;
            if ( ((c->left->block != this) && in) || 
                 ((c->right->block != this) && !in) )
            {
                h->insert(c);
            }
        }
    }
}    
Block* Block::merge(Block* b, Constraint* c) {
#ifdef LIBVPSC_LOGGING
    ofstream f(LOGFILE,ios::app);
    f<<"  merging on: "<<*c<<",c->left->offset="<<c->left->offset<<",c->right->offset="<<c->right->offset<<endl;
#endif
    double dist = c->right->offset - c->left->offset - c->gap;
    Block *l=c->left->block;
    Block *r=c->right->block;
    if (l->vars->size() < r->vars->size()) {
        r->merge(l,c,dist);
    } else {
               l->merge(r,c,-dist);
    }
    Block* mergeBlock=b->deleted?this:b;
#ifdef LIBVPSC_LOGGING
    f<<"  merged block="<<*mergeBlock<<endl;
#endif
    return mergeBlock;
}
/*
 * Merges b into this block across c.  Can be either a
 * right merge or a left merge
 * @param b block to merge into this
 * @param c constraint being merged
 * @param distance separation required to satisfy c
 */
void Block::merge(Block *b, Constraint *c, double dist) {
#ifdef LIBVPSC_LOGGING
    ofstream f(LOGFILE,ios::app);
    f<<"    merging: "<<*b<<"dist="<<dist<<endl;
#endif
    c->active=true;
    //wposn+=b->wposn-dist*b->weight;
    //weight+=b->weight;
    for(Vit i=b->vars->begin();i!=b->vars->end();++i) {
        Variable *v=*i;
        //v->block=this;
        //vars->push_back(v);
        v->offset+=dist;
        addVariable(v);
    }
#ifdef LIBVPSC_LOGGING
    for(Vit i=vars->begin();i!=vars->end();++i) {
        Variable *v=*i;
        f<<"    v["<<v->id<<"]: d="<<v->desiredPosition
            <<" a="<<v->scale<<" o="<<v->offset
            <<endl;
    }
    f<<"  AD="<<ps.AD<<" AB="<<ps.AB<<" A2="<<ps.A2<<endl;
#endif
    //posn=wposn/weight;
    //COLA_ASSERT(wposn==ps.AD - ps.AB);
    posn=(ps.AD - ps.AB) / ps.A2;
    COLA_ASSERT(__NOTNAN(posn));
    b->deleted=true;
}

void Block::mergeIn(Block *b) {
#ifdef LIBVPSC_LOGGING
    ofstream f(LOGFILE,ios::app);
    f<<"  merging constraint heaps... "<<endl;
#endif
    // We check the top of the heaps to remove possible internal constraints
    findMinInConstraint();
    b->findMinInConstraint();
    in->merge(b->in);
#ifdef LIBVPSC_LOGGING
    f<<"  merged heap: "<<*in<<endl;
#endif
}
void Block::mergeOut(Block *b) {    
    findMinOutConstraint();
    b->findMinOutConstraint();
    out->merge(b->out);
}
Constraint *Block::findMinInConstraint() {
    Constraint *v = nullptr;
    std::vector<Constraint*> outOfDate;
    while (!in->isEmpty()) {
        v = in->findMin();
        Block *lb=v->left->block;
        Block *rb=v->right->block;
        // rb may not be this if called between merge and mergeIn
#ifdef LIBVPSC_LOGGING
        ofstream f(LOGFILE,ios::app);
        f<<"  checking constraint ... "<<*v;
        f<<"    timestamps: left="<<lb->timeStamp<<" right="<<rb->timeStamp<<" constraint="<<v->timeStamp<<endl;
#endif
        if(lb == rb) {
            // constraint has been merged into the same block
#ifdef LIBVPSC_LOGGING
            if(v->slack()<0) {
                f<<"  violated internal constraint found! "<<*v<<endl;
                f<<"     lb="<<*lb<<endl;
                f<<"     rb="<<*rb<<endl;
            }
#endif
            in->deleteMin();
#ifdef LIBVPSC_LOGGING
            f<<" ... skipping internal constraint"<<endl;
#endif
        } else if(v->timeStamp < lb->timeStamp) {
            // block at other end of constraint has been moved since this
            in->deleteMin();
            outOfDate.push_back(v);
#ifdef LIBVPSC_LOGGING
            f<<"    reinserting out of date (reinsert later)"<<endl;
#endif
        } else {
            break;
        }
    }
    for(Cit i=outOfDate.begin();i!=outOfDate.end();++i) {
        v=*i;
        v->timeStamp=blocks->blockTimeCtr;
        in->insert(v);
    }
    if(in->isEmpty()) {
        v=nullptr;
    } else {
        v=in->findMin();
    }
    return v;
}
Constraint *Block::findMinOutConstraint() {
    if(out->isEmpty()) return nullptr;
    Constraint *v = out->findMin();
    while (v->left->block == v->right->block) {
        out->deleteMin();
        if(out->isEmpty()) return nullptr;
        v = out->findMin();
    }
    return v;
}
void Block::deleteMinInConstraint() {
    in->deleteMin();
#ifdef LIBVPSC_LOGGING
    ofstream f(LOGFILE,ios::app);
    f<<"deleteMinInConstraint... "<<endl;
    f<<"  result: "<<*in<<endl;
#endif
}
void Block::deleteMinOutConstraint() {
    out->deleteMin();
}
inline bool Block::canFollowLeft(Constraint const* c, Variable const* last) const {
    return c->left->block==this && c->active && last!=c->left;
}
inline bool Block::canFollowRight(Constraint const* c, Variable const* last) const {
    return c->right->block==this && c->active && last!=c->right;
}

// computes the derivative of v and the lagrange multipliers
// of v's out constraints (as the recursive sum of those below.
// Does not backtrack over u.
// also records the constraint with minimum lagrange multiplier
// in min_lm
double Block::compute_dfdv(Variable* const v, Variable* const u,
               Constraint *&min_lm) {
    double dfdv=v->dfdv();
    for(Cit it=v->out.begin();it!=v->out.end();++it) {
        Constraint *c=*it;
        if(canFollowRight(c,u)) {
            c->lm=compute_dfdv(c->right,v,min_lm);
            dfdv+=c->lm*c->left->scale;
            if(!c->equality&&(min_lm==nullptr||c->lm<min_lm->lm)) min_lm=c;
        }
    }
    for(Cit it=v->in.begin();it!=v->in.end();++it) {
        Constraint *c=*it;
        if(canFollowLeft(c,u)) {
            c->lm=-compute_dfdv(c->left,v,min_lm);
            dfdv-=c->lm*c->right->scale;
            if(!c->equality&&(min_lm==nullptr||c->lm<min_lm->lm)) min_lm=c;
        }
    }
    return dfdv/v->scale;
}
double Block::compute_dfdv(Variable* const v, Variable* const u) {
    double dfdv = v->dfdv();
    for(Cit it = v->out.begin(); it != v->out.end(); ++it) {
        Constraint *c = *it;
        if(canFollowRight(c,u)) {
            c->lm =   compute_dfdv(c->right,v);
            dfdv += c->lm * c->left->scale;
        }
    }
    for(Cit it=v->in.begin();it!=v->in.end();++it) {
        Constraint *c = *it;
        if(canFollowLeft(c,u)) {
            c->lm = - compute_dfdv(c->left,v);
            dfdv -= c->lm * c->right->scale;
        }
    }
    return dfdv/v->scale;
}

// The top level v and r are variables between which we want to find the
// constraint with the smallest lm.  
// Similarly, m is initially nullptr and is only assigned a value if the next
// variable to be visited is r or if a possible min constraint is returned from
// a nested call (rather than nullptr).
// Then, the search for the m with minimum lm occurs as we return from
// the recursion (checking only constraints traversed left-to-right 
// in order to avoid creating any new violations).
// We also do not consider equality constraints as potential split points
bool Block::split_path(
    Variable* r, 
    Variable* const v, 
    Variable* const u, 
    Constraint* &m,
    bool desperation=false
    ) 
{
    for(Cit it(v->in.begin());it!=v->in.end();++it) {
        Constraint *c=*it;
        if(canFollowLeft(c,u)) {
#ifdef LIBVPSC_LOGGING
    ofstream f(LOGFILE,ios::app);
    f<<"  left split path: "<<*c<<endl;
#endif
            if(c->left==r) {
                if(desperation&&!c->equality) m=c;
                return true;
            } else {
                if(split_path(r,c->left,v,m)) {
                    if(desperation && !c->equality && (!m||c->lm<m->lm)) {
                               m=c;
                    }
                    return true;
                }
            }
        }
    }
    for(Cit it(v->out.begin());it!=v->out.end();++it) {
        Constraint *c=*it;
        if(canFollowRight(c,u)) {
#ifdef LIBVPSC_LOGGING
    ofstream f(LOGFILE,ios::app);
    f<<"  right split path: "<<*c<<endl;
#endif
            if(c->right==r) {
                if(!c->equality) m=c;
                return true;
            } else {
                if(split_path(r,c->right,v,m)) {
                    if(!c->equality && (!m||c->lm<m->lm))
                               m=c;
                    return true;
                }
            }
        }
    }
    return false;
}
/*
Block::Pair Block::compute_dfdv_between(
        Variable* r, Variable* const v, Variable* const u, 
        const Direction dir = NONE, bool changedDirection = false) {
    double dfdv=v->weight*(v->position() - v->desiredPosition);
    Constraint *m=nullptr;
    for(Cit it(v->in.begin());it!=v->in.end();++it) {
        Constraint *c=*it;
        if(canFollowLeft(c,u)) {
            if(dir==RIGHT) { 
                changedDirection = true; 
            }
            if(c->left==r) {
                       r=nullptr;
                    if(!c->equality) m=c; 
            }
            Pair p=compute_dfdv_between(r,c->left,v,
                    LEFT,changedDirection);
            dfdv -= c->lm = -p.first;
            if(r && p.second) 
                m = p.second;
        }
    }
    for(Cit it(v->out.begin());it!=v->out.end();++it) {
        Constraint *c=*it;
        if(canFollowRight(c,u)) {
            if(dir==LEFT) { 
                changedDirection = true; 
            }
            if(c->right==r) {
                       r=nullptr; 
                    if(!c->equality) m=c; 
            }
            Pair p=compute_dfdv_between(r,c->right,v,
                    RIGHT,changedDirection);
            dfdv += c->lm = p.first;
            if(r && p.second) 
                m = changedDirection && !c->equality && c->lm < p.second->lm 
                    ? c 
                    : p.second;
        }
    }
    return Pair(dfdv,m);
}
*/

// resets LMs for all active constraints to 0 by
// traversing active constraint tree starting from v,
// not back tracking over u
void Block::reset_active_lm(Variable* const v, Variable* const u) {
    for(Cit it=v->out.begin();it!=v->out.end();++it) {
        Constraint *c=*it;
        if(canFollowRight(c,u)) {
            c->lm=0;
            reset_active_lm(c->right,v);
        }
    }
    for(Cit it=v->in.begin();it!=v->in.end();++it) {
        Constraint *c=*it;
        if(canFollowLeft(c,u)) {
            c->lm=0;
            reset_active_lm(c->left,v);
        }
    }
}
void Block::list_active(Variable* const v, Variable* const u) {
    for(Cit it=v->out.begin();it!=v->out.end();++it) {
        Constraint *c=*it;
        if(canFollowRight(c,u)) {
#ifdef LIBVPSC_LOGGING
    ofstream f(LOGFILE,ios::app);
    f<<"  "<<*c<<endl;
#endif
            list_active(c->right,v);
        }
    }
    for(Cit it=v->in.begin();it!=v->in.end();++it) {
        Constraint *c=*it;
        if(canFollowLeft(c,u)) {
#ifdef LIBVPSC_LOGGING
    ofstream f(LOGFILE,ios::app);
    f<<"  "<<*c<<endl;
#endif
            list_active(c->left,v);
        }
    }
}
/*
 * finds the constraint with the minimum lagrange multiplier, that is, the constraint
 * that most wants to split
 */
Constraint *Block::findMinLM() {
    Constraint *min_lm=nullptr;
    reset_active_lm(vars->front(),nullptr);
    compute_dfdv(vars->front(),nullptr,min_lm);
#ifdef LIBVPSC_LOGGING
    ofstream f(LOGFILE,ios::app);
    f<<"  langrangians: "<<endl;
    list_active(vars->front(),nullptr);
#endif
    return min_lm;
}
Constraint *Block::findMinLMBetween(Variable* const lv, Variable* const rv) {
    reset_active_lm(vars->front(),nullptr);
    compute_dfdv(vars->front(),nullptr);
    Constraint *min_lm=nullptr;
    split_path(rv,lv,nullptr,min_lm);
#if 0
    if(min_lm==nullptr) {
        split_path(rv,lv,nullptr,min_lm,true);
    }
#else
    if(min_lm==nullptr) {
#ifdef LIBVPSC_DEBUG
        fprintf(stderr,"Couldn't find split point!\n");
#endif
        UnsatisfiableException e;
        getActivePathBetween(e.path,lv,rv,nullptr);
        throw e;
    }
    COLA_ASSERT(min_lm!=nullptr);
#endif
    return min_lm;
}

// populates block b by traversing the active constraint tree adding variables as they're 
// visited.  Starts from variable v and does not backtrack over variable u.
void Block::populateSplitBlock(Block *b, Variable* v, Variable const* u) {
    b->addVariable(v);
    for (Cit c=v->in.begin();c!=v->in.end();++c) {
        if (canFollowLeft(*c,u))
            populateSplitBlock(b, (*c)->left, v);
    }
    for (Cit c=v->out.begin();c!=v->out.end();++c) {
        if (canFollowRight(*c,u)) 
            populateSplitBlock(b, (*c)->right, v);
    }
}
/*
 * Returns the active path between variables u and v... not back tracking over w
 */
bool Block::getActivePathBetween(Constraints& path, Variable const* u,
               Variable const* v, Variable const *w) const {
    if(u==v) return true;
    for (Cit_const c=u->in.begin();c!=u->in.end();++c) {
        if (canFollowLeft(*c,w)) {
            if(getActivePathBetween(path, (*c)->left, v, u)) {
                path.push_back(*c);
                return true;
            }
        }
    }
    for (Cit_const c=u->out.begin();c!=u->out.end();++c) {
        if (canFollowRight(*c,w)) {
            if(getActivePathBetween(path, (*c)->right, v, u)) {
                path.push_back(*c);
                return true;
            }
        }
    }
    return false;
}
// Search active constraint tree from u to see if there is a directed path to v.
// Returns true if path is found with all constraints in path having their visited flag
// set true.
bool Block::isActiveDirectedPathBetween(Variable const* u, Variable const* v) const {
    if(u==v) return true;
    for (Cit_const c=u->out.begin();c!=u->out.end();++c) {
        if(canFollowRight(*c,nullptr)) {
            if(isActiveDirectedPathBetween((*c)->right,v)) {
                return true;
            }
        }
    }
    return false;
}
bool Block::getActiveDirectedPathBetween(
        Constraints& path, Variable const* u, Variable const* v) const {
    if(u==v) return true;
    for (Cit_const c=u->out.begin();c!=u->out.end();++c) {
        if(canFollowRight(*c,nullptr)) {
            if(getActiveDirectedPathBetween(path,(*c)->right,v)) {
                path.push_back(*c);
                return true;
            }
        }
    }
    return false;
}
/*
 * Block needs to be split because of a violated constraint between vl and vr.
 * We need to search the active constraint tree between l and r and find the constraint
 * with min lagrangrian multiplier and split at that point.
 * Returns the split constraint
 */
Constraint* Block::splitBetween(Variable* const vl, Variable* const vr,
               Block* &lb, Block* &rb) {
#ifdef LIBVPSC_LOGGING
    ofstream f(LOGFILE,ios::app);
    f<<"  need to split between: "<<*vl<<" and "<<*vr<<endl;
#endif
    Constraint *c=findMinLMBetween(vl, vr);
#ifdef LIBVPSC_LOGGING
    f<<"  going to split on: "<<*c<<endl;
#endif
    if(c!=nullptr) {
        split(lb,rb,c);
        deleted = true;
    }
    return c;
}

/*
 * Creates two new blocks, l and r, and splits this block across constraint c,
 * placing the left subtree of constraints (and associated variables) into l
 * and the right into r.
 */
void Block::split(Block* &l, Block* &r, Constraint* c) {
    c->active=false;
    l=new Block(blocks);
    populateSplitBlock(l,c->left,c->right);
    //COLA_ASSERT(l->weight>0);
    r=new Block(blocks);
    populateSplitBlock(r,c->right,c->left);
    //COLA_ASSERT(r->weight>0);
}

/*
 * Computes the cost (squared euclidean distance from desired positions) of the
 * current positions for variables in this block
 */
double Block::cost() {
    double c = 0;
    for (Vit v=vars->begin();v!=vars->end();++v) {
        double diff = (*v)->position() - (*v)->desiredPosition;
        c += (*v)->weight * diff * diff;
    }
    return c;
}
std::ostream& operator <<(std::ostream &os, const Block& b)
{
    os<<"Block(posn="<<b.posn<<"):";
    for(Block::Vit v=b.vars->begin();v!=b.vars->end();++v) {
        os<<" "<<**v;
    }
    if(b.deleted) {
        os<<" Deleted!";
    }
    return os;
}

}

