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
#include "stress.h"
#include "dijkstra.h"
#include "bfs.h"
#include "matrix_ops.h"
#include "kkutils.h"
#include "conjgrad.h"
#include <csolve_VPSC.h>
#include "quad_prog_vpsc.h"
#include "quad_prog_solver.h"
#include "matrix_ops.h"

#define localConstrMajorIterations 100

int 
stress_majorization_vsep(
    vtx_data* graph,    /* Input graph in sparse representation	 */
    int n,              /* Number of nodes */
    int nedges_graph,   /* Number of edges */
    double** d_coords,  /* Coordinates of nodes (output layout)  */
    int dim,            /* Dimemsionality of layout */
    int smart_ini,      /* smart initialization */
    int model,          /* difference model */
    int maxi,           /* max iterations */
    int diredges,       /* 1=generate directed edge constraints */
    double edge_gap,    /* amount to force vertical separation of */
                        /* start/end nodes */
    int noverlap,       /* 1=generate non-overlap constraints */
                        /* 2=remove overlaps after layout */
    double xgap,        /* horizontal gap to enforce when removing overlap*/
    double ygap,        /* vertical    "       "      "      "       "    */
    double* nwidth,     /* node widths */
    double* nheight     /* node heights */
)
{
    int iterations = 0;    /* Output: number of iteration of the process */

	/*************************************************
	** Computation of full, dense, unrestricted k-D ** 
	** stress minimization by majorization          ** 
	** This function imposes HIERARCHY CONSTRAINTS  **
	*************************************************/

	int i,j,k;
	bool directionalityExist = FALSE;
	float * lap1 = NULL;
	float * dist_accumulator = NULL;
	float * tmp_coords = NULL;
	float ** b = NULL;
#ifdef NONCORE
	FILE * fp=NULL;
#endif
	double * degrees = NULL;
	float * lap2=NULL;
	int lap_length;
	float * f_storage=NULL;
	float ** coords=NULL;

	//double conj_tol=tolerance_cg;        /* tolerance of Conjugate Gradient */
	CMajEnvVPSC *cMajEnv = NULL;
	clock_t start_time;
	double y_0;
	int length;
	DistType diameter;
	float * Dij=NULL;
	float constant_term;
	int count;
	double degree;
	int step;
	float val;
	double old_stress, new_stress;
	bool converged;
	int len;
    double nsizeScale=0;
    Constraint** cs=NULL;
    Variable** vs = N_GNEW(n, Variable*);
    int m;
    fprintf(stderr,"Entered: stress_majorization_diredges\n");

	if (graph[0].edists!=NULL) {
		for (i=0; i<n; i++) {
			for (j=1; j<graph[i].nedges; j++) {
				 directionalityExist = directionalityExist || (graph[i].edists[j]!=0);
			}
		}
	}
	if (!directionalityExist) {
		return stress_majorization_kD_mkernel(graph, n, nedges_graph, d_coords, dim, smart_ini, model, maxi);
	}

	if (0/*smart_ini*/) {
		double* x;
		double* y;
		if (dim>2) {
			/* the dim==2 case is handled below			 */
			stress_majorization_kD_mkernel(graph, n, nedges_graph, d_coords+1, dim-1, smart_ini, model, 15);
			/* now copy the y-axis into the (dim-1)-axis */
			for (i=0; i<n; i++) {
				d_coords[dim-1][i] = d_coords[1][i];
			}
		}

		x = d_coords[0]; y = d_coords[1];

		if (dim==2) {
			IMDS_given_dim(graph, n, y, x, Epsilon);
		}
	}
	else {
        initLayout(graph, n, dim, d_coords);
	}
    if (n == 1) return 0;

	/******************************************************************************
	** First, generate separation constraints for edges in largest acyclic subgraph
	*******************************************************************************/

    m=0;
    if(diredges) {
        fprintf(stderr,"  generate edge constraints...\n");
        for(i=0;i<n;i++) {
            vs[i]=newVariable(i,1.0,1.0);
            for(j=1;j<graph[i].nedges;j++) {
                if(graph[i].edists[j]>0) {
                    m++;
                }
            }
        }
        cs=N_GNEW(m, Constraint*);
        m=0;
        for(i=0;i<n;i++) {
            for(j=1;j<graph[i].nedges;j++) {
                int u=i,v=graph[i].edges[j];
                if(graph[i].edists[j]>0) {
                    cs[m]=newConstraint(vs[u],vs[v],edge_gap);
                    m++;
                }
            }
        }
        fprintf(stderr,"  generate constraints... done: n=%d,m=%d\n",n,m);
    }


	/****************************************************
	** Compute the all-pairs-shortest-distances matrix **
	****************************************************/

	if (maxi==0)
		return iterations;

    if (model == MODEL_SUBSET) {
        /* weight graph to separate high-degree nodes */
        /* and perform slower Dijkstra-based computation */
        if (Verbose)
            fprintf(stderr, "Calculating subset model");
        Dij = compute_apsp_artifical_weights_packed(graph, n);
    } else if (model == MODEL_CIRCUIT) {
        Dij = circuitModel(graph, n);
        if (!Dij) {
            agerr(AGWARN,
                  "graph is disconnected. Hence, the circuit model\n");
            agerr(AGPREV,
                  "is undefined. Reverting to the shortest path model.\n");
        }
    }
    if (!Dij) {
        if (Verbose)
            fprintf(stderr, "Calculating shortest paths ");
        Dij = compute_apsp_packed(graph, n);
    }

	diameter=-1;
	length = n+n*(n-1)/2;
	for (i=0; i<length; i++) {
		if (Dij[i]>diameter) {
			diameter = (int)Dij[i];
		}
	}

	if (!smart_ini) {
		/* for numerical stability, scale down layout		 */
		/* No Jiggling, might conflict with constraints			 */
        fprintf(stderr,"SCALING!!!\n");
		double max=1;		
		for (i=0; i<dim; i++) {	
			for (j=0; j<n; j++) {
				if (fabs(d_coords[i][j])>max) {
					max=fabs(d_coords[i][j]);
				}
			}	
		}
		for (i=0; i<dim; i++) {	
			for (j=0; j<n; j++) {
				d_coords[i][j]*=10/max;
			}	
		}
	}		

	if (edge_gap>50) {
		for (i=0; i<length; i++) {
            fprintf(stderr,"Dij[%d]=%f ",i,Dij[i]);
		}
		int length = n+n*(n-1)/2;
		double sum1, sum2, scale_ratio;
		int count;
		sum1=(float)(n*(n-1)/2);
		sum2=0;
		for (count=0, i=0; i<n-1; i++) {
			count++; // skip self distance
			for (j=i+1; j<n; j++,count++) {
				sum2+=distance_kD(d_coords, dim, i, j)/Dij[count];
			}
		}
		scale_ratio=sum2/sum1;
		/* double scale_ratio=10; */
		for (i=0; i<length; i++) {
			Dij[i]*=(float)scale_ratio;
            fprintf(stderr,"Dij[%d]=%f ",i,Dij[i]);
		}
	}

	/**************************
	** Layout initialization **
	**************************/

	for (i=0; i<dim; i++) {		
		orthog1(n, d_coords[i]);
	}

	/* for the y-coords, don't center them, but translate them so y[0]=0 */
	y_0 = d_coords[1][0];
	for (i=0; i<n; i++) {
		d_coords[1][i] -= y_0;
	}

	coords = N_GNEW(dim, float*);
	f_storage = N_GNEW(dim*n, float);
	for (i=0; i<dim; i++) {
		coords[i] = f_storage+i*n;
		for (j=0; j<n; j++) {
			coords[i][j] = (float)(d_coords[i][j]);
		}
	}

	/* compute constant term in stress sum
	 * which is \sum_{i<j} w_{ij}d_{ij}^2
     */
	constant_term=(float)(n*(n-1)/2);
	
	/**************************
	** Laplacian computation **
	**************************/
			
	lap2 = Dij;
	lap_length = n+n*(n-1)/2;
	square_vec(lap_length, lap2);
	/* compute off-diagonal entries */
	invert_vec(lap_length, lap2);
	
	/* compute diagonal entries */
	count=0;
	degrees = N_GNEW(n, double);
	set_vector_val(n, 0, degrees);
	for (i=0; i<n-1; i++) {
		degree=0;
		count++; // skip main diag entry
		for (j=1; j<n-i; j++,count++) {
			val = lap2[count];
			degree+=val; degrees[i+j]-=val;
		}
		degrees[i]-=degree;
	}
	for (step=n,count=0,i=0; i<n; i++,count+=step,step--) {
		lap2[count]=(float)degrees[i];
	}

#ifdef NONCORE
	fpos_t pos;
	if (n>max_nodes_in_mem) {
		#define FILENAME "tmp_Dij$$$.bin"
		fp = fopen(FILENAME, "wb");
		fwrite(lap2, sizeof(float), lap_length, fp);
		fclose(fp);
	}
#endif
		
	/*************************
	** Layout optimization  **
	*************************/
	
	b = N_GNEW (dim, float*);
	b[0] = N_GNEW (dim*n, float);
	for (k=1; k<dim; k++) {
		b[k] = b[0]+k*n;
	}

	tmp_coords = N_GNEW(n, float);
	dist_accumulator = N_GNEW(n, float);
#ifdef NONCORE
	if (n<=max_nodes_in_mem) {
#endif
		lap1 = N_GNEW(lap_length, float);
#ifdef NONCORE
	}
	else {
		lap1=lap2;
		fp = fopen(FILENAME, "rb");
		fgetpos(fp, &pos);
	}
#endif
	
	old_stress=DBL_MAX; /* at least one iteration */

	start_time = clock();

	cMajEnv=initCMajVPSC(n,lap2, vs, m, cs);

    fprintf(stderr,"Entering main loop...\n");
	for (converged=false,iterations=0; iterations<maxi && !converged; iterations++) {

		/* First, construct Laplacian of 1/(d_ij*|p_i-p_j|)  */
		set_vector_val(n, 0, degrees);
#ifdef NONCORE
		if (n<=max_nodes_in_mem) {
#endif
			sqrt_vecf(lap_length, lap2, lap1);
#ifdef NONCORE
		}
		else {
			sqrt_vec(lap_length, lap1);
		}
#endif
		for (count=0,i=0; i<n-1; i++) {
			len = n-i-1;
			/* init 'dist_accumulator' with zeros */
			set_vector_valf(n, 0, dist_accumulator);
			
			/* put into 'dist_accumulator' all squared distances 
             * between 'i' and 'i'+1,...,'n'-1
             */
			for (k=0; k<dim; k++) {	
				set_vector_valf(len, coords[k][i], tmp_coords);
				vectors_mult_additionf(len, tmp_coords, -1, coords[k]+i+1);
				square_vec(len, tmp_coords);
				vectors_additionf(len, tmp_coords, dist_accumulator, dist_accumulator);
			}			
		
			/* convert to 1/d_{ij} */
			invert_sqrt_vec(len, dist_accumulator);
			/* detect overflows */
			for (j=0; j<len; j++) {
				if (dist_accumulator[j]>=FLT_MAX || dist_accumulator[j]<0) {
					dist_accumulator[j]=0;
				}
			}
			
			count++; /* save place for the main diagonal entry */
			degree=0;
			for (j=0; j<len; j++,count++) {
				val=lap1[count]*=dist_accumulator[j];
				degree+=val; degrees[i+j+1]-=val;
			}
			degrees[i]-=degree;			
		}
		for (step=n,count=0,i=0; i<n; i++,count+=step,step--) {
			lap1[count]=(float)degrees[i];
		}

		/* Now compute b[] (L^(X(t))*X(t)) */
		for (k=0; k<dim; k++) {	
			/* b[k] := lap1*coords[k] */
			right_mult_with_vector_ff(lap1, n, coords[k], b[k]);
		}
		
		/* compute new stress
		 * remember that the Laplacians are negated, so we subtract 
         * instead of add and vice versa
         */
		new_stress=0;
		for (k=0; k<dim; k++) {	
			new_stress+=vectors_inner_productf(n, coords[k], b[k]);
		}
		new_stress*=2;
		new_stress+=constant_term; // only after mult by 2		
#ifdef NONCORE
		if (n>max_nodes_in_mem) {
			/* restore lap2 from disk */
			fsetpos(fp, &pos);
			fread(lap2, sizeof(float), lap_length, fp);
		}
#endif
		for (k=0; k<dim; k++) {	
			right_mult_with_vector_ff(lap2, n, coords[k], tmp_coords);
			new_stress-=vectors_inner_productf(n, coords[k], tmp_coords);
		}

#ifdef ALTERNATIVE_STRESS_CALC
		{
		double mat_stress=new_stress;
		double compute_stress(float ** coords, float * lap, int dim, int n);
		new_stress = compute_stress(coords, lap2, dim, n);
		if (fabs(mat_stress-new_stress)/min(mat_stress,new_stress)>0.001) {
			fprintf(stderr,"Diff stress vals: %lf %lf (iteration #%d)\n", mat_stress, new_stress,iterations);
		}
		}
#endif
		/* check for convergence */
        fprintf(stderr,"stress=%f\n",new_stress);
		converged = fabs(new_stress-old_stress)/fabs(old_stress+1e-10) < Epsilon;
		converged = converged || (iterations>1 && new_stress>old_stress); 
			/* in first iteration we allowed stress increase, which 
             * might result ny imposing constraints
             */
		old_stress = new_stress;

        // in determining non-overlap constraints we gradually scale up the
        // size of nodes to avoid local minima
        if(converged&&noverlap==1&&nsizeScale<0.99) {
            nsizeScale= 1.0;
            converged = false;
        }
		
		for (k=0; k<dim; k++) {
			/* now we find the optimizer of trace(X'LX)+X'B by solving 'dim' 
             * system of equations, thereby obtaining the new coordinates.
			 * If we use the constraints (given by the var's: 'ordering', 
             * 'levels' and 'num_levels'), we cannot optimize 
             * trace(X'LX)+X'B by simply solving equations, but we have
			 * to use a quadratic programming solver
			 * note: 'lap2' is a packed symmetric matrix, that is its 
             * upper-triangular part is arranged in a vector row-wise
			 * also note: 'lap2' is really the negated laplacian (the 
             * laplacian is -'lap2')
             */
			
            if(noverlap==1 && nsizeScale > 0.001) {
                generateNonoverlapConstraints(cMajEnv,nwidth,nheight,nsizeScale,coords,k);
                constrained_majorization_vpsc(cMajEnv, b[k], coords, k, dim, localConstrMajorIterations);
                cleanupNonoverlapConstraints(cMajEnv,k);
            } else {
                cMajEnv->m=cMajEnv->em;
                cMajEnv->cs=cMajEnv->edge_cs;
                constrained_majorization_vpsc(cMajEnv, b[k], coords, k, dim, localConstrMajorIterations);
            }
		}
	}
    fprintf(stderr,"Finished majorization!\n");
	//deleteCMajEnvVPSC(cMajEnv);
    fprintf(stderr,"  freed cMajEnv!\n");
	
	if (coords!=NULL) {
		for (i=0; i<dim; i++) {
			for (j=0; j<n; j++) {
				d_coords[i][j] = coords[i][j];
			}
		}
		free (coords[0]); free (coords);
	}
	
	if (b) {	
		free (b[0]); free (b);
	}
	free (tmp_coords);
	free (dist_accumulator);
	free (degrees);
	free (lap2);
	

#ifdef NONCORE
	if (n<=max_nodes_in_mem) {
#endif
		free (lap1); 
#ifdef NONCORE
	}
#endif

    return iterations;
}
#endif /* DIGCOLA */

// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=4:softtabstop=4 :
