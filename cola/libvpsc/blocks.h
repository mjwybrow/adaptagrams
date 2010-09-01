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
 * \brief A block structure defined over the variables
 *
 * A block structure defined over the variables such that each block contains
 * 1 or more variables, with the invariant that all constraints inside a block
 * are satisfied by keeping the variables fixed relative to one another
 *
 * Authors:
 *   Tim Dwyer <tgdwyer@gmail.com>
 */

#ifndef SEEN_LIBVPSC_BLOCKS_H
#define SEEN_LIBVPSC_BLOCKS_H

#ifdef LIBVPSC_LOGGING
#define LOGFILE "libvpsc.log"
#endif

#include <set>
#include <list>
#include <vector>

namespace vpsc {
class Block;
class Variable;
class Constraint;
/**
 * A block structure defined over the variables such that each block contains
 * 1 or more variables, with the invariant that all constraints inside a block
 * are satisfied by keeping the variables fixed relative to one another
 */
class Blocks : public std::set<Block*>
{
public:
	Blocks(std::vector<Variable*> const &vs);
	~Blocks(void);
	void mergeLeft(Block *r);
	void mergeRight(Block *l);
	void split(Block *b, Block *&l, Block *&r, Constraint *c);
	std::list<Variable*> *totalOrder();
	void cleanup();
	double cost();
    
    long blockTimeCtr;
private:
	void dfsVisit(Variable *v, std::list<Variable*> *order);
	void removeBlock(Block *doomed);
	std::vector<Variable*> const &vs;
	int nvs;
};

}
#endif // SEEN_LIBVPSC_BLOCKS_H
