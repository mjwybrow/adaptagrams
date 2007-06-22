/* $Id: mosek_quad_solve.h,v 1.3 2006/04/28 20:33:44 ellson Exp $ $Revision: 1.3 $ */
/* vim:set shiftwidth=4 ts=8: */

/**
 *
 * Authors:
 *   Tim Dwyer <tgdwyer@gmail.com>
 *
 * Copyright (C) 2005 Authors
 *
 * This version is released under the LGPL as part of the Adaptagrams
 * project: http://sourceforge.net/projects/adaptagrams.  
 * If you make improvements or bug fixes to this code it would be much
 * appreciated if you could also contribute those changes back to the
 * Adaptagrams repository.
 */

#ifdef MOSEK_AVAILABLE
#ifndef _QSOLVE_H_
#define _QSOLVE_H_

#include <mosek.h> /* Include the MOSEK definition file. */
#include <libvpsc/csolve_VPSC.h>

#ifdef __cplusplus
extern "C" {
#endif
typedef struct {
	int r;
	MSKenv_t  env;
	MSKtask_t task;
	double *qval;
	int *qsubi,*qsubj;
	double *xx;
	int num_variables;
	int fixVar0;
} MosekEnv;

MosekEnv *mosek_init_sep_ls(int num_variables, 
			 Constraint ** cs, int num_constraints);
MosekEnv* mosek_init_sep(float* lap, int nv, Constraint** cs, 
	int m, int fixVar0);
void mosek_quad_solve_sep(MosekEnv*,float *b,float* coords);
void mosek_delete(MosekEnv*);
#ifdef __cplusplus
}
#endif


#endif /* _QSOLVE_H_ */
#endif /* MOSEK_AVAILABLE */
