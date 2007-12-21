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

#ifndef SEEN_LIBVPSC_CONSTRAINT_H
#define SEEN_LIBVPSC_CONSTRAINT_H

#include <iostream>
#include <float.h>
#include "variable.h"
namespace vpsc {

class Constraint
{
	friend std::ostream& operator <<(std::ostream &os,const Constraint &c);
public:
	Variable *left;
	Variable *right;
	double gap;
	double lm;
	Constraint(Variable *left, Variable *right, double gap, bool equality=false);
	~Constraint();
	double slack() const;
	long timeStamp;
	bool active;
	const bool equality;
	bool unsatisfiable;
};
class CompareConstraints {
public:
	bool operator() (Constraint *const &l, Constraint *const &r) const;
};
typedef std::vector<Constraint*> Constraints;
}

#endif // SEEN_LIBVPSC_CONSTRAINT_H
