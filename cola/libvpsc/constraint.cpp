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
#include <cassert>
namespace vpsc {
Constraint::Constraint(Variable *left, Variable *right, double gap, bool equality)
: left(left),
  right(right),
  gap(gap),
  timeStamp(0),
  active(false),
  equality(equality)
{
	// In hindsight I think it's probably better to build the constraint DAG
	// (by creating variable in/out lists) when needed, rather than in advance
	//left->out.push_back(this);
	//right->in.push_back(this);
}
Constraint::~Constraint() {
	// see constructor: the following is just way too slow.  Better to create a
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
std::ostream& operator <<(std::ostream &os, const Constraint &c)
{
	if(&c==NULL) {
		os<<"NULL";
	} else {
		const char *type=c.equality?"=":"<=";
		os<<*c.left<<"+"<<c.gap<<type<<*c.right<<"("<<c.slack()<<")"<<(c.active?"-active":"");
	}
	return os;
}
}
