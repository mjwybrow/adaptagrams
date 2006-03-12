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

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _QUAD_PROG_VPSC_H_
#define _QUAD_PROG_VPSC_H_

#ifdef DIGCOLA

typedef struct {
	float **A;
	int n; /* number of vars */
	Variable **vs;
	int m; /* number of constraints */
	int em; /* number of diredge constraints in edge_cs */
	Constraint **cs;
	/* dir edge constraints are persistant throughout optimisation process */
	Constraint **edge_cs;
	VPSC *vpsc;
	int *iArray1; /* utility arrays */
	int *iArray2;
	int *iArray3;
	int *iArray4;
	float *fArray1;
	float *fArray2;
	float *fArray3;
	float *fArray4;
} CMajEnvVPSC;

extern CMajEnvVPSC* initCMajVPSC(int n, float *, Variable** vs, int m, Constraint** cs);

extern int constrained_majorization_vpsc(CMajEnvVPSC*, float*, float**, 
                                        int, int, int);

extern void deleteCMajEnvVPSC(CMajEnvVPSC *e);
extern void generateNonoverlapConstraints(
        CMajEnvVPSC* e,
        double* nwidth,
        double* nheight,
        double nsizeScale,
        float** coords,
        int k
);
extern void cleanupNonoverlapConstraints(CMajEnvVPSC *e,int k);

#endif 

#endif /* _QUAD_PROG_VPSC_H_ */

#ifdef __cplusplus
}
#endif
