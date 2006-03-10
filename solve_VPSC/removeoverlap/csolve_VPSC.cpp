/*
 * Bridge for C programs to access solve_VPSC (which is in C++)
 */
#include <iostream>
#include <variable.h>
#include <constraint.h>
#include <generate-constraints.h>
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
IncVPSC* newIncVPSC(Variable* vs[], int n, Constraint* cs[], int m) {
	return new IncVPSC(vs,n,cs,m);
}

int genXConstraints(double minX[], double maxX[], double minY[], double maxY[], int n, Variable*** vs, Constraint*** cs) {
	Rectangle* rs[n];
	double weights[n]; 
	for(int i=0;i<n;i++) {
		rs[i]=new Rectangle(minX[i],maxX[i],minY[i],maxY[i]);
		weights[i]=1;
	}
	int m = generateXConstraints(rs,weights,n,*vs,*cs,true);
	for(int i=0;i<n;i++) {
		delete rs[i];
	}
	return m;
}
int genYConstraints(double minX[], double maxX[], double minY[], double maxY[], int n, Variable*** vs, Constraint*** cs) {
	Rectangle* rs[n];
	double weights[n]; 
	for(int i=0;i<n;i++) {
		rs[i]=new Rectangle(minX[i],maxX[i],minY[i],maxY[i]);
		weights[i]=1;
	}
	int m = generateYConstraints(rs,weights,n,*vs,*cs);
	for(int i=0;i<n;i++) {
		delete rs[i];
	}
	return m;
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
void deleteVPSC(VPSC *vpsc) {
	assert(vpsc!=NULL);
	delete vpsc;
}
void solveVPSC(VPSC* vpsc) {
	vpsc->solve();
}
void splitIncVPSC(IncVPSC* vpsc) {
	vpsc->splitBlocks();
}
void setVariableDesiredPos(Variable *v, double desiredPos) {
	v->desiredPosition = desiredPos;
}
double getVariablePos(Variable *v) {
	return v->position();
}
}
