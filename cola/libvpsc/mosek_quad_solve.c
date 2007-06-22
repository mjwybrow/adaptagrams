/* $Id: mosek_quad_solve.c,v 1.3 2006/04/28 20:33:44 ellson Exp $ $Revision: 1.3 $ */
/* vim:set shiftwidth=4 ts=8: */

/**
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

/* 
 * Interface to Mosek (www.mosek.com) quadratic programming solver for solving
 * instances of the "Variable Placement with Separation Constraints" problem,
 * and also DiG-CoLa style level constraints.
 *
 * Tim Dwyer, 2006
 */
#ifdef MOSEK_AVAILABLE

#include <stdio.h>
#include <assert.h>
#include "mosek_quad_solve.h"
#define GNEW(t) (t*)malloc(sizeof(t))
#define N_GNEW(n,t) (t*)malloc((n)*sizeof(t))

//#define DUMP_CONSTRAINTS

static FILE *logfile;
static void MSKAPI printstr(void *handle, char str[])
{
    fprintf(logfile, "%s", str);
}

/**********************
Initialise least squares problem subject to separation constraints
cs: array of pointers to separation constraints
***********************/
MosekEnv *mosek_init_sep_ls(int num_variables, 
			 Constraint ** cs, int num_constraints)
{
    int i, j;
    MosekEnv *mskEnv = GNEW(MosekEnv);
    mskEnv->num_variables = num_variables;
    mskEnv->fixVar0 = 0;

    //fprintf(stderr, "MOSEK!\n");
    logfile = fopen("quad_solve_log", "w");

    /* nonlinear coefficients matrix of objective function */
    mskEnv->qval = N_GNEW(num_variables, double);
    mskEnv->qsubi = N_GNEW(num_variables, int);
    mskEnv->qsubj = N_GNEW(num_variables, int);

    /* solution vector */
    mskEnv->xx = N_GNEW(mskEnv->num_variables, double);

    for (i = 0; i < mskEnv->num_variables; i++) {
	mskEnv->qval[i] = 2;
	/* assert(mskEnv->qval[count]!=0); */
	mskEnv->qsubi[i] = i;
	mskEnv->qsubj[i] = i;
    }
    /* Make the mosek environment. */
    mskEnv->r = MSK_makeenv(&mskEnv->env, NULL, NULL, NULL, NULL);

    /* Check whether the return code is ok. */
    if (mskEnv->r == MSK_RES_OK) {
	/* Directs the log stream to the user
	   specified procedure 'printstr'. */
	MSK_linkfunctoenvstream(mskEnv->env, MSK_STREAM_LOG, NULL,
				printstr);
    }

    /* Initialize the environment. */
    mskEnv->r = MSK_initenv(mskEnv->env);
    if (mskEnv->r == MSK_RES_OK) {
	/* Make the optimization task. */
	mskEnv->r =
	    MSK_maketask(mskEnv->env, num_constraints,
			 mskEnv->num_variables, &mskEnv->task);

	if (mskEnv->r == MSK_RES_OK) {
	    mskEnv->r =
		MSK_linkfunctotaskstream(mskEnv->task, MSK_STREAM_LOG,
					 NULL, printstr);
	    /* Resize the task. */
	    if (mskEnv->r == MSK_RES_OK)
		mskEnv->r = MSK_resizetask(mskEnv->task, num_constraints, mskEnv->num_variables, 0,	/* no cones!! */
					   /* number of non-zero constraint matrix entries:
					    *   each constraint applies to 2 vars
					    */
					   2 * num_constraints,
					   num_variables);

	    /* Append the constraints. */
	    if (mskEnv->r == MSK_RES_OK)
		mskEnv->r = MSK_append(mskEnv->task, 1, num_constraints);

	    /* Append the variables. */
	    if (mskEnv->r == MSK_RES_OK)
		mskEnv->r =
		    MSK_append(mskEnv->task, 0, mskEnv->num_variables);
	    /* Put variable bounds. */
	    for (j = 0;
		 j < mskEnv->num_variables && mskEnv->r == MSK_RES_OK; j++)
		mskEnv->r =
		    MSK_putbound(mskEnv->task, 0, j, MSK_BK_RA,
				 -MSK_INFINITY, MSK_INFINITY);
	    for (i = 0; i < num_constraints; i++) {
		int u = getLeftVarID(cs[i]);
		int v = getRightVarID(cs[i]);
		double separation = getSeparation(cs[i]);
		MSKidxt subi[2]; double vali[2];
		subi[0] = u; subi[1] = v;
		vali[0] = -getLeftScale(cs[i]); vali[1] = getRightScale(cs[i]);
		//printf("Constraint %f*x[%d]+%f<=%f*x[%d]\n",-1*vali[0],u,separation,vali[1],v);
		mskEnv->r =
		    MSK_putavec(mskEnv->task, 1, i, 2, subi, vali);
		assert(mskEnv->r == MSK_RES_OK);
		mskEnv->r =
		    MSK_putbound(mskEnv->task, 1, i, MSK_BK_LO,
				 separation, MSK_INFINITY);
		assert(mskEnv->r == MSK_RES_OK);
	    }
	    if (mskEnv->r == MSK_RES_OK) {
		/*
		 * The lower triangular part of the Q
		 * matrix in the objective is specified.
		 */
		mskEnv->r =
		    MSK_putqobj(mskEnv->task, num_variables,
				mskEnv->qsubi, mskEnv->qsubj,
				mskEnv->qval);
		assert(mskEnv->r == MSK_RES_OK);
	    }
	}
    }
    return mskEnv;
}
/**********************
lap: the upper RHS of the symmetric graph laplacian matrix which will be
     transformed to the hessian of the non-linear part of the optimisation
     function has dimensions num_variables, dummy vars do not have entries in
     lap
cs: array of pointers to separation constraints
fixVar0: if fixVar0!=0 then the first variable will be omitted from the
         problem and effectively fixed at 0
***********************/
MosekEnv *mosek_init_sep(float *lap, int num_variables, 
			 Constraint ** cs, int num_constraints,
			 int fixVar0)
{
    int i, j;
    MosekEnv *mskEnv = GNEW(MosekEnv);
    int count = 0, offset = 0;
    int nonzero_lapsize = num_variables * (num_variables + 1) / 2;
    mskEnv->num_variables = num_variables;
    mskEnv->fixVar0 = fixVar0;
    if(fixVar0) {
	offset = num_variables;
    	nonzero_lapsize-=num_variables;
	mskEnv->num_variables--;
    }

    //fprintf(stderr, "MOSEK!\n");
    logfile = fopen("quad_solve_log", "w");

    /* nonlinear coefficients matrix of objective function */
    mskEnv->qval = N_GNEW(nonzero_lapsize, double);
    mskEnv->qsubi = N_GNEW(nonzero_lapsize, int);
    mskEnv->qsubj = N_GNEW(nonzero_lapsize, int);

    /* solution vector */
    mskEnv->xx = N_GNEW(mskEnv->num_variables, double);

    /* pointer to beginning of nonzero sequence in a column */

    for (i = 0; i < mskEnv->num_variables; i++) {
	for (j = i; j < mskEnv->num_variables; j++) {
	    mskEnv->qval[count] = -2 * lap[count+offset];
	    /* assert(mskEnv->qval[count]!=0); */
	    mskEnv->qsubi[count] = j;
	    mskEnv->qsubj[count] = i;
	    count++;
	}
    }
#ifdef DUMP_CONSTRAINTS
    fprintf(logfile, "Q=[");
    count = offset;
    for (i = 0; i < mskEnv->num_variables; i++) {
	if (i != 0)
	    fprintf(logfile, ";");
	for (j = 0; j < mskEnv->num_variables; j++) {
	    if (j < i) {
		fprintf(logfile, "0 ");
	    } else {
		fprintf(logfile, "%f ", -2 * lap[count++]);
	    }
	}
    }
    fprintf(logfile, "]\nQ=Q-diag(diag(Q))+Q'\n");
#endif
    /* Make the mosek environment. */
    mskEnv->r = MSK_makeenv(&mskEnv->env, NULL, NULL, NULL, NULL);

    /* Check whether the return code is ok. */
    if (mskEnv->r == MSK_RES_OK) {
	/* Directs the log stream to the user
	   specified procedure 'printstr'. */
	MSK_linkfunctoenvstream(mskEnv->env, MSK_STREAM_LOG, NULL,
				printstr);
    }

    /* Initialize the environment. */
    mskEnv->r = MSK_initenv(mskEnv->env);
    if (mskEnv->r == MSK_RES_OK) {
	/* Make the optimization task. */
	mskEnv->r =
	    MSK_maketask(mskEnv->env, num_constraints,
			 mskEnv->num_variables, &mskEnv->task);

	if (mskEnv->r == MSK_RES_OK) {
	    mskEnv->r =
		MSK_linkfunctotaskstream(mskEnv->task, MSK_STREAM_LOG,
					 NULL, printstr);
	    /* Resize the task. */
	    if (mskEnv->r == MSK_RES_OK)
		mskEnv->r = MSK_resizetask(mskEnv->task, num_constraints, mskEnv->num_variables, 0,	/* no cones!! */
					   /* number of non-zero constraint matrix entries:
					    *   each constraint applies to 2 vars
					    */
					   2 * num_constraints,
					   nonzero_lapsize);

	    /* Append the constraints. */
	    if (mskEnv->r == MSK_RES_OK)
		mskEnv->r = MSK_append(mskEnv->task, 1, num_constraints);

	    /* Append the variables. */
	    if (mskEnv->r == MSK_RES_OK)
		mskEnv->r =
		    MSK_append(mskEnv->task, 0, mskEnv->num_variables);
	    /* Put variable bounds. */
	    for (j = 0;
		 j < mskEnv->num_variables && mskEnv->r == MSK_RES_OK; j++)
		mskEnv->r =
		    MSK_putbound(mskEnv->task, MSK_ACC_VAR, j, MSK_BK_FR,
				 -MSK_INFINITY, MSK_INFINITY);
	    for (i = 0; i < num_constraints; i++) {
		int u = getLeftVarID(cs[i]);
		int v = getRightVarID(cs[i]);
		if(fixVar0) { u--; v--; }
		double separation = getSeparation(cs[i]);
		//printf("Constraint x[%d]+%f<=x[%d]\n",u,separation,v);
		if (u < 0) {
		    mskEnv->r =
			MSK_putbound(mskEnv->task, MSK_ACC_VAR, v, MSK_BK_LO,
				     separation, MSK_INFINITY);
		    assert(mskEnv->r == MSK_RES_OK);
		} else if (v < 0) {
		    mskEnv->r =
			MSK_putbound(mskEnv->task, MSK_ACC_VAR, u, MSK_BK_UP,
				     -MSK_INFINITY, -separation);
		    assert(mskEnv->r == MSK_RES_OK);
		} else {
		    /* fprintf(stderr,"u=%d,v=%d,sep=%f\n",u,v,separation); */
		    MSKidxt subi[2]; double vali[2];
		    subi[0] = u; subi[1] = v;
		    vali[0] = -getLeftScale(cs[i]); vali[1] = getRightScale(cs[i]);
		    mskEnv->r =
			MSK_putavec(mskEnv->task, MSK_ACC_CON, i, 2, subi, vali);
		    assert(mskEnv->r == MSK_RES_OK);
		    mskEnv->r =
			MSK_putbound(mskEnv->task, MSK_ACC_CON, i, MSK_BK_LO,
				     separation, MSK_INFINITY);
		    assert(mskEnv->r == MSK_RES_OK);
		}
	    }
	    if (mskEnv->r == MSK_RES_OK) {
		/*
		 * The lower triangular part of the Q
		 * matrix in the objective is specified.
		 */
		mskEnv->r =
		    MSK_putqobj(mskEnv->task, nonzero_lapsize,
				mskEnv->qsubi, mskEnv->qsubj,
				mskEnv->qval);
		assert(mskEnv->r == MSK_RES_OK);
	    }
	}
    }
    return mskEnv;
}

