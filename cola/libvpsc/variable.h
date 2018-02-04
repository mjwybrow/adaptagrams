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

#ifndef VPSC_VARIABLE_H
#define VPSC_VARIABLE_H

#include <vector>
#include <iostream>

#include "libvpsc/block.h"
#include "libvpsc/assertions.h"

namespace vpsc {

class Constraint;
typedef std::vector<Constraint*> Constraints;

/**
 * @brief A variable is comprised of an ideal position, final position and 
 *        a weight.
 *
 * When creating a variable you specify an ideal value, and a weight---how 
 * much the variable wants to be at its ideal position.  After solving the 
 * problem you can read back the final position for the variable.
*/
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
		, block(nullptr)
		, visited(false)
		, fixedDesiredPosition(false)
	{
	}
	double dfdv(void) const {
		return 2. * weight * ( position() - desiredPosition );
	}
private:
    inline double position(void) const {
                return (block->ps.scale*block->posn+offset)/scale;
    }
	inline double unscaledPosition(void) const {
	    COLA_ASSERT(block->ps.scale == 1);
        COLA_ASSERT(scale == 1);
        return block->posn + offset;
	}
};

//! @brief A vector of pointers to Variable objects.
typedef std::vector<Variable*> Variables;

}
#endif // VPSC_VARIABLE_H
