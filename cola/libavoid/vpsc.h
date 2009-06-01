/*
 * vim: ts=4 sw=4 et tw=0 wm=0
 *
 * libavoid - Fast, Incremental, Object-avoiding Line Router
 *
 * Copyright (C) 2005-2009  Monash University
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * See the file LICENSE.LGPL distributed with the library.
 *
 * Licensees holding a valid commercial license may use this file in
 * accordance with the commercial license agreement provided with the 
 * library.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  
 *
 * Author(s):   Tim Dwyer  <Tim.Dwyer@csse.monash.edu.au>
 *
 * --------------
 *
 * This file contains a slightly modified version of Solver() from libvpsc:
 * A solver for the problem of Variable Placement with Separation Constraints.
 * It has the following changes from the Adaptagrams VPSC version:
 *  -  The required VPSC code has been consolidated into a single file.
 *  -  Unnecessary code (like IncSolver) has been removed.
 *  -  The PairingHeap code has been replaced by a STL priority_queue.
 *
 * Modifications:  Michael Wybrow  <mjwybrow@users.sourceforge.net>
 *
*/

#ifndef LIBAVOID_VPSC_H
#define LIBAVOID_VPSC_H

#include <vector>
#include <list>
#include <set>
#include <queue>

namespace Avoid {

class Variable;
class Constraint;
class CompareConstraints {
public:
	bool operator() (Constraint *const &l, Constraint *const &r) const;
};
struct PositionStats {
	PositionStats() : scale(0), AB(0), AD(0), A2(0) {}
	void addVariable(Variable* const v);
	double scale;
	double AB;
	double AD;
	double A2;
};

typedef std::priority_queue<Constraint*,std::vector<Constraint*>,
        CompareConstraints> Heap;

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
	Block(Variable* const v=NULL);
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
    Heap *in;
    Heap *out;
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
	void setUpConstraintHeap(Heap* &h,bool in);
};


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
typedef std::vector<Constraint*> Constraints;
/*
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
private:
	void dfsVisit(Variable *v, std::list<Variable*> *order);
	void removeBlock(Block *doomed);
	std::vector<Variable*> const &vs;
	int nvs;
};

extern long blockTimeCtr;

struct UnsatisfiableException {
	std::vector<Constraint*> path;
};
struct UnsatisfiedConstraint {
	UnsatisfiedConstraint(Constraint& c):c(c) {}
	Constraint& c;
};
/*
 * Variable Placement with Separation Constraints problem instance
 */
class Solver {
public:
	// the following two methods both attempt to solve a least-squares
	// problem subject to a set of sepation constraints.  They return
	// true if any constraints are active, in both cases false means
	// an unconstrained optimum has been found.
	// satisfy returns an approximate solution subject to the constraints
	virtual bool satisfy();
	// solve returns an optimum solution subject to the constraints
	virtual bool solve();

	Solver(std::vector<Variable*> const &vs, std::vector<Constraint *> const &cs);
	virtual ~Solver();
	std::vector<Variable*> const & getVariables() { return vs; }
protected:
	Blocks *bs;
	unsigned m;
	std::vector<Constraint*> const &cs;
	unsigned n;
	std::vector<Variable*> const &vs;
	void printBlocks();
	void copyResult();
private:
	void refine();
	bool constraintGraphIsCyclic(const unsigned n, Variable* const vs[]);
	bool blockGraphIsCyclic();
};

struct delete_object
{
    template <typename T>
    void operator()(T *ptr){ delete ptr;}
};


}

#endif // AVOID_VPSC_H
