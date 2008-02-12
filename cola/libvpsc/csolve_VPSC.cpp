/**
 * \brief Bridge for C programs to access solve_VPSC (which is in C++)
 *
 * Authors:
 *   Tim Dwyer <tgdwyer@gmail.com>
 *
 * Copyright (C) 2005 Authors
 *
 * Released under GNU LGPL.  Read the file 'COPYING' for more information.
 */
#include <iostream>
#include <cassert>
#include <vector>
#include "variable.h"
#include "constraint.h"
#include "rectangle.h"
#include "solve_VPSC.h"
#include "csolve_VPSC.h"
using namespace vpsc;
using namespace std;
extern "C" {
Variable* newVariable(int id, double desiredPos, double weight) {
	return new Variable(id,desiredPos,weight);
}
Constraint* newConstraint(Variable* left, Variable* right, double gap) {
	return new Constraint(left,right,gap);
}
Solver* newSolver(int n, Variable* vs[], int m, Constraint* cs[]) {
	vector<Variable*> vars(n);
	copy(vs,vs+n,vars.begin());
	vector<Constraint*> vcs(m);
	copy(cs,cs+m,vcs.begin());
	return new Solver(vars,vcs);
}
Solver* newIncSolver(int n, Variable* vs[], int m, Constraint* cs[]) {
	vector<Variable*> vars(n);
	copy(vs,vs+n,vars.begin());
	vector<Constraint*> vcs(m);
	copy(cs,cs+m,vcs.begin());
	return (Solver*)new vpsc::IncSolver(vars,vcs);
}

int genXConstraints(int n, boxf* bb, Variable** vs, Constraint*** cs,int transitiveClosure) {
	vector<Rectangle*> rs(n);
	vector<Variable*> vars(n);
	vector<Constraint*> lcs;
	for(int i=0;i<n;i++) {
		rs[i]=new Rectangle(bb[i].LL.x,bb[i].UR.x,bb[i].LL.y,bb[i].UR.y);
		vars[i]=vs[i];
	}
	generateXConstraints(rs,vars,lcs,transitiveClosure);
	for(int i=0;i<n;i++) {
		delete rs[i];
	}
	*cs = (Constraint**)malloc(sizeof(Constraint*)*lcs.size());
	copy(lcs.begin(),lcs.end(),*cs);
	return lcs.size();
}
int genYConstraints(int n, boxf* bb, Variable** vs, Constraint*** cs) {
	vector<Rectangle*> rs(n);
	vector<Variable*> vars(n);
	vector<Constraint*> lcs;
	for(int i=0;i<n;i++) {
		rs[i]=new Rectangle(bb[i].LL.x,bb[i].UR.x,bb[i].LL.y,bb[i].UR.y);
		vars[i]=vs[i];
	}
	generateYConstraints(rs,vars,lcs);
	for(int i=0;i<n;i++) {
		delete rs[i];
	}
	*cs = (Constraint**)malloc(sizeof(Constraint*)*lcs.size());
	copy(lcs.begin(),lcs.end(),*cs);
	return lcs.size();
}

Constraint** newConstraints(int m) {
	return new Constraint*[m];
}
void deleteConstraints(int m, Constraint **cs) {
	for(int i=0;i<m;i++) {
		delete cs[i];
	}
	delete [] cs;
}
void deleteConstraint(Constraint* c) {
	delete c;
}
void deleteVariable(Variable* v) {
	delete v;
}
void satisfyVPSC(Solver* vpsc) {
	try {
		vpsc->satisfy();
	} catch(const char *e) {
		cerr << e << endl;
		exit(1);
	}
}
int getSplitCnt(IncSolver *vpsc) {
	return vpsc->splitCnt;
}
void deleteVPSC(Solver *vpsc) {
	assert(vpsc!=NULL);
	delete vpsc;
}
void solveVPSC(Solver* vpsc) {
	vpsc->solve();
}
void splitIncVPSC(IncSolver* vpsc) {
	vpsc->splitBlocks();
}
void setVariableDesiredPos(Variable *v, double desiredPos) {
	v->desiredPosition = desiredPos;
}
double getVariablePos(Variable *v) {
	return v->finalPosition;
}
/*
void remapInConstraints(Variable *u, Variable *v, double dgap) {
	for(Constraints::iterator i=u->in.begin();i!=u->in.end();i++) {
		Constraint* c=*i;	
		c->right=v;
		c->gap+=dgap;
		v->in.push_back(c);
	}
	u->in.clear();
}
void remapOutConstraints(Variable *u, Variable *v, double dgap) {
	for(Constraints::iterator i=u->out.begin();i!=u->out.end();i++) {
		Constraint* c=*i;	
		c->left=v;
		c->gap+=dgap;
		v->out.push_back(c);
	}
	u->out.clear();
}
*/
int getLeftVarID(Constraint *c) {
	return c->left->id;
}
int getRightVarID(Constraint *c){
	return c->right->id;
}
double getSeparation(Constraint *c){
	return c->gap;
}
double getLeftScale(Constraint *c) {
	return c->left->scale;
}
double getRightScale(Constraint *c) {
	return c->right->scale;
}
}
