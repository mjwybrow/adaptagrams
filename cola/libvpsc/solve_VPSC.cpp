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
 *             Michael Wybrow
*/

#include <cmath>
#include <sstream>
#include <map>
#include <cfloat>
#include <set>

#include "libvpsc/constraint.h"
#include "libvpsc/block.h"
#include "libvpsc/blocks.h"
#include "libvpsc/solve_VPSC.h"
#include "libvpsc/cbuffer.h"
#include "libvpsc/variable.h"
#include "libvpsc/assertions.h"
#include "libvpsc/exceptions.h"

#ifdef LIBVPSC_LOGGING
#include <fstream>
#endif

using namespace std;

namespace vpsc {

static const double ZERO_UPPERBOUND=-1e-10;
static const double LAGRANGIAN_TOLERANCE=-1e-4;

IncSolver::IncSolver(Variables const &vs, Constraints const &cs) 
    : Solver(vs,cs)
{
    inactive=cs;
    for(Constraints::iterator i=inactive.begin();i!=inactive.end();++i) {
        (*i)->active=false;
    }
}
Solver::Solver(Variables const &vs, Constraints const &cs) 
    : m(cs.size()), 
      cs(cs),
      n(vs.size()),
      vs(vs),
      needsScaling(false)
{
    for(unsigned i=0;i<n;++i) {
        vs[i]->in.clear();
        vs[i]->out.clear();

        // Set needsScaling if any variables have a scale other than 1.
        needsScaling |= (vs[i]->scale != 1);
    }
    for(unsigned i=0;i<m;++i) {
        Constraint *c=cs[i];
        c->left->out.push_back(c);
        c->right->in.push_back(c);
        c->needsScaling = needsScaling;
    }
    bs=new Blocks(vs);
#ifdef LIBVPSC_LOGGING
    printBlocks();
    //COLA_ASSERT(!constraintGraphIsCyclic(n,vs));
#endif
}
Solver::~Solver() {
    delete bs;
}

void IncSolver::addConstraint(Constraint *c)
{
    ++m;
    c->active = false;
    inactive.push_back(c);
    c->left->out.push_back(c);
    c->right->in.push_back(c);
    c->needsScaling = needsScaling;
}

// useful in debugging
void Solver::printBlocks() {
#ifdef LIBVPSC_LOGGING
    ofstream f(LOGFILE,ios::app);
    for(set<Block*>::iterator i=bs->begin();i!=bs->end();++i) {
        Block *b=*i;
        f<<"  "<<*b<<endl;
    }
    for(unsigned i=0;i<m;i++) {
        f<<"  "<<*cs[i]<<endl;
    }
#endif
}

/**
 * Stores the relative positions of the variables in their finalPosition
 * field.
 */
void Solver::copyResult() {
    for(Variables::const_iterator i=vs.begin();i!=vs.end();++i) {
        Variable* v=*i;
        v->finalPosition=v->position();
        COLA_ASSERT(v->finalPosition==v->finalPosition);
    }
}
/**
* Produces a feasible - though not necessarily optimal - solution by
* examining blocks in the partial order defined by the directed acyclic
* graph of constraints. For each block (when processing left to right) we
* maintain the invariant that all constraints to the left of the block
* (incoming constraints) are satisfied. This is done by repeatedly merging
* blocks into bigger blocks across violated constraints (most violated
* first) fixing the position of variables inside blocks relative to one
* another so that constraints internal to the block are satisfied.
*/
bool Solver::satisfy() {
    list<Variable*> *vList=bs->totalOrder();
    for(list<Variable*>::iterator i=vList->begin();i!=vList->end();++i) {
        Variable *v=*i;
        if(!v->block->deleted) {
            bs->mergeLeft(v->block);
        }
    }
    bs->cleanup();
    bool activeConstraints=false;
    for(unsigned i=0;i<m;i++) {
        if(cs[i]->active) activeConstraints=true;
        if(cs[i]->slack() < ZERO_UPPERBOUND) {
#ifdef LIBVPSC_LOGGING
            ofstream f(LOGFILE,ios::app);
            f<<"Error: Unsatisfied constraint: "<<*cs[i]<<endl;
#endif
            //COLA_ASSERT(cs[i]->slack()>-0.0000001);
            throw UnsatisfiedConstraint(*cs[i]);
        }
    }
    delete vList;
    copyResult();
    return activeConstraints;
}

void Solver::refine() {
    bool solved=false;
    // Solve shouldn't loop indefinately
    // ... but just to make sure we limit the number of iterations
    unsigned maxtries=100;
    while(!solved&&maxtries>0) {
        solved=true;
        maxtries--;
        size_t length = bs->size();
        for (size_t i = 0; i < length; ++i)
        {
            Block *b = bs->at(i);
            b->setUpInConstraints();
            b->setUpOutConstraints();
        }
        for (size_t i = 0; i < length; ++i)
        {
            Block *b = bs->at(i);
            Constraint *c=b->findMinLM();
            if(c!=nullptr && c->lm<LAGRANGIAN_TOLERANCE) {
#ifdef LIBVPSC_LOGGING
                ofstream f(LOGFILE,ios::app);
                f<<"Split on constraint: "<<*c<<endl;
#endif
                // Split on c
                Block *l=nullptr, *r=nullptr;
                bs->split(b,l,r,c);
                bs->cleanup();
                // split alters the block set so we have to restart
                solved=false;
                break;
            }
        }
    }
    for(unsigned i=0;i<m;i++) {
        if(cs[i]->slack() < ZERO_UPPERBOUND) {
            COLA_ASSERT(cs[i]->slack()>ZERO_UPPERBOUND);
            throw UnsatisfiedConstraint(*cs[i]);
        }
    }
}
/**
 * Calculate the optimal solution. After using satisfy() to produce a
 * feasible solution, refine() examines each block to see if further
 * refinement is possible by splitting the block. This is done repeatedly
 * until no further improvement is possible.
 */
bool Solver::solve() {
    satisfy();
    refine();
    copyResult();
    return bs->size()!=n;
}

bool IncSolver::solve() {
#ifdef LIBVPSC_LOGGING
    ofstream f(LOGFILE,ios::app);
    f<<"solve_inc()..."<<endl;
#endif
    satisfy();
    double lastcost = DBL_MAX, cost = bs->cost();
    while(fabs(lastcost-cost)>0.0001) {
        satisfy();
        lastcost=cost;
        cost = bs->cost();
#ifdef LIBVPSC_LOGGING
        f<<"  bs->size="<<bs->size()<<", cost="<<cost<<endl;
#endif
    }
    copyResult();
    return bs->size()!=n; 
}
/**
 * incremental version of satisfy that allows refinement after blocks are
 * moved.
 *
 *  - move blocks to new positions
 *  - repeatedly merge across most violated constraint until no more
 *    violated constraints exist
 *
 * Note: there is a special case to handle when the most violated constraint
 * is between two variables in the same block.  Then, we must split the block
 * over an active constraint between the two variables.  We choose the 
 * constraint with the most negative lagrangian multiplier. 
 */
bool IncSolver::satisfy() {
#ifdef LIBVPSC_LOGGING
    ofstream f(LOGFILE,ios::app);
    f<<"satisfy_inc()..."<<endl;
#endif
    splitBlocks();
    //long splitCtr = 0;
    Constraint* v = nullptr;
    //CBuffer buffer(inactive);
    while ( (v = mostViolated(inactive)) && 
            (v->equality || ((v->slack() < ZERO_UPPERBOUND) && !v->active)) ) 
    {
        COLA_ASSERT(!v->active);
        Block *lb = v->left->block, *rb = v->right->block;
        if(lb != rb) {
            lb->merge(rb,v);
        } else {
            if(lb->isActiveDirectedPathBetween(v->right,v->left)) {
                // cycle found, relax the violated, cyclic constraint
                v->unsatisfiable=true;
                continue;
                //UnsatisfiableException e;
                //lb->getActiveDirectedPathBetween(e.path,v->right,v->left);
                //e.path.push_back(v);
                //throw e;
            }
            //if(splitCtr++>10000) {
                //throw "Cycle Error!";
            //}
            // constraint is within block, need to split first
            try {
                Constraint* splitConstraint
                    =lb->splitBetween(v->left,v->right,lb,rb);
                if(splitConstraint!=nullptr) {
                    COLA_ASSERT(!splitConstraint->active);
                    inactive.push_back(splitConstraint);
                } else {
                    v->unsatisfiable=true;
                    continue;
                }
            } catch(UnsatisfiableException e) {
                e.path.push_back(v);
#ifdef LIBVPSC_DEBUG
                std::cerr << "Unsatisfiable:" << std::endl;
                for(std::vector<Constraint*>::iterator r=e.path.begin();
                        r!=e.path.end();++r)
                {
                    std::cerr << **r <<std::endl;
                }
#endif
                v->unsatisfiable=true;
                continue;
            }
            if(v->slack()>=0) {
                COLA_ASSERT(!v->active);
                // v was satisfied by the above split!
                inactive.push_back(v);
                bs->insert(lb);
                bs->insert(rb);
            } else {
                bs->insert(lb->merge(rb,v));
                delete ((lb->deleted) ? lb : rb);
            }
        }
#ifdef LIBVPSC_LOGGING
        f<<"...remaining blocks="<<bs->size()<<", cost="<<bs->cost()<<endl;
#endif
    }
#ifdef LIBVPSC_LOGGING
    f<<"  finished merges."<<endl;
#endif
    bs->cleanup();
    bool activeConstraints=false;
    for(unsigned i=0;i<m;i++) {
        v=cs[i];
        if(v->active) activeConstraints=true;
        if(v->slack() < ZERO_UPPERBOUND) {
            ostringstream s;
            s<<"Unsatisfied constraint: "<<*v;
#ifdef LIBVPSC_LOGGING
            ofstream f(LOGFILE,ios::app);
            f<<s.str()<<endl;
#endif
            throw (char *) s.str().c_str();
        }
    }
#ifdef LIBVPSC_LOGGING
    f<<"  finished cleanup."<<endl;
    printBlocks();
#endif
    copyResult();
    return activeConstraints;
}
void IncSolver::moveBlocks() {
#ifdef LIBVPSC_LOGGING
    ofstream f(LOGFILE,ios::app);
    f<<"moveBlocks()..."<<endl;
#endif
    size_t length = bs->size();
    for (size_t i = 0; i < length; ++i)
    {
        Block *b = bs->at(i);
        b->updateWeightedPosition();
        //b->posn = b->wposn / b->weight;
    }
#ifdef LIBVPSC_LOGGING
    f<<"  moved blocks."<<endl;
#endif
}
void IncSolver::splitBlocks() {
#ifdef LIBVPSC_LOGGING
    ofstream f(LOGFILE,ios::app);
#endif
    moveBlocks();
    splitCnt=0;
    // Split each block if necessary on min LM
    size_t length = bs->size();
    for (size_t i = 0; i < length; ++i)
    {
        Block *b = bs->at(i);
        Constraint* v=b->findMinLM();
        if(v!=nullptr && v->lm < LAGRANGIAN_TOLERANCE) {
            COLA_ASSERT(!v->equality);
#ifdef LIBVPSC_LOGGING
            f<<"    found split point: "<<*v<<" lm="<<v->lm<<endl;
#endif
            splitCnt++;
            Block *b = v->left->block, *l=nullptr, *r=nullptr;
            COLA_ASSERT(v->left->block == v->right->block);
            //double pos = b->posn;
            b->split(l,r,v);
            //l->posn=r->posn=pos;
            //l->wposn = l->posn * l->weight;
            //r->wposn = r->posn * r->weight;
            l->updateWeightedPosition();
            r->updateWeightedPosition();
            bs->insert(l);
            bs->insert(r);
            b->deleted=true;
            COLA_ASSERT(!v->active);
            inactive.push_back(v);
#ifdef LIBVPSC_LOGGING
            f<<"  new blocks: "<<*l<<" and "<<*r<<endl;
#endif
        }
    }
    //if(splitCnt>0) { std::cout<<"  splits: "<<splitCnt<<endl; }
#ifdef LIBVPSC_LOGGING
    f<<"  finished splits."<<endl;
#endif
    bs->cleanup();
}

/**
 * Scan constraint list for the most violated constraint, or the first equality
 * constraint
 */
Constraint* IncSolver::mostViolated(Constraints &l)
{
    double slackForMostViolated = DBL_MAX;
    Constraint* mostViolated = nullptr;
#ifdef LIBVPSC_LOGGING
    ofstream f(LOGFILE,ios::app);
    f << "Looking for most violated..." << endl;
#endif
    size_t lSize = l.size();
    size_t deleteIndex = lSize;
    Constraint *constraint = nullptr;
    double slack = 0;
    for (size_t index = 0; index < lSize; ++index)
    {
        constraint = l[index];
        slack = constraint->slack();
        if (constraint->equality || slack < slackForMostViolated)
        {
            slackForMostViolated = slack;    
            mostViolated = constraint;
            deleteIndex = index;
            if (constraint->equality)
            {
                break;
            }
        }
    }
    // Because the constraint list is not order dependent we just
    // move the last element over the deletePoint and resize
    // downwards.  There is always at least 1 element in the
    // vector because of search.
    if ( (deleteIndex < lSize) && 
         (((slackForMostViolated < ZERO_UPPERBOUND) && !mostViolated->active) || 
          mostViolated->equality) )
    {
        l[deleteIndex] = l[lSize-1];
        l.resize(lSize-1);
    }
#ifdef LIBVPSC_LOGGING
    if (mostViolated)
    {
        f << "  most violated is: " << *mostViolated << endl;
    }
    else
    {
        f << "  non found." << endl;
    }
#endif
    return mostViolated;
}

struct node {
    set<node*> in;
    set<node*> out;
};
// useful in debugging - cycles would be BAD
bool Solver::constraintGraphIsCyclic(const unsigned n, Variable* const vs[]) {
    map<Variable*, node*> varmap;
    vector<node*> graph;
    for(unsigned i=0;i<n;i++) {
        node *u=new node;
        graph.push_back(u);
        varmap[vs[i]]=u;
    }
    for(unsigned i=0;i<n;i++) {
        for(vector<Constraint*>::iterator c=vs[i]->in.begin();c!=vs[i]->in.end();++c) {
            Variable *l=(*c)->left;
            varmap[vs[i]]->in.insert(varmap[l]);
        }

        for(vector<Constraint*>::iterator c=vs[i]->out.begin();c!=vs[i]->out.end();++c) {
            Variable *r=(*c)->right;
            varmap[vs[i]]->out.insert(varmap[r]);
        }
    }
    while(graph.size()>0) {
        node *u=nullptr;
        vector<node*>::iterator i=graph.begin();
        for(;i!=graph.end();++i) {
            u=*i;
            if(u->in.size()==0) {
                break;
            }
        }
        if(i==graph.end() && graph.size()>0) {
            //cycle found!
            return true;
        } else {
            graph.erase(i);
            for(set<node*>::iterator j=u->out.begin();j!=u->out.end();++j) {
                node *v=*j;
                v->in.erase(u);
            }
            delete u;
        }
    }
    for(unsigned i=0; i<graph.size(); ++i) {
        delete graph[i];
    }
    return false;
}

// useful in debugging - cycles would be BAD
bool Solver::blockGraphIsCyclic() {
    map<Block*, node*> bmap;
    vector<node*> graph;
    size_t length = bs->size();
    for (size_t i = 0; i < length; ++i)
    {
        Block *b = bs->at(i);
        node *u=new node;
        graph.push_back(u);
        bmap[b]=u;
    }
    for (size_t i = 0; i < length; ++i)
    {
        Block *b = bs->at(i);
        b->setUpInConstraints();
        Constraint *c=b->findMinInConstraint();
        while(c!=nullptr) {
            Block *l=c->left->block;
            bmap[b]->in.insert(bmap[l]);
            b->deleteMinInConstraint();
            c=b->findMinInConstraint();
        }

        b->setUpOutConstraints();
        c=b->findMinOutConstraint();
        while(c!=nullptr) {
            Block *r=c->right->block;
            bmap[b]->out.insert(bmap[r]);
            b->deleteMinOutConstraint();
            c=b->findMinOutConstraint();
        }
    }
    while(graph.size()>0) {
        node *u=nullptr;
        vector<node*>::iterator i=graph.begin();
        for(;i!=graph.end();++i) {
            u=*i;
            if(u->in.size()==0) {
                break;
            }
        }
        if(i==graph.end() && graph.size()>0) {
            //cycle found!
            return true;
        } else {
            graph.erase(i);
            for(set<node*>::iterator j=u->out.begin();j!=u->out.end();++j) {
                node *v=*j;
                v->in.erase(u);
            }
            delete u;
        }
    }
    for(unsigned i=0; i<graph.size(); i++) {
        delete graph[i];
    }
    return false;
}
}
