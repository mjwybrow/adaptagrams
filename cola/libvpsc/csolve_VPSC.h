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
 * \brief Bridge for C programs to access solve_VPSC (which is in C++)
 *
 * Authors:
 *   Tim Dwyer <tgdwyer@gmail.com>
 *
 */
#ifndef _CSOLVE_VPSC_H_
#define _CSOLVE_VPSC_H_
#ifdef __cplusplus
class vpsc::Variable;
class vpsc::Constraint;
class vpsc::Solver;
class vpsc::IncSolver;
using namespace vpsc;
extern "C" {
#else
typedef struct Variable Variable;
typedef struct Constraint Constraint;
typedef struct Solver Solver;
typedef struct IncSolver IncSolver;
#endif
Variable* newVariable(int id, double desiredPos, double weight);
void setVariableDesiredPos(Variable *, double desiredPos);
double getVariablePos(Variable*);

Constraint* newConstraint(Variable* left, Variable* right, double gap);

Solver* newSolver(int n, Variable* vs[], int m, Constraint* cs[]);
void deleteSolver(Solver*);
void deleteConstraint(Constraint*);
void deleteVariable(Variable*);
Constraint** newConstraints(int m);
void deleteConstraints(int m,Constraint**);
void remapInConstraints(Variable *u, Variable *v, double dgap);
void remapOutConstraints(Variable *u, Variable *v, double dgap);
int getLeftVarID(Constraint *c);
int getRightVarID(Constraint *c);
double getSeparation(Constraint *c);
double getLeftScale(Constraint *c);
double getRightScale(Constraint *c);

#ifndef HAVE_POINTF_S
typedef struct pointf_s { double x, y; } pointf;
typedef struct { pointf LL, UR; } boxf;
#endif
int genXConstraints(int n, boxf[], Variable** vs, Constraint*** cs,
		int transitiveClosure);
int genYConstraints(int n, boxf[], Variable** vs, Constraint*** cs);

void satisfyVPSC(Solver*);
void solveVPSC(Solver*);
Solver* newIncSolver(int n, Variable* vs[], int m, Constraint* cs[]);
void splitIncSolver(IncSolver*);
int getSplitCnt(IncSolver *vpsc);
#ifdef __cplusplus
}
#endif
#endif /* _CSOLVE_VPSC_H_ */
