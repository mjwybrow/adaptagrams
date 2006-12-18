/**
 * \brief A block is a group of variables that must be moved together to improve
 * the goal function without violating already active constraints.
 * The variables in a block are spanned by a tree of active constraints.
 *
 * Authors:
 *   Tim Dwyer <tgdwyer@gmail.com>
 *
 * Copyright (C) 2005 Authors
 *
 * Released under GNU LGPL.  Read the file 'COPYING' for more information.
 */

#ifndef SEEN_LIBVPSC_BLOCK_H
#define SEEN_LIBVPSC_BLOCK_H

#include <vector>
#include <iostream>
template <class T, class TCompare> class PairingHeap;
namespace vpsc {
class Variable;
class Constraint;
class CompareConstraints;

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
	double weight;
	double wposn;
	Block(Variable* const v=NULL);
	~Block(void);
	Constraint* findMinLM();
	Constraint* findMinLMBetween(Variable* const lv, Variable* const rv);
	Constraint* findMinInConstraint();
	Constraint* findMinOutConstraint();
	void deleteMinInConstraint();
	void deleteMinOutConstraint();
	double desiredWeightedPosition();
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
	double compute_dfdv(Variable* const v, Variable* const u);
	double compute_dfdv(Variable* const v, Variable* const u, Constraint *&min_lm);
	bool split_path(Variable*, Variable* const, Variable* const, 
			Constraint* &min_lm);
	bool canFollowLeft(Constraint const* c, Variable const* last) const;
	bool canFollowRight(Constraint const* c, Variable const* last) const;
	void populateSplitBlock(Block *b, Variable* v, Variable const* u);
	void addVariable(Variable* v);
	void setUpConstraintHeap(PairingHeap<Constraint*,CompareConstraints>* &h,bool in);
};

}
#endif // SEEN_LIBVPSC_BLOCK_H
