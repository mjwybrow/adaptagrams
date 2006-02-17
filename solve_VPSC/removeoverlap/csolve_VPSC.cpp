/*
 * Bridge for C programs to access solve_VPSC (which is in C++)
 */
#include <iostream>
#include <variable.h>
#include <constraint.h>
#include <solve_VPSC.h>
#include "csolve_VPSC.h"
extern "C" {
Variable* newVariable(int id, double desiredPos, double weight) {
	return new Variable(id,desiredPos,weight);
}
Constraint* newConstraint(Variable* left, Variable* right, double gap) {
	return new Constraint(left,right,gap);
}
VPSC* newVPSC(Variable* vs[], int n, Constraint* cs[], int m) {
	return new VPSC(vs,n,cs,m);
}
void deleteVPSC(VPSC *vpsc) {
	assert(vpsc!=NULL);
	delete vpsc;
}
void deleteConstraint(Constraint* c) {
	delete c;
}
void deleteVariable(Variable* v) {
	delete v;
}
void satisfyVPSC(VPSC* vpsc) {
	vpsc->satisfy();
}
void solveVPSC(VPSC* vpsc) {
	vpsc->solve();
}
void setVariableDesiredPos(Variable *v, double desiredPos) {
	v->desiredPosition = desiredPos;
}
double getVariablePos(Variable *v) {
	return v->position();
}
}
