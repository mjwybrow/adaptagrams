/**
 * \brief A constraint determines a minimum or exact spacing required between
 * two variables.
 *
 * Authors:
 *   Tim Dwyer <tgdwyer@gmail.com>
 *
 * Copyright (C) 2005 Authors
 *
 * Released under GNU LGPL.  Read the file 'COPYING' for more information.
 */

#include "constraint.h"
#include <sstream>
#include <cassert>
#include <cfloat>
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
#include "block.h"
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