/*
b: coefficients of linear part of optimisation function
coords: optimal y* vector, coord[i] is coordinate of node[i]
*/
void mosek_quad_solve_sep(MosekEnv * mskEnv, float *b,
			  float *coords)
{
    int i, j, n=mskEnv->num_variables, offset=mskEnv->fixVar0?1:0;
    for (i = 0; i < n && mskEnv->r == MSK_RES_OK; i++) {
	mskEnv->r = MSK_putcj(mskEnv->task, i, -2 * b[i+offset]);
    }
    if (mskEnv->r == MSK_RES_OK)
	mskEnv->r = MSK_optimize(mskEnv->task);

    if (mskEnv->r == MSK_RES_OK) {
	MSK_getsolutionslice(mskEnv->task,
			     MSK_SOL_ITR,
			     MSK_SOL_ITEM_XX,
			     0, mskEnv->num_variables, mskEnv->xx);

#ifdef DUMP_CONSTRAINTS
	fprintf(logfile, "Primal solution\n");
#endif
	coords[0] = 0;
	for (j = offset; j < n+offset; j++) {
#ifdef DUMP_CONSTRAINTS
	    fprintf(logfile, "x[%d]: %.2f\n", j, mskEnv->xx[j-offset]);
#endif
	    coords[j] = mskEnv->xx[j-offset];
	}
    }
    fprintf(logfile, "Return code: %d\n", mskEnv->r);
}

/*
please call to clean up
*/
void mosek_delete(MosekEnv * mskEnv)
{
    MSK_deletetask(&mskEnv->task);
    MSK_deleteenv(&mskEnv->env);

    if (logfile) {
	fclose(logfile);
	logfile = NULL;
    }
    free(mskEnv->qval);
    free(mskEnv->qsubi);
    free(mskEnv->qsubj);
    free(mskEnv->xx);
    free(mskEnv);
}
#endif //MOSEK_AVAILABLE
