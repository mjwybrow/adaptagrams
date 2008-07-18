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
 *
 * Authors:
 *   Tim Dwyer <tgdwyer@gmail.com>
 *
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
