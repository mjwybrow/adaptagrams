#ifndef _CSOLVE_VPSC_H_
#define _CSOLVE_VPSC_H_
#ifdef __cplusplus
extern "C" {
#endif
typedef struct Variable Variable;
Variable* newVariable(int id, double desiredPos, double weight);
void setVariableDesiredPos(Variable *, double desiredPos);
double getVariablePos(Variable*);

typedef struct Constraint Constraint;
Constraint* newConstraint(Variable* left, Variable* right, double gap);

typedef struct VPSC VPSC;
VPSC* newVPSC(Variable* vs[], int n, Constraint* cs[], int m);
void deleteVPSC(VPSC*);

void satisfyVPSC(VPSC*);
void solveVPSC(VPSC*);
typedef struct IncVPSC IncVPSC;
void splitIncVPSC(IncVPSC*);
#ifdef __cplusplus
}
#endif
#endif /* _CSOLVE_VPSC_H_ */
