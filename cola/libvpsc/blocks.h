/*
 * vim: ts=4 sw=4 et tw=0 wm=0
 *
 * libvpsc - A solver for the problem of Variable Placement with 
 *           Separation Constraints.
 *
 * Copyright (C) 2005-2012  Monash University
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

/*
 * @brief A block structure defined over the variables
 *
 * A block structure defined over the variables such that each block contains
 * 1 or more variables, with the invariant that all constraints inside a block
 * are satisfied by keeping the variables fixed relative to one another
 *
 */

#ifndef VPSC_BLOCKS_H
#define VPSC_BLOCKS_H

#ifdef LIBVPSC_LOGGING
#define LOGFILE "libvpsc.log"
#endif

#include <list>
#include <vector>

// size_t is strangely not defined on some older MinGW GCC versions. 
#include <cstddef>

namespace vpsc {
class Block;
class Variable;
class Constraint;
/*
 * A block structure defined over the variables such that each block contains
 * 1 or more variables, with the invariant that all constraints inside a block
 * are satisfied by keeping the variables fixed relative to one another
 */
class Blocks
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

    size_t size() const;
    Block *at(size_t index) const;
    void insert(Block *block);
    
    long blockTimeCtr;
private:
	void dfsVisit(Variable *v, std::list<Variable*> *order);
	void removeBlock(Block *doomed);
    
    std::vector<Block*> m_blocks;
	std::vector<Variable*> const &vs;
	size_t nvs;
};

inline size_t Blocks::size() const
{
    return m_blocks.size();
}

inline Block *Blocks::at(size_t index) const
{
    return m_blocks[index];
}

inline void Blocks::insert(Block *block)
{
    m_blocks.push_back(block);
}

}
#endif // VPSC_BLOCKS_H
