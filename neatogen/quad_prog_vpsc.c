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
	float *d = e->fArray4;
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
        e->vpsc = (IncVPSC*)newIncVPSC(e->n,e->vs,e->gm,e->gcs);
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
	e->fArray4 = N_GNEW(n,float);
	e->iArray1 = N_GNEW(n,int);
	e->iArray2 = N_GNEW(n,int);
	e->iArray3 = N_GNEW(n,int);
	e->iArray4 = N_GNEW(n,int);
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
	free (e->fArray4);
	free (e->iArray1);
	free (e->iArray2);
	free (e->iArray3);
	free (e->iArray4);
	free (e);
}

// todo: for clusters we need special behaviour:
// for each cluster with dummy nodes c0,c1:
//   l=min coords[0][i] - nwidth[i]/2 - xgap/2 (for all nodes i in cluster)
//   r=max coords[0][i] + nwidth[i]/2 + xgap/2
//   b=min coords[1][i] - nwidth[i]/2 - xgap/2
//   t=max coords[1][i] + nwidth[i]/2 + xgap/2
//   w=r-l, h=t-b
//   if k==0:
//     nheight[cl]=nheight[cr]=h
//     nwidth=0.00001    
//     coords[0][cl]=l 
//     coords[0][cr]=r 
//     genXConstraints(nodes in cluster and l and r)
//   if k==1:
//     similar
//
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
        float* nwidth,
        float* nheight,
        float xgap,
        float ygap,
        float nsizeScale,
        float** coords,
        int k,
        cluster_data* clusters
) {
    Constraint **csol, **csolptr;
    int i,j,mol=0,mcl;
    double xmin[e->n],xmax[e->n],ymin[e->n],ymax[e->n];
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
        xmin[i]=coords[0][i]-nsizeScale*nwidth[i]/2.0-xgap/2.0; 
        xmax[i]=coords[0][i]+nsizeScale*nwidth[i]/2.0+xgap/2.0; 
        ymin[i]=coords[1][i]-nsizeScale*nheight[i]/2.0-ygap/2.0; 
        ymax[i]=coords[1][i]+nsizeScale*nheight[i]/2.0+ygap/2.0; 
    }
    if(genclusters) {
        Constraint ***cscl = N_GNEW(clusters->nclusters+1,Constraint**);
        int *cm = N_GNEW(clusters->nclusters+1,int);
        for(i=0;i<clusters->nclusters;i++) {
            int cn=clusters->clustersizes[i];
            Variable** cvs = N_GNEW(cn+2,Variable*);
            double *cxmin = N_GNEW(cn+2,double);
            double *cymin = N_GNEW(cn+2,double);
            double *cxmax = N_GNEW(cn+2,double);
            double *cymax = N_GNEW(cn+2,double);
            // compute cluster bounding boxes
            double cxm=100000000;
            double cym=100000000;
            double cxM=-100000000;
            double cyM=-100000000;
            for(j=0;j<cn;j++) {
                int iv=clusters->clusters[i][j];
                cvs[j]=e->vs[iv];
                cxmin[j]=xmin[iv];
                cymin[j]=ymin[iv];
                cxmax[j]=xmax[iv];
                cymax[j]=ymax[iv];
                cxm=MIN(cxm,xmin[iv]);
                cym=MIN(cym,ymin[iv]);
                cxM=MAX(cxM,xmax[iv]);
                cyM=MAX(cyM,ymax[iv]);
            }
            clusters->bbllx[i]=cxm;
            clusters->bblly[i]=cym;
            clusters->bburx[i]=cxM;
            clusters->bbury[i]=cyM;
            cvs[cn]=e->vs[n+2*i];
            cvs[cn+1]=e->vs[n+2*i+1];
            if(k==0) {
                cxmin[cn]=cxm;
                cymin[cn]=cym;
                cxmax[cn]=cxm+0.0001;
                cymax[cn]=cyM;
                cxmin[cn+1]=cxM-0.0001;
                cymin[cn+1]=cym;
                cxmax[cn+1]=cxM;
                cymax[cn+1]=cyM;
                cm[i] = genXConstraints(cn+2,cxmin,cxmax,cymin,cymax,cvs,&cscl[i]);
            } else {
                cxmin[cn]=cxm;
                cymin[cn]=cym;
                cxmax[cn]=cxM;
                cymax[cn]=cym+0.0001;
                cxmin[cn+1]=cxm;
                cymin[cn+1]=cyM-0.0001;
                cxmax[cn+1]=cxM;
                cymax[cn+1]=cyM;
                cm[i] = genYConstraints(cn+2,cxmin,cxmax,cymin,cymax,cvs,&cscl[i]);
            }
            mol+=cm[i];
            free(cvs);
            free(cxmin);
            free(cymin);
            free(cxmax);
            free(cymax);
        }
        // generate top level constraints
        {
            int cn=clusters->ntoplevel+clusters->nclusters;
            int cmtl;
            Variable** cvs = N_GNEW(cn,Variable*);
            double *cxmin = N_GNEW(cn,double);
            double *cymin = N_GNEW(cn,double);
            double *cxmax = N_GNEW(cn,double);
            double *cymax = N_GNEW(cn,double);
            for(i=0;i<clusters->ntoplevel;i++) {
                int iv=clusters->toplevel[i];
                cvs[i]=e->vs[iv];
                cxmin[i]=xmin[iv];
                cymin[i]=ymin[iv];
                cxmax[i]=xmax[iv];
                cymax[i]=ymax[iv];
            }
            // make dummy variables for clusters
            for(i=clusters->ntoplevel;i<cn;i++) {
                cvs[i]=newVariable(123+i,1,1);
                j=i-clusters->ntoplevel;
                cxmin[i]=clusters->bbllx[j];
                cymin[i]=clusters->bblly[j];
                cxmax[i]=clusters->bburx[j];
                cymax[i]=clusters->bbury[j];
            }
            i=clusters->nclusters;
            if(k==0) {
                cm[i] = genXConstraints(cn,cxmin,cxmax,cymin,cymax,cvs,&cscl[i]);
            } else {
                cm[i] = genYConstraints(cn,cxmin,cxmax,cymin,cymax,cvs,&cscl[i]);
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
                    dgap=-(cxmax[i]-cxmin[i])/2.0;
                } else {
                    dgap=-(cymax[i]-cymin[i])/2.0;
                }
                remapInConstraints(cvs[i],e->vs[n+2*j],dgap);
                remapOutConstraints(cvs[i],e->vs[n+2*j+1],dgap);
                deleteVariable(cvs[i]);
            }
            mol+=cm[clusters->nclusters];
            free(cvs);
            free(cxmin);
            free(cymin);
            free(cxmax);
            free(cymax);
        }
        csolptr=csol=newConstraints(mol);
        for(i=0;i<clusters->nclusters+1;i++) {
            // copy constraints into csol
            for(j=0;j<cm[i];j++) {
                *csolptr++=cscl[i][j];
            }
            deleteConstraints(0,cscl[i]);
        }
        free(cscl);
    } else {
        if(k==0) {
            mol = genXConstraints(n,xmin,xmax,ymin,ymax,e->vs,&csol);
        } else {
            mol = genYConstraints(n,xmin,xmax,ymin,ymax,e->vs,&csol);
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

void removeoverlaps(int n,float **coords,float *nwidth, float *nheight,
        float xgap, float ygap, cluster_data *clusters) {
    int i;
	CMajEnvVPSC *e = initCMajVPSC(n,NULL,NULL,0,0, clusters);
    generateNonoverlapConstraints(e,nwidth,nheight,xgap,ygap,1.0,coords,0,clusters);
    solveVPSC(e->vpsc);
    for(i=0;i<n;i++) {
        coords[0][i]=getVariablePos(e->vs[i]);
    }
    generateNonoverlapConstraints(e,nwidth,nheight,xgap,ygap,1.0,coords,1,clusters);
    solveVPSC(e->vpsc);
    for(i=0;i<n;i++) {
        coords[1][i]=getVariablePos(e->vs[i]);
    }
    deleteCMajEnvVPSC(e);
}

#endif /* DIGCOLA */

// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=4:softtabstop=4 :
