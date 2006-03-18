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

#include "defs.h"

typedef struct {
	float **A;
	int n; /* number of vars */
	Variable **vs;
	int m; /* total number of constraints for next iteration */
	int gm; /* number of global constraints */
	Constraint **cs;
	/* global constraints are persistant throughout optimisation process */
	Constraint **gcs;
	VPSC *vpsc;
	float *fArray1; /* utility arrays - reusable memory */
	float *fArray2;
	float *fArray3;
} CMajEnvVPSC;

extern CMajEnvVPSC* initCMajVPSC(int n, float *, vtx_data*, int, float, cluster_data*);

extern int constrained_majorization_vpsc(CMajEnvVPSC*, float*, float*, int);

extern void deleteCMajEnvVPSC(CMajEnvVPSC *e);
extern void generateNonoverlapConstraints(
        CMajEnvVPSC* e,
	pointf* nsize,
	pointf gap,
        float nsizeScale,
        float** coords,
        int k,
	cluster_data* clusters
);

extern void removeoverlaps(int,float**,pointf*,pointf,cluster_data*);

#endif 

#endif /* _QUAD_PROG_VPSC_H_ */

#ifdef __cplusplus
}
#endif
