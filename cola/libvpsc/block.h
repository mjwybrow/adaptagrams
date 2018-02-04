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

/*
 * \brief A block is a group of variables that must be moved together to improve
 * the goal function without violating already active constraints.
 * The variables in a block are spanned by a tree of active constraints.
 */

#ifndef VPSC_BLOCK_H
#define VPSC_BLOCK_H

#include <iostream>
#include <vector>

template <class T, class TCompare> class PairingHeap;

namespace vpsc {
class Variable;
class Constraint;
class CompareConstraints;
class Blocks;

struct PositionStats {
	PositionStats() : scale(0), AB(0), AD(0), A2(0) {}
	void addVariable(Variable* const v);
	double scale;
	double AB;
	double AD;
	double A2;
};

class Block
{
	typedef std::vector<Variable*> Variables;
	typedef std::vector<Constraint*> Constraints;
	typedef Variables::iterator Vit;
	typedef Constraints::iterator Cit;
	typedef Constraints::const_iterator Cit_const;

	friend std::ostream& operator <<(std::ostream &os,const Block &b);
public:
	Variables *vars;
	double posn;
	//double weight;
	//double wposn;
	PositionStats ps;
	Block(Blocks *blocks, Variable* const v=nullptr);
	~Block(void);
	Constraint* findMinLM();
	Constraint* findMinLMBetween(Variable* const lv, Variable* const rv);
	Constraint* findMinInConstraint();
	Constraint* findMinOutConstraint();
	void deleteMinInConstraint();
	void deleteMinOutConstraint();
	void updateWeightedPosition();
	void merge(Block *b, Constraint *c, double dist);
	Block* merge(Block *b, Constraint *c);
	void mergeIn(Block *b);
	void mergeOut(Block *b);
	void split(Block *&l, Block *&r, Constraint *c);
	Constraint* splitBetween(Variable* vl, Variable* vr, Block* &lb, Block* &rb);
	void setUpInConstraints();
	void setUpOutConstraints();
	double cost();
	bool deleted;
	long timeStamp;
	PairingHeap<Constraint*,CompareConstraints> *in;
	PairingHeap<Constraint*,CompareConstraints> *out;
	bool getActivePathBetween(Constraints& path, Variable const* u,
	       	Variable const* v, Variable const *w) const;
	bool isActiveDirectedPathBetween(
			Variable const* u, Variable const* v) const;
	bool getActiveDirectedPathBetween(Constraints& path, Variable const * u, Variable const * v) const;
private:
	typedef enum {NONE, LEFT, RIGHT} Direction;
	typedef std::pair<double, Constraint*> Pair;
	void reset_active_lm(Variable* const v, Variable* const u);
	void list_active(Variable* const v, Variable* const u);
	double compute_dfdv(Variable* const v, Variable* const u);
	double compute_dfdv(Variable* const v, Variable* const u, Constraint *&min_lm);
	bool split_path(Variable*, Variable* const, Variable* const, 
			Constraint* &min_lm, bool desperation);
	bool canFollowLeft(Constraint const* c, Variable const* last) const;
	bool canFollowRight(Constraint const* c, Variable const* last) const;
	void populateSplitBlock(Block *b, Variable* v, Variable const* u);
	void addVariable(Variable* v);
	void setUpConstraintHeap(PairingHeap<Constraint*,CompareConstraints>* &h,bool in);

    // Parent container, that holds the blockTimeCtr.
    Blocks *blocks;
};
}

#endif // VPSC_BLOCK_H
