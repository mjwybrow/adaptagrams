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
constrained_majorization_vpsc(CMajEnvVPSC *e, float * b, float *place, 
                             int max_iterations)
{
	int i,j,counter;
	if(max_iterations==0) return 0;

	bool converged=false;
#ifdef CONMAJ_LOGGING
	static int call_no=0;
#endif
	float *g = e->fArray1;
	float *old_place = e->fArray2;
	float *d = e->fArray3;
    //fprintf(stderr,"Entered: constrained_majorization_vpsc, #constraints=%d\n",e->m);
    if(e->m>0) {
	    for (i=0;i<e->n;i++) {
		    setVariableDesiredPos(e->vs[i],place[i]);
	    }
        //fprintf(stderr,"  calling satisfyVPSC...\n");
        satisfyVPSC(e->vpsc);	
        for (i=0;i<e->n;i++) {
            place[i]=getVariablePos(e->vs[i]);
            //fprintf(stderr,"vs[%d]=%f\n",i,place[i]);
        }
        //fprintf(stderr,"    done.\n");
    }
#ifdef CONMAJ_LOGGING
	float prev_stress=0;
	for (i=0; i<e->n; i++) {
		prev_stress += 2*b[i]*place[i];
		for (j=0; j<e->n; j++) {
			prev_stress -= e->A[i][j]*place[j]*place[i];
		}
	}
	FILE *logfile = fopen("constrained_majorization_log","a");

	//fprintf(logfile,"grad proj %d: stress=%f\n",call_no,prev_stress);
#endif
	
	for (counter=0; counter<max_iterations&&!converged; counter++) {
        //fprintf(stderr,".");
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
        if(e->m>0) {
            //project to constraint boundary
            for (i=0;i<e->n;i++) {
                setVariableDesiredPos(e->vs[i],place[i]);
            }
            satisfyVPSC(e->vpsc);
            for (i=0;i<e->n;i++) {
                place[i]=getVariablePos(e->vs[i]);
            }
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
			test+= fabs(place[i]-old_place[i]);
		}
#ifdef CONMAJ_LOGGING
		float stress=0;
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

CMajEnvVPSC*
initCMajVPSC(int n, float* packedMat, vtx_data* graph,
        int diredges, float edge_gap, cluster_data *clusters)
{
    int i,j;
    // nv is the number of real nodes
    int nv = n - 2*clusters->nclusters;
    int nEdgeCs, nConCs;
    //fprintf(stderr,"Entered initCMajVPSC\n");
	CMajEnvVPSC *e = GNEW(CMajEnvVPSC);
	e->A=NULL;
	e->n=n;
	/******************************************************************************
	** First, generate separation constraints for edges in largest acyclic subgraph
	*******************************************************************************/

    /* if we have clusters then we'll need two constraints for each var in
     * a cluster */
    e->gcs=NULL;
    e->vs=N_GNEW(e->n, Variable*);
    for(i=0;i<n;i++) {
        e->vs[i]=newVariable(i,1.0,1.0);
    }
    nEdgeCs=0;
    if(diredges) {
        //fprintf(stderr,"  generate edge constraints...\n");
        for(i=0;i<nv;i++) {
            for(j=1;j<graph[i].nedges;j++) {
                //fprintf(stderr,"edist=%f\n",graph[i].edists[j]);
                if(graph[i].edists[j]>0.01) {
                    nEdgeCs++;
                }
            }
        }
    }
    nConCs=2*clusters->nvars;
    e->gcs=newConstraints(nEdgeCs+nConCs);
    e->gm=0;
    if(diredges) {
        for(i=0;i<nv;i++) {
            for(j=1;j<graph[i].nedges;j++) {
                int u=i,v=graph[i].edges[j];
                if(graph[i].edists[j]>0) {
                    e->gcs[e->gm++]=newConstraint(e->vs[u],e->vs[v],edge_gap);
                }
            }
        }
        //fprintf(stderr,"  generate edge constraints... done: n=%d,m=%d\n",e->n,e->gm);
    }
    if(clusters->nclusters>0) {
        //fprintf(stderr,"  generate cluster containment constraints...\n");
        for(i=0;i<clusters->nclusters;i++) {
            for(j=0;j<clusters->clustersizes[i];j++) {
                Variable* v=e->vs[clusters->clusters[i][j]];
                Variable* cl=e->vs[nv+2*i];
                Variable* cr=e->vs[nv+2*i+1];
                e->gcs[e->gm++]=newConstraint(cl,v,0);
                e->gcs[e->gm++]=newConstraint(v,cr,0);
            }
        }
        //fprintf(stderr,"  containment constraints... done: \n");
    }

    e->m=0;
    e->cs=NULL;
    if(e->gm>0) {
        e->vpsc = newIncVPSC(e->n,e->vs,e->gm,e->gcs);
        e->m=e->gm;
        e->cs=e->gcs;
    }
    if(packedMat!=NULL) {
	    e->A = unpackMatrix(packedMat,n);
    }

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
    //fprintf(stderr,"  done.\n");
	return e;
}
void
deleteCMajEnvVPSC(CMajEnvVPSC *e) 
{
    int i;
    if(e->A!=NULL) {
        free (e->A[0]); 
        free (e->A);
    }
    if(e->m>0) {
        deleteVPSC(e->vpsc);
        if(e->cs!=e->gcs && e->gcs!=NULL) deleteConstraints(0,e->gcs);
        deleteConstraints(e->m,e->cs);
        for(i=0;i<e->n;i++) {
            deleteVariable(e->vs[i]);
        }
        free (e->vs);
    }
	free (e->fArray1);
	free (e->fArray2);
	free (e->fArray3);
	free (e);
}

// generate non-overlap constraints inside each cluster, including dummy
// nodes at bounds of cluster
// generate constraints again for top level nodes and clusters treating
// clusters as rectangles of dim (l,r,b,t)
// for each cluster map in-constraints to l out-constraints to r 
//
// For now, we'll keep the global containment constraints already
// generated for each cluster, and simply generate non-overlap constraints
// for all nodes and then an additional set of non-overlap constraints for
// clusters that we'll map back to the dummy vars as above.
void generateNonoverlapConstraints(
        CMajEnvVPSC* e,
        pointf* nsize,
        pointf gap,
        float nsizeScale,
        float** coords,
        int k,
        cluster_data* clusters
) {
    Constraint **csol, **csolptr;
    int i,j,mol=0;
    boxf bb[e->n];
    int n=e->n;
    bool genclusters=clusters->nclusters>0;
    if(genclusters) {
        // n is the number of real variables, not dummy cluster vars
        n-=2*clusters->nclusters;
    }
    if(k==0) {
        // grow a bit in the x dimension, so that if overlap is resolved
        // horizontally then it won't be considered overlapping vertically
        nsizeScale*=1.0001;
    }
    for(i=0;i<n;i++) {
        bb[i].LL.x=coords[0][i]-nsizeScale*nsize[i].x/2.0-gap.x/2.0; 
        bb[i].UR.x=coords[0][i]+nsizeScale*nsize[i].x/2.0+gap.x/2.0; 
        bb[i].LL.y=coords[1][i]-nsizeScale*nsize[i].y/2.0-gap.y/2.0; 
        bb[i].UR.y=coords[1][i]+nsizeScale*nsize[i].y/2.0+gap.y/2.0; 
    }
    if(genclusters) {
        Constraint **cscl[clusters->nclusters+1];
        int cm[clusters->nclusters+1];
        for(i=0;i<clusters->nclusters;i++) {
            int cn=clusters->clustersizes[i];
            Variable* cvs[cn+2];
            boxf cbb[cn+2];
            // compute cluster bounding bb
            boxf container;
            container.LL.x=container.LL.y=DBL_MAX;
            container.UR.x=container.UR.y=-DBL_MAX;
            for(j=0;j<cn;j++) {
                int iv=clusters->clusters[i][j];
                cvs[j]=e->vs[iv];
                B2BF(bb[iv],cbb[j]);
                EXPANDBB(container,bb[iv]);
            }
            B2BF(container,clusters->bb[i]);
            cvs[cn]=e->vs[n+2*i];
            cvs[cn+1]=e->vs[n+2*i+1];
            B2BF(container,cbb[cn]);
            B2BF(container,cbb[cn+1]);
            if(k==0) {
                cbb[cn].UR.x=container.LL.x+0.0001;
                cbb[cn+1].LL.x=container.UR.x-0.0001;
                cm[i] = genXConstraints(cn+2,cbb,cvs,&cscl[i]);
            } else {
                cbb[cn].UR.y=container.LL.y+0.0001;
                cbb[cn+1].LL.y=container.UR.y-0.0001;
                cm[i] = genYConstraints(cn+2,cbb,cvs,&cscl[i]);
            }
            mol+=cm[i];
        }
        // generate top level constraints
        {
            int cn=clusters->ntoplevel+clusters->nclusters;
            Variable* cvs[cn];
            boxf cbb[cn];
            for(i=0;i<clusters->ntoplevel;i++) {
                int iv=clusters->toplevel[i];
                cvs[i]=e->vs[iv];
                B2BF(bb[iv],cbb[i]);
            }
            // make dummy variables for clusters
            for(i=clusters->ntoplevel;i<cn;i++) {
                cvs[i]=newVariable(123+i,1,1);
                j=i-clusters->ntoplevel;
                B2BF(clusters->bb[j],cbb[i]);
            }
            i=clusters->nclusters;
            if(k==0) {
                cm[i] = genXConstraints(cn,cbb,cvs,&cscl[i]);
            } else {
                cm[i] = genYConstraints(cn,cbb,cvs,&cscl[i]);
            }
            // remap constraints from tmp dummy vars to cluster l and r vars
            for(i=clusters->ntoplevel;i<cn;i++) {
                j=i-clusters->ntoplevel;
                // dgap is the change in required constraint gap.
                // since we are going from a source rectangle the size
                // of the cluster bounding box to a zero width (in x dim,
                // zero height in y dim) rectangle, the change will be
                // half the bb width.
                double dgap;
                if(k==0) {
                    dgap=-(cbb[i].UR.x-cbb[i].LL.x)/2.0;
                } else {
                    dgap=-(cbb[i].UR.y-cbb[i].LL.y)/2.0;
                }
                remapInConstraints(cvs[i],e->vs[n+2*j],dgap);
                remapOutConstraints(cvs[i],e->vs[n+2*j+1],dgap);
                // there may be problems with cycles between
                // cluster non-overlap and diredge constraints,
                // to resolve:
                // 
                // for each constraint c:v->cvs[i]:
                //   if exists diredge constraint u->v where u in c:
                //     remap v->cl to cr->v (gap = height(v)/2)
                //
                // in=getInConstraints(cvs[i])
                // for(c : in) {
                //   assert(c.right==cvs[i]);
                //   vin=getOutConstraints(v=c.left)
                //   for(d : vin) {
                //     if(d.left.cluster==i):
                //       tmp=d.left
                //       d.left=d.right
                //       d.right=tmp
                //       d.gap==height(d.right)/2
                //   }
                // }
                //       
                deleteVariable(cvs[i]);
            }
            mol+=cm[clusters->nclusters];
        }
        csolptr=csol=newConstraints(mol);
        for(i=0;i<clusters->nclusters+1;i++) {
            // copy constraints into csol
            for(j=0;j<cm[i];j++) {
                *csolptr++=cscl[i][j];
            }
            deleteConstraints(0,cscl[i]);
        }
    } else {
        if(k==0) {
            mol = genXConstraints(n,bb,e->vs,&csol);
        } else {
            mol = genYConstraints(n,bb,e->vs,&csol);
        }
    }
    // remove constraints from previous iteration
    if(e->m>0) {
        // can't reuse instance of VPSC when constraints change!
        deleteVPSC(e->vpsc);
        for(i=e->gm==0?0:e->gm;i<e->m;i++) {
            // delete previous overlap constraints
            deleteConstraint(e->cs[i]);
        }
        // just delete the array, not the elements
        if(e->cs!=e->gcs) deleteConstraints(0,e->cs);
    }
    // if we have no global constraints then the overlap constraints 
    // are all we have to worry about.
    // Otherwise, we have to copy the global and overlap constraints 
    // into the one array
    if(e->gm==0) {
        e->m = mol;
        e->cs = csol;
    } else {
        e->m = mol + e->gm;
        e->cs = newConstraints(e->m);
        for(i=0;i<e->m;i++) {
            if(i<e->gm) {
                e->cs[i]=e->gcs[i];
            } else {
                e->cs[i]=csol[i-e->gm];
            }
        }
        // just delete the array, not the elements
        deleteConstraints(0,csol);
    }
    e->vpsc = newIncVPSC(e->n,e->vs,e->m,e->cs);
}

void removeoverlaps(int n,float** coords, pointf* nsize,
        pointf gap, cluster_data *clusters) {
    int i;
	CMajEnvVPSC *e = initCMajVPSC(n,NULL,NULL,0,0, clusters);
    generateNonoverlapConstraints(e,nsize,gap,1.0,coords,0,clusters);
    solveVPSC(e->vpsc);
    for(i=0;i<n;i++) {
        coords[0][i]=getVariablePos(e->vs[i]);
    }
    generateNonoverlapConstraints(e,nsize,gap,1.0,coords,1,clusters);
    solveVPSC(e->vpsc);
    for(i=0;i<n;i++) {
        coords[1][i]=getVariablePos(e->vs[i]);
    }
    deleteCMajEnvVPSC(e);
}

#endif /* DIGCOLA */

// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=4:softtabstop=4 :
