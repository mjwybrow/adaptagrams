/**
 * \brief Remove overlaps function
 *
 * Authors:
 *   Tim Dwyer <tgdwyer@gmail.com>
 *
 * Copyright (C) 2005 Authors
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

#ifndef SEEN_REMOVEOVERLAP_BLOCKS_H
#define SEEN_REMOVEOVERLAP_BLOCKS_H

#ifdef RECTANGLE_OVERLAP_LOGGING
#define LOGFILE "cRectangleOverlap.log"
#endif

#include <set>
#include <list>

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
	Blocks(Variable *vs[], const int n);
	~Blocks(void);
	void mergeLeft(Block *r);
	void mergeRight(Block *l);
	void split(Block *b, Block *&l, Block *&r, Constraint *c);
	std::list<Variable*> *totalOrder();
	void cleanup();
	double cost();
private:
	void dfsVisit(Variable *v, std::list<Variable*> *order);
	void removeBlock(Block *doomed);
	Variable **vs;
	int nvs;
};

extern long blockTimeCtr;
#endif // SEEN_REMOVEOVERLAP_BLOCKS_H
