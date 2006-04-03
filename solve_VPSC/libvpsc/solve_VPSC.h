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

#ifndef SEEN_REMOVEOVERLAP_SOLVE_VPSC_H
#define SEEN_REMOVEOVERLAP_SOLVE_VPSC_H

#include <list>
class Variable;
class Constraint;
class Blocks;

/**
 * Variable Placement with Separation Constraints problem instance
 */
class VPSC {
public:
	virtual void satisfy();
	virtual void solve();

	VPSC(const int n, Variable *vs[], const int m, Constraint *cs[]);
	~VPSC();
protected:
	Blocks *bs;
	Constraint **cs;
	int m;
	void printBlocks();
private:
	void refine();
	bool constraintGraphIsCyclic(const int n, Variable *vs[]);
	bool blockGraphIsCyclic();
};

class IncVPSC : VPSC {
public:
	int splitCnt;
	void satisfy();
	void solve();
	void moveBlocks();
	void splitBlocks();
	IncVPSC(const int n, Variable *vs[], const int m, Constraint *cs[]);
private:
	typedef std::list<Constraint*> ConstraintList;
	ConstraintList inactive;
	double mostViolated(ConstraintList &l,Constraint* &v);
};
#endif // SEEN_REMOVEOVERLAP_SOLVE_VPSC_H
