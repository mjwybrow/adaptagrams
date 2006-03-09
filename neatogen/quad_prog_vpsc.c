/**********************************************************
*      This software is part of the graphviz package      *
*                http://www.graphviz.org/                 *
*                                                         *
*            Copyright (c) 1994-2004 AT&T Corp.           *
*                and is licensed under the                *
*            Common Public License, Version 1.0           *
*                      by AT&T Corp.                      *
*                                                         *
*        Information and Software Systems Research        *
*              AT&T Research, Florham Park NJ             *
**********************************************************/

#include "digcola.h"
#ifdef DIGCOLA
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <float.h>
#include <assert.h>
#include "matrix_ops.h"
#include "kkutils.h"
#include <csolve_VPSC.h>
#include "quad_prog_vpsc.h"
#include "quad_prog_solver.h"

#define CONMAJ_LOGGING 1
#define quad_prog_tol 1e-2

int 
constrained_majorization_vpsc(CMajEnvVPSC *e, float * b, float **coords, 
                             int cur_axis, int dims, int max_iterations)
{
	int i,j,counter;
	if(max_iterations==0) return 0;

	bool converged=false;
#ifdef CONMAJ_LOGGING
	static int call_no=0;
#endif
	float *place = coords[cur_axis];
	float *g = e->fArray1;
	float *old_place = e->fArray2;
	float *d = e->fArray4;
    fprintf(stderr,"Entered: constrained_majorization_vpsc\n");
	for (i=0;i<e->n;i++) {
		setVariableDesiredPos(e->vs[i],place[i]);
	}
    fprintf(stderr,"  calling satisfyVPSC...\n");
	satisfyVPSC(e->vpsc);	
    fprintf(stderr,"    done.\n");
    /*
	for (i=0;i<e->n;i++) {
		place[i]=getVariablePos(e->vs[i]);
        fprintf(stderr,"vs[%d]=%f\n",i,place[i]);
	}
    */
#ifdef CONMAJ_LOGGING
	double prev_stress=0;
	for (i=0; i<e->n; i++) {
		prev_stress += 2*b[i]*place[i];
		for (j=0; j<e->n; j++) {
			prev_stress -= e->A[i][j]*place[j]*place[i];
		}
	}
	FILE *logfile = fopen("constrained_majorization_log","a");

	fprintf(logfile,"grad proj %d: stress=%f\n",call_no,prev_stress);
#endif
	
	for (counter=0; counter<max_iterations&&!converged; counter++) {
        fprintf(stderr,".");
		converged=true;		
		// find steepest descent direction
		for (i=0; i<e->n; i++) {
			old_place[i]=place[i];
			g[i] = 2*b[i];
			for (j=0; j<e->n; j++) {
				g[i] -= 2*e->A[i][j]*place[j];
			}
		}		
		float numerator = 0, denominator = 0, r;
		for (i=0; i<e->n; i++) {
			numerator += g[i]*g[i];
			r=0;
			for (j=0; j<e->n; j++) {
				r += 2*e->A[i][j]*g[j];
			}
			denominator -= r*g[i];
		}
		float alpha = numerator/denominator;
		for (i=0; i<e->n; i++) {
			place[i]-=alpha*g[i];
		}
		//project to constraint boundary
		for (i=0;i<e->n;i++) {
			setVariableDesiredPos(e->vs[i],place[i]);
		}
		//splitIncVPSC(e->vpsc);
		satisfyVPSC(e->vpsc);
		for (i=0;i<e->n;i++) {
			place[i]=getVariablePos(e->vs[i]);
		}
		// set place to the intersection of old_place-g and boundary and compute d, the vector from intersection pnt to projection pnt
		for (i=0; i<e->n; i++) {
			d[i]=place[i]-old_place[i];
		}	
		// now compute beta
		numerator = 0, denominator = 0;
		for (i=0; i<e->n; i++) {
			numerator += g[i]*d[i];
			r=0;
			for (j=0; j<e->n; j++) {
				r += 2*e->A[i][j]*d[j];
			}
			denominator += r*d[i];
		}
		float beta = numerator/denominator;

		float test=0;
		for (i=0; i<e->n; i++) {
			// beta > 1.0 takes us back outside the feasible region
			// beta < 0 clearly not useful and may happen due to numerical imp.
			if(beta>0&&beta<1.0) {
				place[i]=old_place[i]+beta*d[i];
			}
			//setVariableDesiredPos(e->vs[i],place[i]);
			test+= fabs(place[i]-old_place[i]);
		}
		//splitIncVPSC(e->vpsc);
#ifdef CONMAJ_LOGGING
		double stress=0;
		for (i=0; i<e->n; i++) {
			stress += 2*b[i]*place[i];
			for (j=0; j<e->n; j++) {
				stress -= e->A[i][j]*place[j]*place[i];
			}
		}
		fprintf(logfile,"%d: stress=%f, test=%f, %s\n",call_no,stress,test,(stress>=prev_stress)?"No Improvement":"");
		prev_stress=stress;
#endif
		if(test>quad_prog_tol) {
			converged=false;
		}
	}
#ifdef CONMAJ_LOGGING
	call_no++;
	fclose(logfile);
#endif
	return counter;
}

void
deleteCMajEnvVPSC(CMajEnvVPSC *e) 
{
    int i;
	free (e->A[0]); 
    free (e->A);
    deleteVPSC(e->vpsc);
    for(i=0;i<e->m;i++) {
        deleteConstraint(e->cs[i]);
    }
    free (e->cs);
    for(i=0;i<e->n;i++) {
        deleteVariable(e->vs[i]);
    }
    free (e->vs);
	free (e->fArray1);
	free (e->fArray2);
	free (e->fArray3);
	free (e->fArray4);
	free (e->iArray1);
	free (e->iArray2);
	free (e->iArray3);
	free (e->iArray4);
	free (e);
}

CMajEnvVPSC*
initCMajVPSC(float* packedMat, Variable** vs, int n, Constraint** cs, int m)
{
    int i,j;
    fprintf(stderr,"Entered initCMajVPSC\n");
	CMajEnvVPSC *e = GNEW(CMajEnvVPSC);
	e->A=NULL;
	e->n=n;
	e->cs=cs;
    e->m=m;
    e->vs=vs;

    e->vpsc = newIncVPSC(vs,n,cs,m);
    
	e->A = unpackMatrix(packedMat,n);

    /*
    fprintf(stderr,"e->A\n");
    for(i=0;i<e->n;i++) {
        for(j=0;j<e->n;j++) {
            fprintf(stderr,"%f ",e->A[i][j]);
        }
        fprintf(stderr,"\n");
    }
    */
	e->fArray1 = N_GNEW(n,float);
	e->fArray2 = N_GNEW(n,float);
	e->fArray3 = N_GNEW(n,float);
	e->fArray4 = N_GNEW(n,float);
	e->iArray1 = N_GNEW(n,int);
	e->iArray2 = N_GNEW(n,int);
	e->iArray3 = N_GNEW(n,int);
	e->iArray4 = N_GNEW(n,int);
    fprintf(stderr,"  done.\n");
	return e;
}
#endif /* DIGCOLA */

// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=4:softtabstop=4 :
