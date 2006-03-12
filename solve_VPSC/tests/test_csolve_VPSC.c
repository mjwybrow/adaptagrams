#include <stdio.h>
#include <malloc.h>
#include "csolve_VPSC.h"

int main(int argc, char **argv) {
	Variable* v1 = newVariable(1, 1.0, 1.0);
	Variable* v2 = newVariable(1, 2.0, 1.0);
	Constraint* c = newConstraint(v1,v2,2.0);
	int n=2, m=1, i;
	Variable** vs = (Variable**)malloc(n*sizeof(Variable*));
	Constraint** cs = (Constraint**)malloc(m*sizeof(Constraint*));
	vs[0]=v1;
	vs[1]=v2;
	cs[0]=c;
	VPSC* vpsc = newIncVPSC(n,vs,m,cs);
	satisfyVPSC(vpsc);
	splitIncVPSC(vpsc);
	printf("v=%f,%f\n",getVariablePos(v1),getVariablePos(v2));
	for(i=0;i<n;i++) {
		free(vs[i]);
	}
	free(vs);
	for(i=0;i<m;i++) {
		free(cs[i]);
	}
	free(cs);
	deleteVPSC(vpsc);
	return 0;
}
