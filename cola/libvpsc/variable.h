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
 *
 * Authors:
 *   Tim Dwyer <tgdwyer@gmail.com>
 */
#ifndef SEEN_REMOVEOVERLAP_VARIABLE_H
#define SEEN_REMOVEOVERLAP_VARIABLE_H

#include <vector>
#include <iostream>
#include "libvpsc/block.h"

namespace vpsc {

class Constraint;
typedef std::vector<Constraint*> Constraints;
class Variable
{
	friend std::ostream& operator <<(std::ostream &os, const Variable &v);
	friend class Block;
	friend class Constraint;
	friend class Solver;
public:
	int id; // useful in log files
	double desiredPosition;
	double finalPosition;
	double weight; // how much the variable wants to 
	               // be at it's desired position
	double scale; // translates variable to another space
	double offset;
	Block *block;
	bool visited;
	bool fixedDesiredPosition;
	Constraints in;
	Constraints out;
	char *toString();
	inline Variable(const int id, const double desiredPos=-1.0, 
			const double weight=1.0, const double scale=1.0)
		: id(id)
		, desiredPosition(desiredPos)
		, finalPosition(desiredPos)
		, weight(weight)
		, scale(scale)
		, offset(0)
		, block(NULL)
		, visited(false)
		, fixedDesiredPosition(false)
	{
	}
	double dfdv() const {
		return 2. * weight * ( position() - desiredPosition );
	}
private:
	double position() const {
		return (block->ps.scale*block->posn+offset)/scale;
	}
};
typedef std::vector<Variable*> Variables;
}
#endif // SEEN_REMOVEOVERLAP_VARIABLE_H
