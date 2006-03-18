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
VPSC* newVPSC(int n, Variable* vs[], int m, Constraint* cs[]);
void deleteVPSC(VPSC*);
void deleteConstraint(Constraint*);
void deleteVariable(Variable*);
Constraint** newConstraints(int m);
void deleteConstraints(int m,Constraint**);
void remapInConstraints(Variable *u, Variable *v, double dgap);
void remapOutConstraints(Variable *u, Variable *v, double dgap);

int genXConstraints(int n, double minX[], double maxX[], double minY[], double maxY[], Variable** vs, Constraint*** cs);
int genYConstraints(int n, double minX[], double maxX[], double minY[], double maxY[], Variable** vs, Constraint*** cs);

void satisfyVPSC(VPSC*);
void solveVPSC(VPSC*);
typedef struct IncVPSC IncVPSC;
void splitIncVPSC(IncVPSC*);
#ifdef __cplusplus
}
#endif
#endif /* _CSOLVE_VPSC_H_ */
