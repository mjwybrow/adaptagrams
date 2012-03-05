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

/**
 * \brief A constraint determines a minimum or exact spacing required between
 * two variables.
 *
 * Authors:
 *   Tim Dwyer <tgdwyer@gmail.com>
 */

#include <sstream>
#include <cassert>
#include <cfloat>

#include "libvpsc/constraint.h"
#include "libvpsc/variable.h"

namespace vpsc {
Constraint::Constraint(Variable *left, Variable *right, double gap, bool equality)
: left(left),
  right(right),
  gap(gap),
  timeStamp(0),
  active(false),
  equality(equality),
  unsatisfiable(false)
{
    // In hindsight I think it's probably better to build the constraint DAG
    // (by creating variable in/out lists) when needed, rather than in advance
    //left->out.push_back(this);
    //right->in.push_back(this);
}
Constraint::~Constraint() {
    // see constructor: the following is just way too slow.  
    // Better to create a
    // new DAG on demand than maintain the lists dynamically.
    //Constraints::iterator i;
    //for(i=left->out.begin(); i!=left->out.end(); i++) {
        //if(*i==this) break;
    //}
    //left->out.erase(i);
    //for(i=right->in.begin(); i!=right->in.end(); i++) {
        //if(*i==this) break;
    //}
    //right->in.erase(i);
}
double Constraint::slack() const { 
    return unsatisfiable ? DBL_MAX
           : right->scale * right->position() 
         - gap - left->scale * left->position(); 
}
std::ostream& operator <<(std::ostream &os, const Constraint &c)
{
    if(&c==NULL) {
        os<<"NULL";
    } else {
        const char *type=c.equality?"=":"<=";
        std::ostringstream lscale, rscale;
        if(c.left->scale!=1) {
            lscale << c.left->scale << "*";
        }
        if(c.right->scale!=1) {
            rscale << c.right->scale << "*";
        }
        os<<lscale.str()<<*c.left<<"+"<<c.gap<<type<<rscale.str()<<*c.right;
        if(c.left->block&&c.right->block)
            os<<"("<<c.slack()<<")"<<(c.active?"-active":"")
                <<"(lm="<<c.lm<<")";
        else
            os<<"(vars have no position)";
    }
    return os;
}
#include "libvpsc/block.h"
bool CompareConstraints::operator() (
    Constraint *const &l, Constraint *const &r
) const {
    double const sl = 
        l->left->block->timeStamp > l->timeStamp
        ||l->left->block==l->right->block
        ?-DBL_MAX:l->slack();
    double const sr = 
        r->left->block->timeStamp > r->timeStamp
        ||r->left->block==r->right->block
        ?-DBL_MAX:r->slack();
    if(sl==sr) {
        // arbitrary choice based on id
        if(l->left->id==r->left->id) {
            if(l->right->id<r->right->id) return true;
            return false;
        }
        if(l->left->id<r->left->id) return true;
        return false;
    }
    return sl < sr;
}
}
